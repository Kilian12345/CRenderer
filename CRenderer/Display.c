#include "Display.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int window_width = 800;
int window_height = 600;

uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture = NULL;

bool initialize_window(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initializing SDL");
		return false;
	}

	// Set widht and height of the SDL_Window with the max screen resolution
	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);
	window_width = display_mode.w;
	window_height = display_mode.h;

	// Create SDL Window
	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		window_width,
		window_height,
		SDL_WINDOW_BORDERLESS
	);
	if (!window)
	{
		fprintf(stderr, "Error Creating SDL Window");
		return false;
	}


	// Create SDL Renderer
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer)
	{
		fprintf(stderr, "Error Creating SDL Renderer");
		return false;
	}

	return true;
}

void draw_grid(int grid_size)
{
	for (int y = 0; y < window_height; y += grid_size)
	{
		for (int x = 0; x < window_width; x += grid_size)
		{
			color_buffer[(window_width * y) + x] = 0xFFFFFFFF;
		}
	}
}

void draw_rect(int x_param, int y_param, int width_param, int height_param, uint32_t color)
{
	for (int y = 0; y < height_param; y++)
	{
		for (int x = 0; x < width_param; x++)
		{
			int current_x = x + x_param;
			int current_y = y + y_param;

			draw_pixel(current_x, current_y, color);
		}
	}
}

void draw_pixel(int x_param, int y_param, uint32_t color)
{
	if(x_param >= 0 && x_param < window_width && y_param >= 0 && y_param < window_height)
		color_buffer[(window_width * y_param) + x_param] = color;
}

void render_color_buffer(void)
{
	SDL_UpdateTexture(
		color_buffer_texture,
		NULL,
		color_buffer,
		(int)(window_width * sizeof(uint32_t))
	);

	SDL_RenderCopy(
		renderer,
		color_buffer_texture,
		NULL,
		NULL
	);
}

void clear_color_buffer(uint32_t color)
{
	for (int y = 0; y < window_height; y++)
	{
		for (int x = 0; x < window_width; x++)
		{
			color_buffer[(window_width * y) + x] = color;
		}
	}

}

void destroy_window(void)
{
	free(color_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
