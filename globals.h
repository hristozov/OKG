#pragma once
#include <stdlib.h> /* size_t */

/* g_x и g_y регулират размера на прозореца */
extern int g_x;
extern int g_y;

/* VIEWPORT_FACTOR се използва за определяне на това в каква пропорция ще разделяме екрана на две части */
#define VIEWPORT_FACTOR 1.5

/* VIEWPORT_BORDER дефинира къде е границата между двете части на екрана */
#define VIEWPORT_BORDER g_x/VIEWPORT_FACTOR

/* Макроси за проверка в коя част на екрана се намира дадена стойност x */
#define IS_IN_LEFT_VIEWPORT(x) ((x) < VIEWPORT_BORDER ? 1 : 0)
#define IS_IN_RIGHT_VIEWPORT(x) ((x) < VIEWPORT_BORDER ? 0 : 1)

/* Големина на числовия интервал, в който да се проектират точките, избрани с мишката */
#define PROJECT_INTERVAL_X 10.f
#define PROJECT_INTERVAL_Y 10.f

/* Целта на по-долните макроси е да проектира координатите (x,y) в интервали, избрани с PROJECT_INTERVAL_* */
#define PROJECT_IN_X(x) ((float)(((x)-(g_x/VIEWPORT_FACTOR))*(PROJECT_INTERVAL_X/(g_x-g_x/VIEWPORT_FACTOR))))
#define PROJECT_IN_Y(y) (PROJECT_INTERVAL_Y - (float)((y)*(PROJECT_INTERVAL_Y/g_y)))

/* no_segments определя колко страни трябва да имат многоъгълниците, с които апроксимираме окръжностите около оста */
extern size_t no_segments;

struct point {
	float x,y,z;
};

struct vertex {
	struct point coord;
	struct point normal;
};

struct polygon {
	struct vertex *p[4];
	struct point normal;
};

/* Глобалният буфер, в който съхраняваме всички vertex-и */
extern size_t buffer_size;
extern struct vertex **vertex_buffer;

extern size_t polygon_size;
extern struct polygon *polygon_buffer;
