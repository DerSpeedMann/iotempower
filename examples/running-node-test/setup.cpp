/* setup.cpp
 * This is the configuration code for a ulnoiot node.
 * This file is a full base example for all devices possible in 
 * ulnoiot. There is a configuration for each device possible in here.
 * It serves and includes current configuration tests.
 * 
 * TODO: split into single files/example node-folders.
 * */


//////// Device setup  ////////
// output example
// output(blue_led, "blue", ONBOARDLED, "off", "on").set("off");

// input example
// Add a button with a slightly higher debounce rate, which will report
// in the topic myroom/test1/button1.
// input(b1, "button1", D3, "released", "pressed")
//     .with_threshold(3)
//     .with_on_change_callback([&] {
//         if(b1.is("pressed")) {
//             blue_led.toggle();
//         }
//     });

// Examples for analog device:
// analog(a0, "a0").with_precision(5); //.with_threshold(100, "high", "low");
// analog(a0, "a0").with_filter_callback([&] {
//     const int buflen = 100;
//     static long sum = 0;
//     static long values_count = 0;
//     int v = a0.read_int();
//     sum += v;
//     values_count ++;
//     if(values_count >= buflen) {
//         a0.write_int(sum/values_count);
//         values_count = 0;
//         sum = 0;
//         return true;
//     }
//     return false;
// });
// analog(a0, "a0").with_filter_callback(filter_average(int, 100, a0));

// edge_counter example
// Count rising and falling signals on D2=Pin(4) and
// report number counted at myroom/test1/shock1.
// edge_counter(ecounter1, "shock1", D2);

// Examples for temperature (and humidity) measurement
// dht(temp1, "th", D7);
// ds18b20(temp2, "ds", D2);

// Servo motor
// servo(m1, "servo", D5);

// Distance sensor
// hcsr04(dist, "distance", D6, D5).with_precision(5);

// Example for single RGB-led
// rgb(rgb_led, "r0", D6, D5, D0, true);

// RGB strips
// rgb_strip(strip1, "strip1", 50, WS2811, D3, BRG);
// // rgb_strip(strip2, "strip2", 50, WS2811, D5, BRG);
// // rgb_strip(strip3, "strip3", 50, WS2811, D4, BRG);
// // rgb_strip(strip4, "strip4", 50, WS2811, D1, BRG);

// A matrix consiting of several strips
// rgb_matrix(matrix, "matrix", 25, 2)
//        .with(strip1, 0, 0, Right_Down, 25);
// //       .with(strip2, 0, 1)
// //       .with(strip3, 0, 2)
// //       .with(strip4, 0, 3);

// // following is a long example for the animator device with
// // its global variables and  support functions 
// // (this one animates the matrix defined beforehand)
//
// frames, anim_type, and anim_types are needed for testing the animation
// unsigned long frames[4]={0,0,0,0};
// enum anim_type {none, fade, fade_to, scroll};
// anim_type anim_types[4]={none, none, none, none};
//
// void draw_pattern(int p, int line, int len) {
//     switch(p) {
//         case 1:
//             matrix.rainbow(0, line, len, 1);
//             break;
//         case 2:
//             matrix.gradient_row(CRGB::Green, CRGB::Blue, 0, line, len, 1);
//             break;
//         case 3:
//             matrix.gradient_row(CRGB::Blue, CRGB::Red, 0, line, len, 1);
//             break;
//         default:
//             break;
//     }
// }
//
// void set_animation(Ustring& command, anim_type at, int leds, int frame_count) {
//     int stripnr = limit(command.as_int() - 1, 0, 3);
//     command.strip_param();
//     int pattern = limit(command.as_int(), 1, 3);
//     draw_pattern(matrix, pattern, stripnr, -1);
//     anim_types[stripnr] = at;
//     frames[stripnr] = frame_count;
// }
//
// // this defines the actual animator object
// animator(anim, "anim", matrix)
//     .with_fps(10)
//     .with_frame_builder( [&] () {
//         for(int i=0; i<4; i++) {
//             if(frames[i]>0 && anim_types[i] != none) {
//                 switch(anim_types[i]) {
//                     case fade:
//                         matrix.fade(8, 0, i, -1, 1);
//                         break;
//                     case scroll:
//                         matrix.scroll_right(false,i);
//                         break;
//                     case fade_to:
//                         matrix.fade_to(CRGB::Red, 16, 0, i, -1, 1);
//                     default:
//                         break; 
//                 }
//                 frames[i] --;
//             }
//         }
//     } ).with_command_handler( "fade", [&] (Ustring& command) {
//         set_animation(command, fade, -1, 100);
//     } ).with_command_handler( "fade_to", [&] (Ustring& command) {
//         set_animation(command, fade_to, -1, 50);
//     } ).with_command_handler( "scroll", [&] (Ustring& command) {
//         set_animation(command, scroll, 5, 150);
//     } );

// Examples for displays
display(display1, "d1", font_medium);
//display44780(display2, "d2", 16, 2);

// Example for pulse width modulation
// pwm(blue_led, "blue", ONBOARDLED).with_frequency(1000);

// //// actual test code
// output(blue_led, "blue", ONBOARDLED, "off", "on").set("on");
// void blink() {
//     blue_led.toggle();
//     do_later(2000, blink);
// };

// Optional init function, which is called right before device initialization
// void init() {
// } // end init

// Optional start function, which is called directly after all devices are
// started 
// Here you can define everything to get things started, often nothing 
// is necessary here. This is called after device initialization.
// Things like initial do_later calls should go here
// as well as a call to transmission_interval if something else than
// the initial 5s is preferred.
// void start() { // begin start, uncomment, if you need to start things 
//
//     // Trigger first blink
//     do_later(2000, blink);
//
//     // Send updates of current status every 10s (default 5)
//     transmission_interval(10);
//
// } // end start
