#include "buffer.h"
#include "normal.h"

//extern size_t no_segments;

/* Инициализация на vertex буфера */
void buffer_init() {
	buffer_size = 0;
	vertex_buffer = NULL;
	
	polygon_size = 0;
	polygon_buffer = NULL;
}

/* Преоразмеряване на vertex буфера */
void vertex_buffer_resize(size_t new_size) {
	printf("buffer_resize() - old size: %lu new size: %lu\n", buffer_size, new_size);
	if (new_size == 0) {
		buffer_kill();
		return;
	}
	
	if (new_size < buffer_size) {
		for (size_t i = buffer_size; i > new_size; i--) {
			free(vertex_buffer[i-1]);
		}
			
		if ((vertex_buffer = (struct vertex**) realloc(vertex_buffer, new_size * sizeof(struct vertex*))) == NULL)
			exit(-1);
			
		buffer_size = new_size;
	}
	
	if (new_size > buffer_size) {
		if ((vertex_buffer = (struct vertex**) realloc(vertex_buffer, new_size * sizeof(struct vertex*))) == NULL)
			exit(-1);
		
		for (size_t i = buffer_size; i < new_size; i++) {
			if ((vertex_buffer[i] = (struct vertex*) calloc(no_segments, sizeof(struct vertex))) == NULL)
				exit(-1);
		}
		
		buffer_size = new_size;
	}
}

/* Освобождаване на паметта */
void buffer_kill() {
	for (int i=0; i < buffer_size; i++) {
		free(vertex_buffer[i]);
	}
	free(vertex_buffer);
	vertex_buffer = NULL;
	buffer_size = 0;
	
	free(polygon_buffer);
	polygon_size = 0;
}

/* 
 * Функцията add_point взима за аргумент контролна точка от образуващата
 * След това тя изчислява и записва точки, които чрез правилен многоъгълник апроксимират окръжност около оста на ротация
 * Оста има координати (0, y, 0)
 */
void add_point(float x, float y, float z) {
	size_t cur_index = buffer_size;
	vertex_buffer_resize(buffer_size + 1);
	
	printf("Adding point %lu (%f %f %f)\n", cur_index, x, y, z);
	
	/* Разстоянието от оста */
	float r = sqrtf(x*x + z*z);
	
	for (size_t i = 0; i < no_segments; i++) {
		/* Ъгълът на текущия сегмент */
		float angle = (2.0f * M_PI * (float)i) / (float)no_segments;
		
		/* 
		 * Изчисляваме координатите на текущата точка
		 * Променяме само x и z, защото апроксимираме фигура около оста с координати (0, y, 0)
		 */
		vertex_buffer[cur_index][i].coord.x = r * cosf(angle);
		vertex_buffer[cur_index][i].coord.y = y;
		vertex_buffer[cur_index][i].coord.z = r * sinf(angle);
		
		//printf("POINT: %f %f %f DISTANCE FROM 0: %f\n", vertex_buffer[cur_index][i].x, vertex_buffer[cur_index][i].y, vertex_buffer[cur_index][i].z, sqrtf(vertex_buffer[cur_index][i].x*vertex_buffer[cur_index][i].x + vertex_buffer[cur_index][i].y*vertex_buffer[cur_index][i].y + vertex_buffer[cur_index][i].z*vertex_buffer[cur_index][i].z));
	}
	
	#ifdef SMOOTH_SHADING
		calculateVertexNormals();
	#endif
	fill_polygon_buffer();
}

/* fill_polygon_buffer() запълва буфера за полигони с върховете им и нормалните им вектори */
void fill_polygon_buffer() {
	printf("Call to fill_polygon_buffer()\n");
	size_t index;
	struct polygon *cur;
	if (polygon_buffer != NULL)
		free (polygon_buffer);
		
	polygon_size = (buffer_size-1) * (no_segments-1);
	
	if (polygon_size == 0) /* Нямаме достатъчно точки за построяване на полигона */
		return;
		
	polygon_buffer = malloc (polygon_size * sizeof(struct polygon));
	
	for (size_t i=0; i < buffer_size-1; i++)
		for (size_t j=0; j < no_segments-1; j++) {
			index = i*(no_segments-1) + j;
			cur = &polygon_buffer[index];
			cur->p[0] = &vertex_buffer[i][j];
			cur->p[1] = &vertex_buffer[i+1][j];
			cur->p[2] = &vertex_buffer[i][j+1];
			cur->p[3] = &vertex_buffer[i+1][j+1];
			calculateNormal(&cur->p[0]->coord, &cur->p[1]->coord, &cur->p[2]->coord, &cur->normal);
		}
}
