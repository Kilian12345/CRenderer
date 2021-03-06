#ifndef MESH_H
#define MESH_H

#include "Vector.h"
#include "Triangle.h"
#include "Array.h"

#define N_CUBE_VERTICES 8 
extern vec3_t cube_vertices[N_CUBE_VERTICES];


#define N_CUBE_FACES (6 * 2) // 6 cube faces -> 2 triangles per face
extern face_t cube_faces[N_CUBE_FACES];



/// <summary>
/// ///// define a struct for dynamic size meshes, with array of vertices and faces
/// </summary>

typedef struct
{
	vec3_t* vertices;	// dynamic array of vertices
	face_t* faces;		// dynamic array of faces
	vec3_t rotation;	// rotation with x, y, z values
	vec3_t scale;		// scale with x, y, z values
	vec3_t translation;	// translation with x, y, z values
} mesh_t;

extern mesh_t mesh;

void load_cube_mesh_data(void);
void load_obj_file_data(char* filename);


#endif
