#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Vector.h"

typedef struct
{
	vec2_t points[3];
} triangle_t;

typedef struct
{
	int a;
	int b;
	int c;
} face_t;

#endif