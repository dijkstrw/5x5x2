local awful = require("awful")
local gears = require("gears")
local keys = {}

local uid = io.popen("id -u"):read("*l")
local orochi_fifo = string.format("/run/user/%s/orochi/fifo", uid)

-- Send display for a screen to orochi
function orochi_set_display(screen, display)
   local event = string.format("DD%02x%02x\n", screen & 0xff, display &0xff)
   file = io.open(orochi_fifo, "w+")
   if file then
      file:write(event)
      file:close()
   end
end

-- Set volume on sink (output) and (un)mute when sensible
function volume_control(step)
    local cmd
    if step == 0 then
       cmd = "pactl set-sink-mute @DEFAULT_SINK@ toggle "..
          "&& pactl set-source-mute @DEFAULT_SOURCE@ toggle"
    else
        sign = step > 0 and "+" or ""
        cmd = "pactl set-sink-mute @DEFAULT_SINK@ 0 "..
           "&& pactl set-source-mute @DEFAULT_SOURCE@ 0 "..
           "&& pactl set-sink-volume @DEFAULT_SINK@ "..
           sign..tostring(step).."%"
    end
    awful.spawn.with_shell(cmd)
end

-- Apps tied to keys
apps = {}
apps.browser = function ()
    awful.spawn("firefox", {switchtotag = true})
end
apps.editor = function ()
    awful.spawn("emacs", { switchtotag = true })
end
apps.passwords = function ()
    awful.spawn.with_shell("~/bin/passmenu.sh --type")
end
apps.ssh = function ()
    awful.spawn.with_shell("~/bin/sshmenu.sh --type")
end
apps.screensaver = function ()
    awful.spawn("slock")
end
apps.worker = function ()
    awful.spawn("worker", { switchtotag = true })
end

-- Orochi keys
keys.orochi = gears.table.join(
   -- Volume Control with volume knob or volume knob click
   awful.key( { }, "XF86AudioMute",
      function()
         volume_control(0)
      end,
      {description = "(un)mute volume", group = "volume"}),
   awful.key( { }, "XF86AudioLowerVolume",
      function()
         volume_control(-1)
      end,
      {description = "lower volume", group = "volume"}),
   awful.key( { }, "XF86AudioRaiseVolume",
      function()
         volume_control(1)
      end,
      {description = "raise volume", group = "volume"}),

   -- 3rd row
   awful.key( { }, "XF86Documents", apps.editor,
      {description = "editor", group = "launcher"}),

   awful.key( { }, "XF86ScreenSaver", apps.screensaver,
      {description = "screensaver", group = "launcher"}),

   awful.key( { }, "XF86Explorer", apps.worker,
      {description = "worker", group = "launcher"}),

   awful.key( { }, "XF86WWW", apps.browser,
      {description = "browser", group = "launcher"}),

   -- 4th row, F22 - F24
   awful.key({ }, "#200", function() awful.spawn("xkill") end,
      {description = "xkill", group = "launcher"}),

   awful.key({ }, "#201", apps.ssh,
      {description = "ssh launcher", group = "launcher"}),

   awful.key({ }, "#202", apps.passwords,
      {description = "pass launcher", group = "launcher"}))

for i = 1, 9 do
   keys.orochi = gears.table.join(keys.orochi,
      awful.key({ }, "#" .. i + 190,
         function()
            for s in screen do
               local tag = s.tags[i]
               if tag then
                  tag:view_only()
               end
            end
            orochi_set_display(0, i)
         end,
         {description = "goto tag " .. i, group = "tags"}),

      awful.key({ "Control" }, "#" .. i + 190,
         function ()
            for s in screen do
               local tag = s.tags[i]
               if tag then
                  tag.selected = not tag.selected
               end
            end
         end,
         {description = "add or remove clients from tag " .. i, group = "tags"})
   )
end

return keys.orochi
