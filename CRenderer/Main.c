#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "Array.h"
#include "Display.h"
#include "Matrix.h"
#include "Vector.h"
#include "Mesh.h"


triangle_t* triangles_to_render = NULL;

vec3_t camera_position = { 0, 0, 0 };
//vec3_t cube_rotation = { .x = 0, .y = 0, .z = 0 };


float fov_factor = 640;

bool is_running = false;
int previous_frame_time = 0;

void setup(void)
{
	render_method = RENDER_WIRE;
	cull_method = CULL_BACKFACE;

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

	// Loads the cube values in the mesh data structure
	load_cube_mesh_data();
	//load_obj_file_data("./mesh/AK47.obj");
	//load_obj_file_data("./mesh/cube.obj");

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
				is_running = false;
			if (event.key.keysym.sym == SDLK_1)
				render_method = RENDER_WIRE_VERTEX;
			if (event.key.keysym.sym == SDLK_2)
				render_method = RENDER_WIRE;
			if (event.key.keysym.sym == SDLK_3)
				render_method = RENDER_FILL_TRIANGLE;
			if (event.key.keysym.sym == SDLK_4)
				render_method = RENDER_FILL_TRIANGLE_WIRE;
			if (event.key.keysym.sym == SDLK_c)
				cull_method = CULL_BACKFACE;
			if (event.key.keysym.sym == SDLK_v)
				cull_method = CULL_NONE;
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
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
		SDL_Delay(time_to_wait);

	previous_frame_time = SDL_GetTicks();

	// Initilaze the array of triangle to render
	triangles_to_render = NULL;

	// Change the mesh scale, rotation values per animation frames
	mesh.rotation.x += 0.01f;
	mesh.rotation.y += 0.01f;
	mesh.rotation.z += 0.01f;
	mesh.scale.x += 0.002f;
	mesh.scale.y += 0.001f;
	mesh.translation.x += 0.01;
	mesh.translation.z = 7.0;

	// Create a scale matrix that will be used to multiply the mesh vertices
	mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
	mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
	mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

	// Loop all triangle faces of our mesh
	int num_faces = array_length(mesh.faces);
	for (int i = 0; i < num_faces; i++)
	{
		face_t mesh_face = mesh.faces[i];

		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		vec4_t transformed_vertices[3];

		//Loop all three vertices of this current face and apply transformation
		for (int j = 0; j < 3; j++)
		{
			vec4_t  transformed_vertex = vec4_from_vec3(face_vertices[j]);
			
			// Create a world matrix combining scale, rotation, translation
			mat4_t world_matrix = mat4_identity();

			// Order matters : First Scale, then rotate, then translate 
			world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
			world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
			world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

			transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);


			//Save transformed vertices
			transformed_vertices[j] = transformed_vertex;
		}

		///////////////////////////////////////////////////////////////Check Backface Culling
		if (cull_method == CULL_BACKFACE)
		{
			vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
			vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
			vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);

			vec3_t vector_ab = vec3_sub(vector_b, vector_a);
			vec3_t vector_ac = vec3_sub(vector_c, vector_a);
			vec3_normalize(&vector_ab);
			vec3_normalize(&vector_ac);

			//Compute the face normal using the cross product to find the perpendicular
			vec3_t normal = vec3_cross(vector_ab, vector_ac);

			//Normalize the face normal vector
			vec3_normalize(&normal);

			//Find a vector between a point in the triangle and the camera origin
			vec3_t camera_ray = vec3_sub(camera_position, vector_a);

			// Calculate how aligned is the camera ray with the normal
			float dot_normal_camera = vec3_dot(camera_ray, normal);

			if (dot_normal_camera < 0) continue;

		}
		//-- end of back face culling



		vec2_t projected_points[3];

		// Loop all three vertices to perform projection
		for(int j = 0; j < 3; j++)
		{
			// Project the current vertex
			projected_points[j] = project(vec3_from_vec4(transformed_vertices[j]));

			//Scale and translate the projected point in the middle of the screen
			projected_points[j].x += (window_width / 2);
			projected_points[j].y += (window_height / 2);
		}

		// Calculate the average depth for each face based on the vertices after transformation
		float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3;

		triangle_t projected_triangle =
		{
			.points = 
			{
				{projected_points[0].x, projected_points[0].y},
				{projected_points[1].x, projected_points[1].y},
				{projected_points[2].x, projected_points[2].y}
			},
			.color = mesh_face.color,
			.avg_depth = avg_depth
		};

		// Save the projected triangle in the array of triangle to render
		//triangles_to_render[i] = projected_triangle;
		array_push(triangles_to_render, projected_triangle);
	}

	// Sort the triangles to render by their avg_depth
	int num_triangles = array_length(triangles_to_render);
	for (int i = 0; i < num_triangles; i++)
	{
		for (int j = i; j < num_triangles; j++)
		{
			if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth)
			{
				// Swap the triangles psoitions in the array
				triangle_t temp = triangles_to_render[i];
				triangles_to_render[i] = triangles_to_render[j];
				triangles_to_render[j] = temp;
			}
		}
	}
}


void render()
{

	draw_grid(10);

	int num_triangles = array_length(triangles_to_render);
	for (int i = 0; i < num_triangles; i++)
	{
		triangle_t triangle = triangles_to_render[i];
		
		if (render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE)
		{
			draw_filled_triangle(
				triangle.points[0].x,
				triangle.points[0].y,
				triangle.points[1].x,
				triangle.points[1].y,
				triangle.points[2].x,
				triangle.points[2].y,
				triangle.color
			);
		}

		if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE)
		{
			draw_triangle(
				triangle.points[0].x,
				triangle.points[0].y,
				triangle.points[1].x,
				triangle.points[1].y,
				triangle.points[2].x,
				triangle.points[2].y,
				0xFFFFFFFF
			);
		}

		if (render_method == RENDER_WIRE_VERTEX )
		{
			// Draw Vertices
			draw_rect(triangle.points[0].x - 2, triangle.points[0].y - 2, 4, 4, 0xFFFF0000);
			draw_rect(triangle.points[1].x - 2, triangle.points[1].y - 2, 4, 4, 0xFFFF0000);
			draw_rect(triangle.points[2].x - 2, triangle.points[2].y - 2, 4, 4, 0xFFFF0000);
		}
	}

	//Clear the array of triangles to render every frame loop
	array_free(triangles_to_render);

	render_color_buffer();
	clear_color_buffer(0xFF000000);


	SDL_RenderPresent(renderer);
}

//Free memory that was dynamically allocated by the program
void free_resources(void)
{
	free(color_buffer);
	array_free(mesh.faces);
	array_free(mesh.vertices);
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
	free_resources();

	return 0; 
}