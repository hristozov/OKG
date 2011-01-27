#include "buffer.h"
#include "normal.h"

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
	
	/* Ако новият размер е нула, просто унищожаваме буфера.
	 * Буферът с полигоните също ще бъде премахнат, но той така или иначе е зависим от вертексния */
	if (new_size == 0) {
		buffer_kill();
		return;
	}
	
	/* Смаляваме буфера */
	if (new_size < buffer_size) {
		/* Изчистваме вече ненужните елементи */
		for (size_t i = buffer_size; i > new_size; i--)
			free(vertex_buffer[i-1]);
			
		/* Накрая реалокираме паметта */
		if ((vertex_buffer = (struct vertex**) realloc(vertex_buffer, new_size * sizeof(struct vertex*))) == NULL)
			exit(-1);
	}
	
	/* Увеличаваме буфера */
	if (new_size > buffer_size) {
		/* Реалокираме буфера с новия размер */
		if ((vertex_buffer = (struct vertex**) realloc(vertex_buffer, new_size * sizeof(struct vertex*))) == NULL)
			exit(-1);
		
		/* Добавяме всеки от елементите (масиви от vertex структури) на буфера */
		for (size_t i = buffer_size; i < new_size; i++) {
			/* Заделяме памет за масив от vertex-и */
			if ((vertex_buffer[i] = (struct vertex*) calloc(no_segments, sizeof(struct vertex))) == NULL)
				exit(-1);
				
			/* Нулираме полетата с указатели във всеки vertex */
			for (size_t j=0; j < no_segments; j++)
				vertex_buffer[i][j].p[0] = vertex_buffer[i][j].p[1] = vertex_buffer[i][j].p[2] = vertex_buffer[i][j].p[3] = NULL;
		}
	}
	
	/* Записваме новия размер */
	buffer_size = new_size;
}

/* Освобождаване на паметта */
void buffer_kill() {
	for (size_t i=0; i < buffer_size; i++) {
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
	/* Проверяваме дали точката вече не е добавена и излизаме, ако това е вярно */
	for (size_t i=0; i < buffer_size; i++)
		if (vertex_buffer[i][0].coord.x == x && vertex_buffer[i][0].coord.y == y && vertex_buffer[i][0].coord.z == z)
			return;
			
	/* Изчисляваме текущия индекс в буфера и преоразмеряваме */
	size_t cur_index = buffer_size;
	vertex_buffer_resize(buffer_size + 1);
	
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
	
	/* Винаги първо запълваме буфера с полигоните.
	 * Там се намират нормалните им вектори, които по-късно ще послужат при евентуално ползване на calculate_vertex_normals() */
	fill_polygon_buffer();
	#ifdef SMOOTH_SHADING
		calculate_vertex_normals(); /* Изчисляваме нормалните вектори на всеки връх */
	#endif
}

/* Добавя указател към полигон в списъка с указатели на дадения връх */
void add_polygon_ptr_to_vertex(struct polygon *poly, struct vertex *vert) {
	if (poly == NULL || vert == NULL)
		return;
	for (size_t i=0; i<4; i++) {
		if (vert->p[i] != NULL) { /* Вече има записан полигон на позиция i в списъка */
			if (vert->p[i] == poly) /* ...и ако този полигон е настоящият (poly), излизаме */
				return;
			continue; /* Продължаваме към следващия елемент */
		}
		
		/* Тук стигаме само ако p[i]==NULL, тоест имаме празна позиция.
		 * Записваме настоящия полигон на тази позиция излизаме от функцията */
		vert->p[i] = poly;
		return;
	}
}

/* fill_polygon_buffer() запълва буфера за полигони с върховете им и нормалните им вектори */
void fill_polygon_buffer() {
	printf("Call to fill_polygon_buffer()\n");
	size_t index;
	struct polygon *cur;
	
	/* Изчистваме буфера, ако вече е запълнен */
	if (polygon_buffer != NULL)
		free (polygon_buffer);
		
	/* Полигоните са колкото броя на контролните точки минус 1, умножен по броя на сегментите */
	polygon_size = (buffer_size-1) * (no_segments);
	
	if (polygon_size == 0) /* Нямаме достатъчно точки за построяване на полигона, ще се занимаваме с това по-късно */
		return;
		
	polygon_buffer = malloc (polygon_size * sizeof(struct polygon));
	
	/* В следващите два цикъла обхождаме всички елементи на vertex_buffer, тоест всички върхове */
	for (size_t i=0; i < buffer_size-1; i++)
		for (size_t j=0; j < no_segments; j++) {
			/* Индексът на всеки полигон се определя от индекса на долния му ляв връх */
			index = i*(no_segments) + j;
			cur = &polygon_buffer[index];
			
			/* Първо добавяме долния ляв връх. Последователността е такава заради употребата на GL_TRIANGLE_STRIP */
			cur->v[0] = &vertex_buffer[i][j];
			add_polygon_ptr_to_vertex(cur, &vertex_buffer[i][j]);
			
			/* Горен ляв */
			cur->v[1] = &vertex_buffer[i+1][j];
			add_polygon_ptr_to_vertex(cur, &vertex_buffer[i+1][j]);
			
			/* Долен десен */
			cur->v[2] = &vertex_buffer[i][(j+1)%(no_segments)];
			add_polygon_ptr_to_vertex(cur, &vertex_buffer[i][(j+1)%(no_segments)]);
			
			/* Горен десен */
			cur->v[3] = &vertex_buffer[i+1][(j+1)%(no_segments)];
			add_polygon_ptr_to_vertex(cur, &vertex_buffer[i+1][(j+1)%(no_segments)]);
			
			/* Изчисляваме нормалния вектор на полигона */
			calculate_normal(&cur->v[0]->coord, &cur->v[1]->coord, &cur->v[2]->coord, &cur->normal);
		}
}
