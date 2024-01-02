include <dimension.scad>;
include <plate.scad>;

if (PROJECTION) {
  projection(cut=true)
    second_plate();
 } else {
     second_plate();
 }
