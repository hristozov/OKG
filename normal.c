#include "buffer.h"
#include "normal.h"

/* Изчислява нормален вектор */
void calculate_normal(struct point *start, struct point *end1, struct point *end2, struct point *normal) {  
	struct point vector1, vector2;
	
	/* Първият вектор */
	vector1.x = end1->x - start->x;
	vector1.y = end1->y - start->y;
	vector1.z = end1->z - start->z;
	
	/* Вторият вектор */
	vector2.x = end2->x - start->x;
	vector2.y = end2->y - start->y;
	vector2.z = end2->z - start->z;
	
	/* Векторното им произведение */
	normal->x = vector1.y*vector2.z - vector1.z*vector2.y;
	normal->y = vector1.z*vector2.x - vector1.x*vector2.z;
	normal->z = vector1.x*vector2.y - vector1.y*vector2.x;
}

#ifdef SMOOTH_SHADING

/*
 * Изчислява нормален вектор на даден връх като средно аритметично на нормалните вектори на полигоните, в които участва.
 * Съседството на върховете е дадено по схемата:
 * i+1 j-1 | i+1 j | i+1 j+1
 * i   j-1 | i   j | i   j+1
 * i-1 j-1 | i-1 j | i-1 j+1
 */
void vertex_normal(size_t i, size_t j) {
	struct vertex *cur = &vertex_buffer[i][j];
	size_t nelem = 0;
	float sum_x = 0, sum_y = 0, sum_z = 0;
	float len = 0.f;
	
	/* Невалидни стойности за размера на буфера */
	if (i >= buffer_size || j >= no_segments)
		return;
		
	for (int i=0; i<4; i++) {
		if (cur->p[i] == NULL)
			break;
		sum_x += cur->p[i]->normal.x;
		sum_y += cur->p[i]->normal.y;
		sum_z += cur->p[i]->normal.z;
		nelem++;
	}
	
	cur->normal.x = sum_x/nelem;
	cur->normal.y = sum_y/nelem;
	cur->normal.z = sum_z/nelem;
	
	len = VECTOR_LENGTH(cur->normal);
	
	if (len == 0.f) /* Don't try to normalize with length zero */
		return;
	
	cur->normal.x /= len;
	cur->normal.y /= len;
	cur->normal.z /= len;
}

/* Изчислява и записва в буфера нормалните вектори на всички върхове */
void calculate_vertex_normals() {
	printf("Calculating vertex normals...\n");
	for (size_t i=0; i < buffer_size; i++)
		for (size_t j=0; j < no_segments; j++)
			vertex_normal(i, j);
}

#endif
