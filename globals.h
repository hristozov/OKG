/*
 * Тук са глобалните променливи, макроси и структури
 */

#pragma once
#include <math.h> /* sqrtf() */
#include <stdlib.h> /* size_t */

/* Глобален макрос, определящ какъв shading метод да се ползва */
#define SMOOTH_SHADING 1

/* Да сортираме ли контролните точки? */
#define SORT_CONTROL_POINTS 0

/* Глобален макрос, определящ дали да се показват нормалните вектори на полигоните (за debug) */
#define SHOW_POLYGON_NORMALS 0

/* Глобален макрос, определящ дали да се показват нормалните вектори на vertex-ите (за debug) */
#define SHOW_VERTEX_NORMALS 0

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

/* Дължина на вектор */
#define VECTOR_LENGTH(pt) ((sqrtf ((pt).x*(pt).x + (pt).y*(pt).y + (pt).z*(pt).z)))

/* Големина на числовия интервал, в който да се проектират точките, избрани с мишката */
#define PROJECT_INTERVAL_X 10.f
#define PROJECT_INTERVAL_Y 10.f

/* Целта на по-долните макроси е да проектира координатите (x,y) в интервали, избрани с PROJECT_INTERVAL_* */
#define PROJECT_IN_X(x) ((float)(((x)-(g_x/VIEWPORT_FACTOR))*(PROJECT_INTERVAL_X/(g_x-g_x/VIEWPORT_FACTOR))))
#define PROJECT_IN_Y(y) (PROJECT_INTERVAL_Y - (float)((y)*(PROJECT_INTERVAL_Y/g_y)))

/* Тези макроси взимат за аргумент индексите на даден връх и връщат полигоните от неговите различни страни
 * Индексът на всеки полигон [i][j] означава, че [i][j] е върхът в долния му ляв ъгъл.
 * Двубуквеното съкращение в средата означава:
 * Първа буква: U=upper ; L=lower
 * Втора буква: L=left ; R=right */
 
/* Помощен макрос за "нормализиране" на j - индекс на връх или на полигон.
 * Понеже последния и първия полигон са съседни, този макрос служи за връщане на коректна стойност при избиране на съседите им */
#define NORMALIZE_J(j) (((j) + no_segments) % no_segments)

#define CHECK_I_P(i) (((i) >= 0) && ((i) < P_SIZE)) /* Проверка за валиден първи индекс */
#define GET_P(i, j) ((CHECK_I_P((i))) ? &polygon_buffer[(i)][NORMALIZE_J((j))] : NULL) /* Съшинско обръщение към буфера */
 
#define GET_LL_P(i, j) (GET_P(i-1, j-1))
#define GET_UL_P(i, j) (GET_P((i), (j)-1))
#define GET_UR_P(i, j) (GET_P((i), (j)))
#define GET_LR_P(i, j) (GET_P((i)-1, (j)))

/* Тези макроси взимат за аргумент индексите на полигон и връщат върховете, от които е съставен
 * Отново взимаме предвид, че полигонът [i][j] има връх [i][j] долу вляво
 * Съкращенията са аналогични */

#define CHECK_I_V(i) (((i) >= 0) && ((i) < V_SIZE)) /* Проверка за валиден първи индекс */
#define GET_V(i, j) ((CHECK_I_V((i))) ? &vertex_buffer[(i)][NORMALIZE_J((j))] : NULL) /* Съшинско обръщение към буфера */

#define GET_LL_V(i, j) (GET_V((i), (j)))
#define GET_UL_V(i, j) (GET_V((i)+1, (j)))
#define GET_UR_V(i, j) (GET_V((i)+1, (j)+1))
#define GET_LR_V(i, j) (GET_V((i), (j)+1))

/* no_segments определя колко страни трябва да имат многоъгълниците, с които апроксимираме окръжностите около оста */
extern size_t no_segments;

/* Точка в пространството */
struct point {
	float x,y,z;
};

/* Структура за съхраняване на отделните върхове в ротационното тяло */
struct vertex {
	struct point coord; /* Координати на текущия връх */
	struct point normal; /* Нормален вектор на върха (зададен като точка) */
};

/* Структура за съхраняване на отделните полигони (трапци), които ще бъдат рисувани */
struct polygon {
	struct point normal; /* Нормален вектор на полигона (зададен като точка) */
};

/* Глобалният буфер, в който съхраняваме всички vertex-и */
extern size_t buffer_size;
extern struct vertex **vertex_buffer;

extern struct polygon **polygon_buffer;

/* По-удобни начини за взимане на размери: */
#define V_SIZE buffer_size
#define P_SIZE (buffer_size > 0 ? (buffer_size-1) : 0) /* За да избегнем integer overflow връщаме нула при buffer_size==0 */
