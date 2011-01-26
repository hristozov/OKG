#include "buffer.h"
#include "normal.h"

/* Изчислява нормален вектор */
void calculateNormal(struct point *start, struct point *end1, struct point *end2, struct point *normal) {  
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
void vertexNormal(size_t i, size_t j) {
	struct point normal1, normal2, normal3, normal4;
	
	/* Невалидни стойности за размера на буфера */
	if (i >= buffer_size-1 || j >= buffer_size-1)
		return;
		
	if (i == 0 || j == 0) {
		calculateNormal(&vertex_buffer[i][j].coord, &vertex_buffer[i+1][j].coord, &vertex_buffer[i][j+1].coord, &vertex_buffer[i][j].normal);
		return;
	}
		
	/* Изчисляваме нормалните вектори на полигоните, в които участва върхът [i][j] и ги записваме в normal[1-4] */
	calculateNormal(&vertex_buffer[i][j].coord, &vertex_buffer[i][j-1].coord, &vertex_buffer[i+1][j].coord, &normal1);
	calculateNormal(&vertex_buffer[i][j].coord, &vertex_buffer[i+1][j].coord, &vertex_buffer[i][j+1].coord, &normal2);
	calculateNormal(&vertex_buffer[i][j].coord, &vertex_buffer[i][j+1].coord, &vertex_buffer[i-1][j].coord, &normal3);
	calculateNormal(&vertex_buffer[i][j].coord, &vertex_buffer[i][j-1].coord, &vertex_buffer[i-1][j].coord, &normal4);
	
	/* Записваме в буфера средните стойности на normal[1-4]*/
	vertex_buffer[i][j].normal.x = (normal1.x + normal2.x + normal3.x + normal4.x) * .25f;
	vertex_buffer[i][j].normal.y = (normal1.y + normal2.y + normal3.y + normal4.y) * .25f;
	vertex_buffer[i][j].normal.z = (normal1.z + normal2.z + normal3.z + normal4.z) * .25f;
}

/* Изчислява и записва в буфера нормалните вектори на всички върхове */
void calculateVertexNormals() {
	printf("Calculating vertex normals...\n");
	for (int i=0; i < buffer_size; i++)
		for (int j=0; j < no_segments; j++)
			vertexNormal(i, j);
}

#endif
