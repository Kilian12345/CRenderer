#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>

bool is_running = false;
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

void setup(void)
{
	// Allocate the required memory in bytes to hold the color buffer
	color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

	// Creating a SDL texture that i sused to display the color buffer
	color_buffer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		window_width,
		window_height
	);
}

void process_input()
{
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type)
	{
		case SDL_QUIT:
			is_running = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				is_running = false;
			}
			break;
	}
}

void update()
{

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

void draw_rect(int x_param, int y_param, int width_param, int height_param, uint32_t color, int density_param)
{
	for (int y = 0; y < height_param; y += density_param)
	{
		for (int x = 0; x < width_param; x += density_param)
		{
			int current_x = x + x_param;
			int current_y = y + y_param;

			color_buffer[(window_width * current_y) + current_x] = color;
		}
	}
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

void render()
{
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderClear(renderer);

	draw_grid(20);
	draw_rect(100, 100, 400, 150, 0xFFFF00FF, 3);

	render_color_buffer();
	clear_color_buffer(0xFF000000);


	SDL_RenderPresent(renderer);
}

void destroy_window(void)
{
	free(color_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	/* Create SDL Window */
	is_running = initialize_window();
	
	setup();

	while (is_running)
	{
		process_input();
		update();
		render();
	}


	destroy_window();

	return 0; 
}