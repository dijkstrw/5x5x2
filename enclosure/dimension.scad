/*
 * Dimension of the boards as a stack:
 *
 *      mx switch stem     +                     3.6 mm
 *      mx switch top    ╭---╮                   6.6 mm
 *      mx switch foot   |   |                   5.0 mm -╮
 *             top pcb ╦═╪═══╪═══════════════╦   1.6 mm  |
 *                     ║                     ║   5.9 mm  | 15.1 mm
 *           lower pcb ╩═╪═════════════════╪═╩   1.6 mm  |
 *      solder fillets                           1.0 mm -╯
 *
 * The enclosure must enclose the pcb (100 x 100) and vertically from
 * the lower pcb solder fillets upto the mx switch foot. This is a
 * 15.1 mm span. To fixate the switches the top will be closed by a
 * metal plate with mx sized (13.8 x 13.8) holes in it. The plate is
 * 1.6mm thick, so the total vertical span to cover with the enclosure
 * material is 15.1 - 1.6 = 13.5 mm.
 *
 * The lower pcb has an usb c receptable, so room must be left for the
 * usb plug and overmoulding.
 *
 * PMMA layers can be bought in a variety of thicknesses, but we must
 * take care that in our stack-up the PMMA layer is supported by the
 * top or lower pcb to stay in place. This means that layer
 * transitions can only happen between 1.2 - 2.4 mm or 6.1 - 7.3 mm.
 *
 * Our 3mm screw inserts are 4.9mm high, so even with a 3mm baseplate,
 * our lower pcb will only start from 4.9mm -- this does mean that the
 * usb receptacle is at 4.9 + 0.8, and with usb overmoulding needing
 * 6.8mm, we need:
 *
 * - second layer = 1.9 screw insert +
 *                  0.8 middle of pcb +
 *                  6.8 / 2 usb plug size = 6.1mm
 * - third layer = 13.5 - 6.1             = 7.4mm
 *
 */

pcb_width = 100;             // width and depth of pcb to enclose
pcb_thickness = 1.6;         // thickness of the pcb
pcb_fillets = 1;             // max size of the fillets under pcb

wall_width = 3;              // width of the wall around the enclosure
screw_insert_size = 4.9;     // height of screw inserts

bottom_height = 3;           // height of bottom plate
second_height = 6.1;         // height of second plate
third_height = 13.5 - second_height;
top_height = 1.6;            // height of keys base plate

screw_hole_size = 3;         // hole size, m3 = 3mm
hole_offset = 3.5;           // offset of hole center to edge

usb_plug_height = 6.8;       // height of usb c plug overmoulding
usb_plug_width = 11;         // width of usb c plug overmoulding
usb_plug_depth = 24.3;       // depth of usb c plug, contact + overmoulding
usb_contact_height = 3;      // height of usb c contact

mx_plate_width = 13.8;
mx_edge_space = 5.5;
mx_inter_space = 5;

$fn = 50;                    // number of fragments in an arc
