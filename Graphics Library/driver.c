/**
 * Author: Shawn Jin
 * This is the driver code, after it runs, it
 */

#include "graphics.h"
#define BUILD_COLOR(r,g,b) ((color_t) ( (r & 0x1f) << 11) | ((g & 0x3f) << 5) | (b & 0x1f))


int main() {
    // shapes it support
    enum Shape{
        Circle = 1, Rectangle
    };
    // initial the terminal: clean and disable the keypress echo
    // & buffering the keypress
    init_graphics();
    char key;
    // set x, y be the middle of the screen(terminal)
    int x = (640-20)/2;
    int y = (480-20)/2;
    // This is text info that will be shown at left-top of the screen
    const char* author_info = "Author: Shawn Jin";
    const char* help_info = "Press C to draw circle; press x to draw rectangle";
    // write down those text
    draw_text(2,0,author_info, BUILD_COLOR(0,50,0));
    draw_text(2,16,help_info, BUILD_COLOR(10,10,10));
    enum Shape shape = 2;		// set default shape is rectange
    do {
        key = getkey();
        // draw black rectangle to erase the old one
        if (shape == 1) {
            draw_circle(x,y,40,0);
        } else {
            draw_rect(x,y,20,30,0);
        }
        /**
         w: move up    (10 pixels)
         s: move down  (10 pixels)
         a: move left  (10 pixels)
         d: move right (10 pixels)
         
         c: draw circle     (radius: 20 pixels)
         x: draw rectangle  (height: 20, weight: 30 pixels)
         */
        switch (key) {
            case 'w':
                y -= 10;
                break;
            case 's':
                y += 10;
                break;
            case 'a':
                x -= 10;
                break;
            case 'd':
                x += 10;
                break;
            case 'c':
                shape = 1;
                break;
            case 'x':
                shape = 2;
                break;

        }
        // draw shapes
        if (shape == 1) {
            draw_circle(x,y,40,BUILD_COLOR(50,0,0));
        } else {
            draw_rect(x,y,20,30,BUILD_COLOR(0,0,15));
        }
        sleep_ms(20);

    } while (key != 'q');
    // exit "canvas mode" and re-able the keypress echo & buffering the keypresses
    exit_graphics();
}
