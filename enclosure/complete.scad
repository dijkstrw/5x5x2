include <dimension.scad>;
include <plate.scad>;

bottom_plate();
translate([0, 0, bottom_height/2 + second_height/2]) second_plate();
translate([0, 0, bottom_height/2 + second_height + third_height/2]) third_plate();
translate([0, 0, bottom_height/2 + second_height + third_height]) top_plate();
