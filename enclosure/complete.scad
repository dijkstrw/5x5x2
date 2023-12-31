include <dimension.scad>;
include <plate.scad>;

bottom_plate();
translate([0, 0, bottom_height]) second_plate();
translate([0, 0, bottom_height + second_height]) third_plate();
translate([0, 0, bottom_height + second_height + third_height]) top_plate();
