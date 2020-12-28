#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>

extern SDL_Window* window;
extern SDL_Renderer* renderer;

extern int window_width;
extern int window_height;

extern uint32_t* color_buffer;
extern SDL_Texture* color_buffer_texture;


bool initialize_window(void);
void draw_grid(int grid_size);
void draw_rect(int x_param, int y_param, int width_param, int height_param, uint32_t color);
void draw_pixel(int x_param, int y_param, uint32_t color);
void render_color_buffer(void);
void clear_color_buffer(uint32_t color);
void destroy_window(void);

#endif
