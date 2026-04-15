// --- PARAMETERS ---
wall = 2;               // Wall thickness
main_w = 90;            // Main body width
main_h = 50;            // Main body height
main_d = 80;            // Main body depth
btn_size = 25;          // Button box size

// Dimensions of your components
panel_w = 80;   panel_h = 45; // Solar panel dimensions
screen_w = 26;  screen_h = 15; // OLED display dimensions

$fn = 50;               // Smoothness of circles/cylinders

module main_case() {
    difference() {
        // 1. MAIN PYRAMID BODY
        rotate([90, 0, 90])
        linear_extrude(height = main_w)
        polygon(points=[[0,0], [main_d,0], [main_d/2, main_h]]);

        // 2. INTERNAL HOLLOW (Space to fit electronics)
        translate([wall, wall, wall])
        rotate([90, 0, 90])
        linear_extrude(height = main_w - wall*2)
        polygon(points=[[0,0], [main_d-wall*2,0], [(main_d-wall*2)/2, main_h-wall*2]]);

        // 3. CUTOUT FOR DISPLAY (FRONT SIDE)
        // Located on the left side of your previous screenshot
        translate([main_w/2 - screen_w/2, 10, 15]) 
        rotate([-35, 0, 0]) // Angle for the front slope
        cube([screen_w, screen_h, 20]); // Depth of 20 to ensure it cuts through

        // 4. CUTOUT FOR SOLAR PANEL (BACK SIDE)
        // Located on the right side of your previous screenshot
        translate([main_w/2 - panel_w/2, main_d - 25, 20]) 
        rotate([35, 0, 0]) // Angle for the back slope
        cube([panel_w, panel_h, 10]); 

        // 5. HOLE FOR USB (REAR BOTTOM)
        translate([main_w/2 - 6, main_d - 10, -1])
        cube([12, 15, 8]);
        
        // 6. HOLE FOR BUTTON WIRING
        translate([main_w - 5, 20, 10])
        rotate([0, 90, 0])
        cylinder(h = 10, r = 4);
    }
}

module button_box() {
    translate([main_w, 10, 0])
    difference() {
        cube([btn_size, btn_size, btn_size]);
        translate([wall, wall, wall]) cube([btn_size-wall*2, btn_size-wall*2, btn_size]);
        
        // Hole for the button itself (top side)
        translate([btn_size/2, btn_size/2, btn_size-5]) cylinder(h = 10, r = 3.5);
        
        // Hole for wiring (left side, towards the pyramid)
        translate([-5, btn_size/2 - 5, 10]) rotate([0, 90, 0]) cylinder(h = 10, r = 4);
    }
}

main_case();
button_box();