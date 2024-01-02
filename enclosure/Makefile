SRCS := bottom_plate.scad second_plate.scad third_plate.scad top_plate.scad
OBJS := ${SRCS:scad=dxf}

.PHONY: all
all: ${OBJS}

clean:
	rm -f ${OBJS}

%.dxf: %.scad Makefile
	openscad -DPROJECTION=1 $< -o $@
