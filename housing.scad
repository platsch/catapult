wall_t = 6.0;
x = 176+1;
y = 100;
z = 31;

difference() {
    housing();
    backplate(screws=true);
}
translate([20, 0, 0]) backplate(screws=false);
translate([0, 0, 100]) cover();

translate([0, -50, 0]) led_holder(h=260,negative=false, cable_slit=true, top_stack=true);

translate([0, -100, 0]) led_holder(h=280,negative=false, cable_slit=false, bottom_stack=true, top_stack=false);


module cover() {
    difference() {
        translate([x/2+wall_t, y/2+wall_t, 80]) {
            // main cover body
            rotate([180, 0, 0]) bottom(x+2*wall_t, y+2*wall_t, 8, 5);
        }
        
        // screws in 4 corners & "ears"
        housing(negative=true);
        
        // led cutout
        translate([60, y/2+wall_t, 80-wall_t]) {
            led_ring(negative=true);
            %led_ring(negative=false);
            // screw insets
            for(i=[0:3]) {
                rotate([0, 0, i*90]) {
                    translate([20, 20, -2.01]) {
                        cylinder(d=4, h=6, $fn=100);
                    }
                }
            }
        }
    }
    
    //led clamp
    translate([-100, 0, 0])
    difference() {
        union() {
            hull() {
                for(i=[0:3]) {
                    rotate([0, 0, i*90]) {
                        translate([25, 25, 0]) {
                            cylinder(d=2, h=2, $fn=50);
                        }
                    }
                }
            }
            translate([0, 0, 2]) cylinder(d=44, h=3, $fn=100);
        }
        for(i=[0:3]) {
            rotate([0, 0, i*90]) {
                translate([20, 20, 0]) {
                    cylinder(d=3.5, h=6, $fn=50, center=true);
                }
            }
        }
        // cable outlets
        for(i=[0, -1]) mirror([0, i, 0])
        hull() {
            translate([16, 15, -1]) cylinder(h=100, d=6, $fn=100);
            translate([25, 12, -1]) cylinder(h=100, d=6, $fn=100);
        }
        rotate([0, 0, 90])
        hull() {
            translate([16, 15, -1]) cylinder(h=100, d=6, $fn=100);
            translate([25, 12, -1]) cylinder(h=100, d=6, $fn=100);
        }
    }
}

module housing(negative=false) {
union() {
difference() {
    // main body
    difference() {
        translate([x/2+wall_t, y/2+wall_t, 0]) bottom(x+2*wall_t, y+2*wall_t, z+wall_t+35, 5);
        // cut away back plate
        translate([25+x+wall_t-1, 0, 0]) cube([50, 400, 400], center=true);
    }
    
    // psu
    translate([wall_t, wall_t, wall_t+0.1]) psu(x, y, z);
    
    // cutout pcb
    translate([wall_t, wall_t, z+2*wall_t]) cube([x-50, y, 100]);
    
    // knobs / PCB
    translate([wall_t, y/2+wall_t, z+2*wall_t]) {
        pcb();
        if(!negative) %pcb();
    }
    
    
    // cutout back
    cutout_x = 90;
    translate([x-cutout_x+wall_t, wall_t, wall_t]) cube([cutout_x, y, 100]);
    
    // cable openings for stips
    translate([80, y+2*wall_t+0.5, z+wall_t+35]) rotate([90, 0, 0]) cylinder(d=12, h=y+2*wall_t+1, $fn=100);
    
    // cooling bottom
    for(i=[0:10]) {
        if(i!=5) {
            translate([15+i*15, wall_t+2, -1]) cube([6, y-4, wall_t*3]);
        }
    }
    // cooling side
    translate([100-24, -10, wall_t+2]) cube([6, 2*y, z-8]);
    translate([100-12, -10, wall_t+2]) cube([6, 2*y, z-8]);
    translate([100, -10, wall_t+2]) cube([6, 2*y, z]);
    translate([112, -10, wall_t+2]) cube([6, 2*y, z]);
    translate([124, -10, wall_t+2]) cube([6, 2*y, z]);
    translate([136, -100, wall_t+2]) cube([6, 2*y, z]);
    translate([154, -10, wall_t+2]) cube([6, 2*y, z]);
}
// screw counterparts backplate
for(i=[1, -1]) {
    translate([x+wall_t-1, wall_t+y/2+i*y/2, z+20]) {
        rotate([0, 90, 0]) rotate([0, 0, i*90]) screw_sink();
    }
}

// screw counterparts top-plate
translate([wall_t, wall_t, z+wall_t+15]) screw_sink_top(negative=negative);
translate([x+wall_t-1, wall_t, z+wall_t+15]) rotate([0, 0, 90]) screw_sink_top(negative=negative);
translate([x+wall_t-1, y+wall_t, z+wall_t+15]) rotate([0, 0, 180]) screw_sink_top(negative=negative);
translate([wall_t, y+wall_t, z+wall_t+15]) rotate([0, 0, -90]) screw_sink_top(negative=negative);

// led holders
difference() {
    translate([0, y/2+wall_t, 0]) {
        led_scaling = negative ? 1.6 : 1.5;
        for(i=[-1, 1]) {
            translate([wall_t+50, i*(y/2+wall_t+15), 0]) rotate([0, 0, i*-20]) scale([led_scaling, led_scaling, 1]) led_holder(h=180, top_stack=true);
        }
    }
    translate([0, y/2+wall_t, 0]) {
        for(i=[-1, 1]) {
            translate([wall_t+50, i*(y/2+wall_t+15), wall_t]) rotate([0, 0, i*-20]) {
                scale([1.05, 1.05, 1]) led_holder(h=180,negative=true, top_stack=true);
                led_holder(h=250,negative=false, cable_slit=true, top_stack=true);
            }
        }
    }
    
    translate([-x/2, -y/2, z+wall_t+35+30]) cube([2*x, 3*y, 400]);
}
}
}

module backplate(screws=false) {
translate([x/2+wall_t, y/2+wall_t, 0]) {
    union() {
        difference() {
            bottom(x+2*wall_t, y+2*wall_t, z+wall_t+35, 5);
            translate([-wall_t/2-0.1, 0, z]) cube([x+wall_t, 2*y, 4*z], center=true);
            for(i=[-1, 1]) {
                translate([x/2, i*(y/2-2), z+20]) {
                    rotate([0, 90, 0]) {
                        cylinder(d=3.5, h=50, $fn=50, center=true);
                        translate([0, 0, 2]) cylinder(d=6, h=50, $fn=50, center=false);
                    }
                }
            }
        }
        if(screws) {
            for(i=[-1, 1]) {
                translate([x/2, i*(y/2-2), z+20]) {
                    rotate([0, 90, 0]) cylinder(d=4, h=30, $fn=50, center=true);
                }
            }
        }
    }
}
}

module bottom(x, y, z, c) {
    points = [[c, 0], [x-c, 0], [x, c], [x, y-c], [x-c, y], [c, y], [0, y-c], [0, c]];
    // scaling factors
    scalex=1+(2*c/x);
    scaley=1+(2*c/y);
    
    union() {
        //bottom part
        scale([1/scalex, 1/scaley, 1]) {
            linear_extrude(c, center=false, scale=[scalex, scaley]) {
                translate([-x/2, -y/2, 0])
                polygon(points);
            }
        }
        
        // main body
        translate([0, 0, c-0.001]) {
            linear_extrude(z-c+0.001, center=false) {
                translate([-x/2, -y/2, 0])
                polygon(points);
            }
        }
    }
    //#translate([0, 0, z/2+0.1]) cube([x, y, z], center=true);
}

module screw_sink(dia=4.0) {
h=20;
    translate([0, 0, -h])
    difference() {
        hull() {
            translate([0, 0, 0.5]) cube([0.1, 4, 1], center=true);
            translate([0, 0, h-6-1]) cube([14, 10, h-6], center=true);
        }
        translate([0, -50, -1]) cube([100, 100, 100]);
    }
}

module screw_sink_top(dia=4.0, negative=false) {
    union() {
        difference() {
            linear_extrude(height=20) {
                polygon([[0, 0], [10, 0], [0, 10]]);
            }
            translate([dia/2, dia/2, 3]) cylinder(d=dia, h=50, $fn=100);
        }
        // negative screw
        if(negative) {
            translate([dia/2, dia/2, 3]) cylinder(d=dia, h=50, $fn=100);
            translate([dia/2, dia/2, 20+3]) cylinder(d=dia*1.5, h=30, $fn=100);
        }
    }
}

module led_holder(h=250, negative=false, cable_slit=false, bottom_stack=false, top_stack=false) {
    strip_w = 16;
    chamfer = 4;
    base_h = top_stack ? h : 80;
    
    difference() {
        union() {
            hull() {
                for(i=[-1, 1]) {
                    translate([0, i*(strip_w/2+2), 0]) {
                        cylinder(h=base_h, r=chamfer, $fn=100);
                    }
                }
                translate([strip_w/2+2, 0, 0]) {
                    cylinder(h=base_h, r=chamfer, $fn=100);
                }
                // top part
                if(!top_stack) {
                    for(i=[-1, 1]) {
                        translate([0, i*(strip_w/2+2), 0]) {
                            cylinder(h=base_h+20, r=chamfer, $fn=100);
                        }
                    }
                }
            }
            hull() {
                for(i=[-1, 1]) {
                    translate([0, i*(strip_w/2+2), 0]) {
                        cylinder(h=h, r=chamfer, $fn=100);
                    }
                }
            }
            
            if(top_stack) {
                translate([0.5, 0, h]) {
                    scale([0.75, 0.75, 1])
                    hull() {
                        for(i=[-1, 1]) {
                            translate([0, i*(strip_w/2+2), 0]) {
                                cylinder(h=20, r=chamfer, $fn=100);
                            }
                        }
                        translate([strip_w/2+2, 0, 0]) {
                            cylinder(h=20, r=chamfer, $fn=100);
                        }
                    }
                }
            }
            
            if(negative) {
            // led strip
            translate([-8, -strip_w/2-1, 0]) cube([10, strip_w+2, base_h]);
            
            // slit at backside
            translate([18, 0, h/2+3]) cube([20, 4, h], center=true);
            }
        }
        
        if(bottom_stack) {
                translate([0.5, 0, -0.1]) {
                    scale([0.83, 0.83, 1])
                    hull() {
                        for(i=[-1, 1]) {
                            translate([0, i*(strip_w/2+2), 0]) {
                                cylinder(h=21, r=chamfer, $fn=100);
                            }
                        }
                        translate([strip_w/2+2, 0, 0]) {
                            cylinder(h=21, r=chamfer, $fn=100);
                        }
                    }
                }
            }
        
        if(cable_slit) {
            // bottom slit
            translate([0, 0, 3.9]) cube([50, 10, 15], center=true);
        }
    }
}

module psu(x, y, z) {
    union() {
        cube([x, y, z]);
        // cable outlet
        hull() {
            translate([x-9, 1, 7])
            rotate([90, 0, 0]) cylinder(d=14, h=50, $fn=100);
            translate([x-9, -50, 0]) cube([9,50,14]);
        }
        
        // screws
        for(i=[-1,1]) {
            translate([20, y, z/2+i*9]) {
                rotate([-90, 0, 0]) {
                    cylinder(d=4, h=100, $fn=50, center=true);
                    translate([0, 0, 2]) cylinder(d=6, h=100, $fn=50, center=false);
                }
            }
        }
        translate([137, y, z/2]) {
            rotate([-90, 0, 0]) {
                cylinder(d=4, h=100, $fn=50, center=true);
                translate([0, 0, 2]) cylinder(d=6, h=100, $fn=50, center=false);
            }
        }
    }
}

module pcb() {
    x = 50;
    y = 70;
    z = 28;
    translate([x/2, 0, 0]) {
        union() {
            // main PCB
            translate([0, 0, z/2]) cube([x, y, z], center = true);
            // potis
            for(i=[0,1]) {
                translate([0, 1+(i*22), 17]) rotate([0, -90, 0]) {
                    cylinder(d=9, h=50, $fn=100);
                    translate([0, 0, 40]) cylinder(d=19, h=18, $fn=100);
                }
            }
            // arduino port
            translate([-35, -y/2+16, 13]) cube([40, 14, 10], center=true);
        }
    }
}

module led_ring(negative=false) {
    d_in = 31;
    d_out = 45;
    h = 3.5;
    
    
    union() {
        difference() {
            cylinder(d=d_out, h=h, $fn=100);
            if(!negative) {
                cylinder(d=d_in, h=3*h, center=true, $fn=100);
            }
        }
        if(negative) {
            cylinder(d=d_out-4, h=5*h, $fn=100);
            translate([0, 0, -h+0.01]) cylinder(d=d_out, h=2*h, $fn=100);
        }
    }
}