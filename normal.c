#include "buffer.h"
#include "normal.h"

/* Изчислява нормален вектор като произведение на два вектора с еднакво начало и два края */
void calculate_normal(struct point *start, struct point *end1, struct point *end2, struct point *normal) {  
	struct point vector1, vector2;
	float len;
	
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
	
	len = VECTOR_LENGTH(*normal);
	
	if (len == 0.f) /* Не нормираме при нулева дължина */
		return;
	
	/* Нормираме */
	normal->x /= len;
	normal->y /= len;
	normal->z /= len;
}

#if SMOOTH_SHADING == 1 /* Функциите не са ни нужни при flat shading */

/*
 * Изчислява нормален вектор на даден връх като нормирано средно аритметично на нормалните вектори на полигоните, в които участва.
 * Съседството на върховете е дадено по схемата:
 * i+1 j-1 | i+1 j | i+1 j+1
 * i   j-1 | i   j | i   j+1
 * i-1 j-1 | i-1 j | i-1 j+1
 */
void vertex_normal(size_t i, size_t j) {
	struct vertex *cur = &vertex_buffer[i][j];
	struct polygon *cur_p = NULL;
	size_t nelem = 0;
	float sum_x = 0, sum_y = 0, sum_z = 0;
	float len = 0.f;
	
	/* Невалидни стойности за размера на буфера */
	if (i >= buffer_size || j >= no_segments)
		return;
	
	if ((cur_p = GET_LL_P(i, j)) != NULL) {
		sum_x += cur_p->normal.x;
		sum_y += cur_p->normal.y;
		sum_z += cur_p->normal.z;
		nelem++;
	}
	
	if ((cur_p = GET_UL_P(i, j)) != NULL) {
		sum_x += cur_p->normal.x;
		sum_y += cur_p->normal.y;
		sum_z += cur_p->normal.z;
		nelem++;
	}
	
	if ((cur_p = GET_LR_P(i, j)) != NULL) {
		sum_x += cur_p->normal.x;
		sum_y += cur_p->normal.y;
		sum_z += cur_p->normal.z;
		nelem++;
	}
	
	if ((cur_p = GET_UR_P(i, j)) != NULL) {
		sum_x += cur_p->normal.x;
		sum_y += cur_p->normal.y;
		sum_z += cur_p->normal.z;
		nelem++;
	}
	
	/* Намираме средните стойности на координатите и ги записваме в normal */
	cur->normal.x = sum_x/nelem;
	cur->normal.y = sum_y/nelem;
	cur->normal.z = sum_z/nelem;
	
	/* Дължината на вектора, която ще ни служи за нормиране */
	len = VECTOR_LENGTH(cur->normal);
	
	if (len == 0.f) /* Не нормираме при нулева дължина */
		return;
	
	/* Нормираме */
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
