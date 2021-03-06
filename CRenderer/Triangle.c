#include "Triangle.h"
#include "Display.h"


void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
	// Find the two slopes (two triangle legs)
	float inv_slope_1 = (float)(x1 -x0) / (float)(y1 - y0);
	float inv_slope_2 = (float)(x2 - x0) / (float)(y2 - y0);

	// Start x_start and x_end from the top vertex (x0,y0)
	float x_start = x0;
	float x_end = x0;

	// Loop all the scanlines from top to bottom
	for (int y = y0; y <= y2; y++)
	{
		draw_line(x_start, y, x_end, y, color);
		x_start += inv_slope_1;
		x_end += inv_slope_2;
	}
} 

void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
	// Find the two slopes (two triangle legs)
	float inv_slope_1 = (float)(x2 - x0) / (float)(y2 - y0);
	float inv_slope_2 = (float)(x2 - x1) / (float)(y2 - y1);

	// Start x_start and x_end from the bottom vertex (x2,y2)
	float x_start = x2;
	float x_end = x2;

	// Loop all the scanlines from bottom to top
	for (int y = y2; y >= y0; y--)
	{
		draw_line(x_start, y, x_end, y, color);
		x_start -= inv_slope_1;
		x_end -= inv_slope_2;
	}
}

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
	// Find the vectors between the vertices ABC and point p
	vec2_t ab = vec2_sub(b, a);
	vec2_t bc = vec2_sub(c, b);
	vec2_t ac = vec2_sub(c, a);
	vec2_t ap = vec2_sub(p, a);
	vec2_t bp = vec2_sub(p, b);

	// Calcualte the area of the full triangle ABC using cross product (area of parallelogram)
	float area_triangle_abc = (ab.x * ac.y - ab.y * ac.x);

	// Weight alpha is the area of subtriangle BCP divided by the area of the full triangle ABC
	float alpha = (bc.x * bp.y - bp.x * bc.y) / area_triangle_abc;

	// Weight beta is the area of subtriangle ACP divided by the area of the full triangle ABC
	float beta = (ap.x * ac.y - ac.x * ap.y) / area_triangle_abc;

	// Weight gamma is easily found since barycentric cooordinates always add up to 1
	float gamma = 1 - alpha - beta;

	vec3_t weights = { alpha, beta, gamma };
	return weights;
}


// Function to draw the textured pixel at position x / y using interpolation
draw_texel(int x,int y, uint32_t* texture,
	vec2_t point_a, vec2_t point_b, vec2_t point_c,
	float u0, float v0, float u1, float v1, float u2, float v2)
{
	vec2_t point_p = { x,y };
	vec3_t weights = barycentric_weights(point_a, point_b, point_c, point_p);

	float alpha = weights.x;
	float beta = weights.y;
	float gamma = weights.z;

	// Perforl the interpolation of all U and V values using barycentric weights
	float interpolated_u = u0 * alpha + u1 * beta + u2 * gamma;
	float interpolated_v = v0 * alpha + v1 * beta + v2 * gamma;

	// Map the UV coord to the full texture
	int tex_x = abs((int)(interpolated_u * texture_width));
	int tex_y = abs((int)(interpolated_v * texture_height));

	printf("Tex_widht + tex_y = %d || tex_x = %d \n", (texture_width * tex_y), tex_x);

	if ((texture_width * tex_y) + tex_x < 4096)
	{
		draw_pixel(x, y, texture[(texture_width * tex_y) + tex_x]);
	}
}

void draw_textured_triangle(
	int x0, int y0, float u0, float v0,
	int x1, int y1, float u1, float v1,
	int x2, int y2, float u2, float v2,
	uint32_t* texture)
{
	// Loop all the pixel of the triangle to render them based on the color that comes from the texture
	// We need to sort hte vertices by their Y coord

	if (y0 > y1)
	{
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
		float_swap(&u0, &u1);
		float_swap(&v0, &v1);
	}
	if (y1 > y2)
	{
		int_swap(&y1, &y2);
		int_swap(&x1, &x2);
		float_swap(&u1, &u2);
		float_swap(&v1, &v2);
	}
	if (y0 > y1)
	{
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
		float_swap(&u0, &u1);
		float_swap(&v0, &v1);
	}

	// create vector points after we sort vertices
	vec2_t point_a = { x0, y0 };
	vec2_t point_b = { x1, y1 };
	vec2_t point_c = { x2, y2 };

	/////////////////////////////////////////////////////
	// Render the upper part of the triangle (flat-bottom)
	/////////////////////////////////////////////////////
	float inv_slope1 = 0;
	float inv_slope2 = 0;

	if( y1 - y0 != 0) inv_slope1 = (float)(x1 - x0) / abs(y1 - y0);
	if (y2 - y0 != 0) inv_slope2 = (float)(x2 - x0) / abs(y2 - y0);

	if (y1 - y0 != 0)
	{
		for (int y = y0; y <= y1; y++)
		{
			int x_start = x1 + (y - y1) * inv_slope1;
			int x_end = x0 + (y - y0) * inv_slope2;

			if (x_end < x_start)
			{
				int_swap(&x_start, &x_end); // swap if x_start is to the right of x_end
			}

			for (int x = x_start; x < x_end; x++)
			{
				draw_texel(x, y, texture, point_a, point_b,  point_c, u0, v0, u1, v1, u2, v2);
			}
		}
	}

	/////////////////////////////////////////////////////
	// Render the bottom part of the triangle (flat-bottom)
	/////////////////////////////////////////////////////

	inv_slope1 = 0;
	inv_slope2 = 0;

	if (y2 - y1 != 0) inv_slope1 = (float)(x2 - x1) / abs(y2- y1);
	if (y2 - y0 != 0) inv_slope2 = (float)(x2 - x0) / abs(y2 - y0);

	if (y2 - y1 != 0)
	{
		for (int y = y1; y <= y2; y++)
		{
			int x_start = x1 + (y - y1) * inv_slope1;
			int x_end = x0 + (y - y0) * inv_slope2;

			if (x_end < x_start)
			{
				int_swap(&x_start, &x_end); // swap if x_start is to the right of x_end
			}

			for (int x = x_start; x < x_end; x++)
			{
				draw_texel(x, y, texture, point_a, point_b, point_c, u0, v0, u1, v1, u2, v2);
			}
		}
	}
}

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
	//We nned to sort the vertices by y_coordinate ascending (y0 < y1 < y2)

	if (y0 > y1)
	{
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
	}
	if (y1 > y2)
	{
		int_swap(&y1, &y2);
		int_swap(&x1, &x2);
	}
	if (y0 > y1)
	{
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
	}

	if (y1 == y2)
	{
		// We can simply draw the flat_bottom triangle
		fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
	}
	else if (y0 == y1)
	{
		// We can simply draw the flat_top triangle
		fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
	}
	else
	{

		// Calculate the new vertex (Mx,My) using triangle similarity
		int My = y1;
		int Mx = ((float)((x2 - x0) * (y1 - y0)) / (float)(y2 - y0)) + x0;

		// Draw flat_bottom triangle
		fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

		// Draw flat_top triangle
		fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
	}
}
