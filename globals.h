#pragma once
#include <stdlib.h> /* size_t */

/* no_segments определя колко страни трябва да имат многоъгълниците, с които апроксимираме окръжностите около оста */
extern size_t no_segments;

struct point {
	float x,y,z;
};

struct vertex {
	struct point *coord;
	struct point *normal;
};

/* Глобалният буфер, в който съхраняваме всички vertex-и */
extern size_t buffer_size;
extern struct vertex **vertex_buffer;
