#!/bin/env python
"""
Susanoo transmits information about PC state to the Orochi 5x5x2
keyboard. This makes that the rgb lights on Orochi can accurately
display the current active display and sound mute status.
"""

# Wikipedia Susanoo:
# Susanoo (スサノオ; historical orthography: スサノヲ, 'Susanowo'),
# often referred to by the honorific title Susanoo-no-Mikoto, is a
# kami in Japanese mythology. The younger brother of Amaterasu,
# goddess of the sun and mythical ancestress of the Japanese imperial
# line, he is a multifaceted deity with contradictory characteristics
# (both good and bad), being portrayed in various stories either as a
# wild, impetuous god associated with the sea and storms, as a heroic
# figure who killed a monstrous serpent, or as a local deity linked
# with the harvest and agriculture.
#

import contextlib
import os
from asyncio import Event, Queue, TaskGroup, get_event_loop, start_unix_server
from statistics import mean

import pulsectl_asyncio
import pyudev
from serial_asyncio import open_serial_connection

SOCKET_NAME = f"/run/user/{os.getuid()}/orochi/socket"
FIFO_NAME = f"/run/user/{os.getuid()}/orochi/fifo"


class Mux:
    """Mux between a number of terminals and serial"""

    def __init__(self, reader, writer, debug=False):
        self.running = True
        self.reader = reader
        self.writer = writer
        self.debug = debug
        self.serial_out = Queue()
        self.terminals = []

    def terminal_add(self, terminal):
        self.terminals.append(terminal)
        print(f"{len(self.terminals)} clients active")

    async def terminal_line_received(self, line):
        if self.debug:
            print(line.decode())
        await self.serial_out.put(line)

    async def terminal_line_transmit(self, line):
        if self.debug:
            print(line.decode())
        for t in self.terminals:
            try:
                await t.write(line)
            except Exception:
                self.terminal_abort(t)

    def terminal_abort(self, terminal):
        if terminal in self.terminals:
            self.terminals.remove(terminal)
        if self.debug:
            print(f"{len(self.terminals)} clients active")

    def stop(self):
        self.running = False

    async def read(self):
        while self.running:
            line = await self.reader.readline()
            await self.terminal_line_transmit(line)

    async def write(self):
        while self.running:
            line = await self.serial_out.get()
            if line:
                self.writer.write(line)
                await self.writer.drain()


class Terminal:
    """Terminal instance connected to a mux"""

    def __init__(self, taskgroup, mux, reader, writer):
        self.mux = mux
        self.reader = reader
        self.writer = writer
        self.mux.terminal_add(self)
        taskgroup.create_task(self.read())

    async def write(self, data):
        if self.writer.is_closing():
            self.mux.terminal_abort(self)
        else:
            self.writer.write(data)
            await self.writer.drain()

    async def read(self):
        while self.mux.running:
            if self.reader.at_eof():
                self.mux.terminal_abort(self)
                return
            line = await self.reader.readline()
            await self.mux.terminal_line_received(line)


class NamedPipe:
    """NamedPipe instance connected to a mux"""

    def __init__(self, taskgroup, mux, filename):
        self.taskgroup = taskgroup
        self.mux = mux
        self.filename = filename
        self.event = Event()
        self.line = None

    async def open(self):
        while self.mux.running:
            self.fd = os.open(self.filename, os.O_RDONLY | os.O_NONBLOCK)
            get_event_loop().add_reader(self.fd, self.syncread)
            await self.event.wait()
            self.event.clear()

    def syncread(self):
        try:
            line = os.read(self.fd, 1024)
            self.taskgroup.create_task(self.mux.terminal_line_received(line))
        except Exception:
            pass
        finally:
            get_event_loop().remove_reader(self.fd)
            os.close(self.fd)
            self.event.set()


class PulseListener:
    """Listener for Pulse Audio that transmits audio events to a mux"""

    def __init__(self, mux):
        self.mux = mux

    async def update_sound_output(self, pulse):
        sinks = await pulse.sink_list()
        mute = all(s.mute for s in sinks)
        if not mute:
            volume = min(
                int(
                    mean(
                        [
                            int(s.volume.value_flat * 0xFFFF)
                            for s in sinks
                            if s.mute == 0
                        ]
                    )
                ),
                0xFFFF,
            )
        else:
            volume = 0

        await self.mux.terminal_line_received(
            f"DV{volume:04x}\nDM{mute:01x}\n".encode()
        )

    async def update_sound_input(self, pulse):
        sources = await pulse.source_list()
        micmute = all(s.mute for s in sources)
        await self.mux.terminal_line_received(f"DR{micmute:01x}\n".encode())

    async def listen(self):
        async with pulsectl_asyncio.PulseAsync("event-printer") as pulse:
            # Initial update to ensure keyboard has state to work with
            await self.update_sound_input(pulse)
            await self.update_sound_output(pulse)
            # And then only update when something changes
            async for event in pulse.subscribe_events("sink", "source"):
                if event.facility == "sink" and event.t == "change":
                    await self.update_sound_output(pulse)
                elif event.facility == "source" and event.t == "change":
                    await self.update_sound_input(pulse)


def find_orochi():
    """Find Orochi keyboard device name"""
    context = pyudev.Context()
    return next(
        device.properties["DEVNAME"]
        for device in context.list_devices(subsystem="tty")
        if device.properties.get("ID_MODEL") == "Orochi"
    )


async def run(serial_name, file_name):
    """Start all async tasks"""
    print(f"Attaching to {serial_name}, mux at {file_name}")
    serial_reader, serial_writer = await open_serial_connection(
        url=serial_name, baudrate=115200
    )
    mux = Mux(serial_reader, serial_writer, debug=True)
    pulse_listener = PulseListener(mux)

    os.makedirs(os.path.dirname(FIFO_NAME), mode=0o700, exist_ok=True)
    os.makedirs(os.path.dirname(SOCKET_NAME), mode=0o700, exist_ok=True)
    with contextlib.suppress(OSError):
        os.remove(FIFO_NAME)

    os.mkfifo(FIFO_NAME)
    os.chmod(FIFO_NAME, 0o600)

    async with TaskGroup() as task_group:
        named_pipe = NamedPipe(task_group, mux, FIFO_NAME)
        task_group.create_task(named_pipe.open())
        task_group.create_task(mux.read())
        task_group.create_task(mux.write())
        task_group.create_task(pulse_listener.listen())
        task_group.create_task(
            start_unix_server(lambda r, w: Terminal(task_group, mux, r, w), file_name)
        )


serial_device = find_orochi()
loop = get_event_loop()
loop.run_until_complete(run(serial_device, SOCKET_NAME))
