include <dimension.scad>;
include <plate.scad>;

if (PROJECTION) {
  projection(cut=true)
    bottom_plate();
 } else {
  bottom_plate();
 }
