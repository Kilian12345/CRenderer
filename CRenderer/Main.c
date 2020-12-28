#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "Display.h"
#include "Vector.h"

const int N_POINTS = 9*9*9;
vec3_t cube_points[9 * 9 * 9]; // 9x9x9 cube
vec2_t projected_points[9 * 9 * 9];

vec3_t camera_position = { 0, 0, -5 };
vec3_t cube_rotation = { .x = 0, .y = 0, .z = 0 };


float fov_factor = 640;

bool is_running = false;


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

	//Start Loading my array of vectors
	//From -1 to 1 (in this 9x9x9 cube)

	int point_count = 0;

	for (float x = -1; x <= 1; x += 0.25f)
	{
		for (float y = -1; y <= 1; y += 0.25f)
		{
			for (float z = -1; z <= 1; z += 0.25f)
			{
				vec3_t new_point = { x, y, z };
				cube_points[point_count++] = new_point;
			}
		}
	}
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

// Function that receive a 3D vector and returns a projected 2D point
vec2_t project(vec3_t point)
{

	vec2_t projected_point =
	{
		.x = (fov_factor * point.x) / point.z,
		.y = (fov_factor * point.y) / point.z
	};

	return projected_point;
}

void update()
{
	cube_rotation.x += 0.005;
	cube_rotation.y += 0.005;
	cube_rotation.z += 0.005;

	for (int i = 0; i < N_POINTS; i++)
	{
		vec3_t point = cube_points[i];

		vec3_t transformed_point = vec3_rotate_x(point, cube_rotation.x);
		transformed_point = vec3_rotate_y(transformed_point, cube_rotation.y);
		transformed_point = vec3_rotate_z(transformed_point, cube_rotation.z);

		// Translate the points away from the camera
		transformed_point.z -= camera_position.z;


		vec2_t projected_point = project(transformed_point);

		// Save the projected 2D vector array of projected points
		projected_points[i] = projected_point;
	}
}


void render()
{

	draw_grid(20);

	for (int i = 0; i < N_POINTS; i++)
	{
		vec2_t projected_point = projected_points[i];
		draw_rect(
			projected_point.x + (window_width * 0.5f),
			projected_point.y + (window_height * 0.5f),
			4,
			4,
			0xFFFFFF00
		);
	}

	render_color_buffer();
	clear_color_buffer(0xFF000000);


	SDL_RenderPresent(renderer);
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