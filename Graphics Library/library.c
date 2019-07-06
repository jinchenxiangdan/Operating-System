// Author: Shawn Jin
// Assignment: AS1
// Due Date: Monday, june 17, 2019


/**
	use typedef def an unsiged int(16bit[5bits red (0-31)], [6 green 
	0-63], [5bits blue 0-31]

**/
//-----------------
// #include <stdio.h>
//-----------------


#include <stddef.h>
// #include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <termios.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <time.h>

#include "graphics.h"		// our header file
#include "iso_font.h"		// apple's support font

// global variables
int frame_buffer;
color_t* frame_buffer_mem;

long x_rec = 0;
long y_rec = 0;
long size  = 0;


/**
 * clear the screen
 */
void clear_screen() {
	// 0:stdin, 1: stdout, 2: stderr
	write(STDOUT_FILENO,"\033[2J",8);
}


/**
 * set input from std in off
 */
void init_graphics() {
	if ((frame_buffer = open("/dev/fb0", O_RDWR)) < 0) {
		return ;	// if cannot open file, return
	}
	// get the screen size and bits per pixels
	struct fb_var_screeninfo fb_varinfo;
	struct fb_fix_screeninfo fb_fixinfo;
	//
	if (ioctl(frame_buffer, FBIOGET_VSCREENINFO, &fb_varinfo) == -1) 
	{
		return ;	// if cannot get varinfo, return
	}
	//
	if (ioctl(frame_buffer, FBIOGET_FSCREENINFO, &fb_fixinfo) == -1) 
	{
		return ;	// if cannot get fixinfo, return
	}

	x_rec = fb_varinfo.xres_virtual;    // screen weight
	y_rec = fb_varinfo.yres_virtual;    // screen height
	size = fb_fixinfo.line_length;		// line_length
    // the address in our address space that now represents the contents of the file.
	frame_buffer_mem = (color_t*)mmap(NULL, y_rec*size, 
	PROT_READ|PROT_WRITE,
	MAP_SHARED, frame_buffer, 0);

    // disable the keypress echo buffering the keypresses
	struct termios terminal_setting;
	ioctl(STDIN_FILENO, TCGETS, &terminal_setting);
	terminal_setting.c_lflag &= ~ICANON;
	terminal_setting.c_lflag &= ~ECHO;
	ioctl(STDIN_FILENO, TCSETS, &terminal_setting);
	// clear screen
	clear_screen();
}



/**
 * put the stdin back and clear screen
 */
void exit_graphics() {
	struct termios terminal_setting;
	ioctl(STDIN_FILENO, TCGETS, &terminal_setting);
	terminal_setting.c_lflag |= ICANON;
	terminal_setting.c_lflag |= ECHO;
	ioctl(STDIN_FILENO, TCSETS, &terminal_setting);
	munmap(frame_buffer_mem, y_rec*size);
	close(frame_buffer);
	// clear_screen();
}


/**
 * set a helper function to draw letters
 */
char getkey() {
	fd_set fds;				// file descriptor
	FD_ZERO(&fds);			// clear the set for save
	FD_SET(0, &fds);		// add
	char key;
	struct timeval time;
	// need it to be zero to fix no image if no move
	time.tv_sec=0;
	time.tv_usec=0;
	if (select(STDIN_FILENO+1, &fds, NULL, NULL, &time) > 0
		&& read(STDIN_FILENO, &key, 1) > 0) {
			return key;
	}
}

/**
 *  sleep for  a while in ms
 */
void sleep_ms(long ms) {
	struct timespec time;
	time.tv_sec = ms / 1000;
	time.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&time, NULL);

}

/**
 * draw pixels
 */
void draw_pixel(int x, int y, color_t color) {

	// check if x is out of range
	if (x < 0 || x >= x_rec) {

		return;
	}
	// check if y is out of range
	if (y < 0 || y >= y_rec) {
		return;
	}
	int pos = x_rec * y + x;

	color_t* pixel = frame_buffer_mem + pos;
	*pixel = color;
}

/**
 * to draw a rectangle
 */
void draw_rect(int x, int y, int width, int height, color_t c) {
	// check if the rect is out of screen (out of file  range)
	// if (x < 0 || y < 0 || x + width > x_rec || y + height > y_rec) {
	// 	return;
	// }
	if (x < 0) {
		x = 0;
	} else if (x > x_rec - width) {
		x = x_rec - width;
	}
	if (y < 0) {
		y = 0;
	} else if (y > y_rec - height) {
		y = y_rec - height - 1;
	}
	int i, j;
	// draw top and bottom
	for (i = x; i < x + width; i++) {
		draw_pixel(i, y, c);
		draw_pixel(i, y + height, c);
	}
	// draw left and right
	for (j = y; j < y + height; j++){
		draw_pixel(x, j, c);
		draw_pixel(x+width, j, c);
	}
}

/**
 * draw a char
 */
void draw_char(int x, int y, char ch, color_t color) {
	int ch_ascii_code = (int)ch;
	int i = 0;
	// each y pixel line
	for (; i < 16; i++) {
		int j = 0;
		int current_pixel = iso_font[ch_ascii_code*16+i];
		// each x pixel line
		for (; j < 8;j++){

			if (current_pixel & 0x01) {
				draw_pixel(x+j, y+i, color);
			}
			current_pixel >>= 1;
		}
	}
}

/**
 * draw a text
 */
void draw_text(int x, int y, const char *text, color_t c) {
	int i = 0, ptr = 0;
	for (;text[i] != '\0'; i++) {
		draw_char(x,y,text[i], c);
		x+=8;		// move to the next char position
	}
}

/**
 * Draw a circle
 * Algorithm: Midpoint circle algorithm
 */
void draw_circle(int x0, int y0, int r, color_t c) {
	// if (x0 - r < 0 || x0 + r > x_rec || y0 - r < 0 || y0 + r > y_rec)
	// 	return;
	if (x0 - r < 0) {
		x0 = r;
	} else if (x0 + r > x_rec) {
		x0 = x_rec - r;
	}
	if (y0 - r < 0) {
		y0 = r;
	} else if (y0 + r > y_rec) {
		y0 = y_rec - r - 1;
	}
	
	int x = 0, y = r, d = 5 / 4 - r;

	while (x <= y) {
		// draw 8 pixels
		draw_pixel(x+x0, y+y0, c);
		draw_pixel(x+x0, y0-y, c);
		draw_pixel(x0-x, y+y0, c);
		draw_pixel(x0-x, y0-y, c);

		draw_pixel(y+x0, y0+x, c);
		draw_pixel(y+x0, y0-x, c);
		draw_pixel(x0-y, x+y0, c);
		draw_pixel(x0-y, y0-x, c);

		// sides
		if (d >= 0) {
			y -= 1;
			d = d + (2 *(x-y)) + 5;
		} else {
			d = d + 2*x + 3;
		}
		x+=1;
	}

}

