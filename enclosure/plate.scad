include <dimension.scad>;

module plate(height = bottom_height)
{
  cube([pcb_width + 2 * wall_width,
        pcb_width + 2 * wall_width,
        height], center=true);
}

module screw_hole(height = bottom_height, size = screw_hole_size)
{
  cylinder(height, d = size, center = true);
}

module usb_plug()
{
  cube([usb_plug_width, usb_plug_depth, usb_plug_height], center = true);
}

module screw_plate(height = bottom_height)
{
  difference() {
    plate(height);
    // screw holes
    translate([(pcb_width / 2) - hole_offset,
               (pcb_width / 2) - hole_offset,
               0])
      screw_hole();
    translate([-1 * ((pcb_width / 2) - hole_offset),
               (pcb_width / 2) - hole_offset,
               0])
      screw_hole();
    translate([-1 * ((pcb_width / 2) - hole_offset),
               -1 * ((pcb_width / 2) - hole_offset),
               0])
      screw_hole();
    translate([((pcb_width / 2) - hole_offset),
               -1 * ((pcb_width / 2) - hole_offset),
               0])
      screw_hole();
  }
}

module bottom_plate(height = bottom_height)
{
  difference() {
    screw_plate(height);
    // room for usb plug
    translate([0, pcb_width / 2, 0])
      translate([0, (usb_plug_depth / 2) - wall_width, 0])
      usb_plug();
  }
}

module second_plate()
{
  difference() {
    plate(second_height);
    cube([pcb_width, pcb_width, second_height], center = true);
    // room for usb plug
    translate([0, pcb_width / 2, 0])
      translate([0, (usb_plug_depth / 2) - wall_width, (plug_height / 2)])
      usb_plug();
  }
}

module third_plate()
{
  difference() {
    plate(third_height);
    cube([pcb_width, pcb_width, third_height], center = true);
  }
}

module top_plate()
{
  difference() {
    screw_plate(height = top_height);
    // keys
        translate([-pcb_width / 2,
               -pcb_width / 2,
               -pcb_thickness / 2])
      for (row = [1 : 1 : 5]) {
        for (col = [1 : 1 : 5]) {
          translate([mx_edge_space + (col - 1) * (mx_width + mx_inter_space) + (mx_width - mx_plate_width) / 2,
                     mx_edge_space + (row - 1) * (mx_width + mx_inter_space) + (mx_width - mx_plate_width) / 2,
                     0])
            cube([mx_plate_width, mx_plate_width, top_height]);
        }
      }
  }
}
