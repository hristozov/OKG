#include "buffer.h"
#include "normal.h"

/* Инициализация на буферите */
void buffer_init() {
	buffer_size = 0;
	vertex_buffer = NULL;
	
	polygon_buffer = NULL;
}

/* Преоразмеряване на vertex буфера */
void buffer_resize(size_t new_size_v) {
	printf("buffer_resize_v() - old size: %lu new size: %lu\n", buffer_size, new_size_v);
	
	/* Ако новият размер е нула, просто унищожаваме буфера.
	 * Буферът с полигоните също ще бъде премахнат, но той така или иначе е зависим от вертексния */
	if (new_size_v == 0) {
		buffer_kill();
		return;
	}
	
	/* Само уголемяваме :) */
	if (new_size_v <= buffer_size) 
		return;
	
	/* Увеличаваме буфера */
	if (new_size_v > buffer_size) {
		/* Реалокираме буфера с новия размер */
		if ((vertex_buffer = (struct vertex**) realloc(vertex_buffer, new_size_v * sizeof(struct vertex*))) == NULL)
			exit(-1);
			
		/* Добавяме всеки от елементите (масиви от vertex структури) на буфера */
		for (size_t i = buffer_size; i < new_size_v; i++) {
			/* Заделяме памет за масив от vertex-и */
			if ((vertex_buffer[i] = (struct vertex*) calloc(no_segments, sizeof(struct vertex))) == NULL)
				exit(-1);
		}
	}
	
	if (new_size_v > 1) {
		size_t new_size_p = new_size_v - 1;
		if ((polygon_buffer = (struct polygon**) realloc(polygon_buffer, new_size_p * sizeof(struct polygon*))) == NULL)
			exit(-1);
		
		for (size_t i = P_SIZE; i < new_size_p; i++) {
			if ((polygon_buffer[i] = (struct polygon*) calloc(no_segments, sizeof(struct polygon))) == NULL)
				exit(-1);
		}
	}
	
	/* Записваме новия размер */
	buffer_size = new_size_v;
}

/* Освобождаване на паметта */
void buffer_kill() {
	for (size_t i=0; i < V_SIZE; i++)
		free(vertex_buffer[i]);
		
	for (size_t i=0; i < P_SIZE; i++)
		free(polygon_buffer[i]);
		
	free(vertex_buffer);
	vertex_buffer = NULL;
	buffer_size = 0;
	
	free(polygon_buffer);
	polygon_buffer = NULL;
}

/* 
 * Функцията add_point взима за аргумент контролна точка от образуващата
 * След това тя изчислява и записва точки, които чрез правилен многоъгълник апроксимират окръжност около оста на ротация
 * Оста има координати (0, y, 0)
 */
void add_point(float x, float y, float z) {
	/* Проверяваме дали точката вече не е добавена и излизаме, ако това е вярно */
	for (size_t i=0; i < V_SIZE; i++)
		if (vertex_buffer[i][0].coord.x == x && vertex_buffer[i][0].coord.y == y && vertex_buffer[i][0].coord.z == z)
			return;
			
	/* Изчисляваме текущия индекс в буфера и преоразмеряваме */
	size_t cur_index = V_SIZE;
	buffer_resize(V_SIZE + 1);
	
	printf("Adding point %lu (%f %f %f)\n", cur_index, x, y, z);
	
	/* Разстоянието от оста */
	float r = sqrtf(x*x + z*z);
	
	for (size_t i = 0; i < no_segments; i++) {
		/* Ъгълът на текущия сегмент */
		float angle = (2.0f * M_PI * (float)i) / (float)no_segments;
		
		/* Изчисляваме координатите на текущата точка
		 * Променяме само x и z, защото апроксимираме фигура около оста с координати (0, y, 0) */
		vertex_buffer[cur_index][i].coord.x = r * cosf(angle);
		vertex_buffer[cur_index][i].coord.y = y;
		vertex_buffer[cur_index][i].coord.z = r * sinf(angle);
	}
	
	#if SORT_CONTROL_POINTS == 1
		sort_control_points();
	#endif
	
	/* Винаги първо запълваме буфера с полигоните.
	 * Там се намират нормалните им вектори, които по-късно ще послужат при евентуално ползване на calculate_vertex_normals() */
	fill_polygon_buffer();
	#if SMOOTH_SHADING == 1
		calculate_vertex_normals(); /* Изчисляваме нормалните вектори на всеки връх */
	#endif
}

/* fill_polygon_buffer() запълва буфера за полигони с върховете им и нормалните им вектори */
void fill_polygon_buffer() {
	printf("Call to fill_polygon_buffer()\n");
	
	/* Няма какво да се запълва :) */
	if (P_SIZE == 0)
		return;
		
	/* В следващите два цикъла обхождаме всички елементи на vertex_buffer, тоест всички върхове */
	for (size_t i=0; i < P_SIZE; i++)
		for (size_t j=0; j < no_segments; j++)
			/* Изчисляваме нормалния вектор на полигона */
			calculate_normal(&GET_LL_V(i, j)->coord, &GET_UL_V(i, j)->coord, &GET_LR_V(i, j)->coord, &(polygon_buffer[i][j].normal));
}

#if SORT_CONTROL_POINTS == 1

void swap_indexes(size_t i, size_t j) {
	printf("Swapping %lu and %lu\n", i, j);
	
	size_t len = no_segments * sizeof(struct vertex);
	struct vertex *tmp;
	
	if ((tmp = (struct vertex*) malloc(len)) == NULL)
		exit(-1);
		
	memcpy(tmp, vertex_buffer[i], len);
	memcpy(vertex_buffer[i], vertex_buffer[j], len);
	memcpy(vertex_buffer[j], tmp, len);
	free(tmp);
}

void sort_control_points() {
	char swapped = 0;
	do {
		for (size_t i=1; i < buffer_size; i++) 
			if (vertex_buffer[i-1][0].coord.y > vertex_buffer[i][0].coord.y)
				swap_indexes(i-1, i);
		swapped = 0;
	} while(swapped);
}

#endif
