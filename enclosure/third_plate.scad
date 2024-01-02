include <dimension.scad>;
include <plate.scad>;

if (PROJECTION) {
  projection()
    third_plate();
 } else {
  third_plate();
 }
