#ifndef library_header
#define library_header



typedef unsigned short color_t;

void init_graphics();

void exit_graphics();

char getKey();

void sleep_ms(long ms);

void clear_screen();

void draw_pixel(int x, int y, color_t color);

void draw_rect(int x1, int y1, int width, int height, color_t color);

void draw_text(int x, int y, const char*text, color_t color);

void draw_circle(int x, int y, int r, color_t color);

void draw_line(int x1, int y1, int x2, int y2, color_t color);



#endif
