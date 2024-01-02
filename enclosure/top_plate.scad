include <plate.scad>;

if (PROJECTION) {
  projection(cut=true)
    top_plate();
 } else {
  top_plate();
 }
