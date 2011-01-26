/* gcc -pipe -g -Wall -std=c99 -D_GNU_SOURCE  -lm -lGLU -lglut -o project main.c */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glut.h>
#include <GL/glu.h>

#define SMOOTH_SHADING 1

/* no_segments определя колко страни трябва да имат многоъгълниците, с които апроксимираме окръжностите около оста */
size_t no_segments = 72;

/* g_x и g_y регулират размера на прозореца */
int g_x = 800;
int g_y = 600;

/* VIEWPORT_FACTOR се използва за определяне на това в каква пропорция ще разделяме екрана на две части */
#define VIEWPORT_FACTOR 1.5

/* Големина на числовия интервал, в който да се проектират точките, избрани с мишката */
#define PROJECT_INTERVAL_X 10.f
#define PROJECT_INTERVAL_Y 10.f

struct point {
	float x,y,z;
};

struct vertex {
	struct point *coord;
	struct point *normal;
};

/* Глобалният буфер, в който съхраняваме всички vertex-и */
size_t buffer_size;
struct vertex **vertex_buffer;

/* Градусите за ротиране на "слънцето" */
int alpha_degrees = 0;

void buffer_init();
void buffer_resize(size_t);
void buffer_kill();
void add_point(float, float, float);
void calculateNormal(struct point*, struct point*, struct point*, struct point*);
void vertexNormal(size_t, size_t);
void calculateVertexNormals();
void drawpolygons();
void mouse(int, int, int, int);
void reshape(int, int);
void render();

/* Инициализация на vertex буфера */
void buffer_init() {
	buffer_size = 0;
	vertex_buffer = NULL;
}

/* Преоразмеряване на vertex буфера */
void buffer_resize(size_t new_size) {
	printf("buffer_resize() - old size: %lu new size: %lu\n", buffer_size, new_size);
	if (new_size == 0) {
		buffer_kill();
		return;
	}
	
	if (new_size < buffer_size) {
		for (size_t i = buffer_size; i > new_size; i--) {
			for (int j=0; j < no_segments; j++) {
				free(vertex_buffer[i-1][j].coord);
				free(vertex_buffer[i-1][j].normal);
			}
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
				
			for (int j=0; j < no_segments; j++) {
				if ((vertex_buffer[i][j].coord = (struct point*) malloc(sizeof(struct point))) == NULL)
					exit (-1);
				if ((vertex_buffer[i][j].normal = (struct point*) malloc(sizeof(struct point))) == NULL)
					exit (-1);
				vertex_buffer[i][j].normal->x = vertex_buffer[i][j].normal->y = vertex_buffer[i][j].normal->z = 0;
			}
		}
		
		buffer_size = new_size;
	}
}

/* Освобождаване на паметта */
void buffer_kill() {
	for (int i=0; i < buffer_size; i++) {
		for (int j=0; j < no_segments; j++) {
			free(vertex_buffer[i][j].coord);
			free(vertex_buffer[i][j].normal);
		}
		free(vertex_buffer[i]);
	}
	free(vertex_buffer);
	vertex_buffer = NULL;
	buffer_size = 0;
}

/* 
 * Функцията add_point взима за аргумент контролна точка от образуващата
 * След това тя изчислява и записва точки, които чрез правилен многоъгълник апроксимират окръжност около оста на ротация
 * Оста има координати (0, y, 0)
 */
void add_point(float x, float y, float z) {
	size_t cur_index = buffer_size;
	buffer_resize(buffer_size + 1);
	
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
		vertex_buffer[cur_index][i].coord->x = r * cosf(angle);
		vertex_buffer[cur_index][i].coord->y = y;
		vertex_buffer[cur_index][i].coord->z = r * sinf(angle);
		
		//printf("POINT: %f %f %f DISTANCE FROM 0: %f\n", vertex_buffer[cur_index][i].x, vertex_buffer[cur_index][i].y, vertex_buffer[cur_index][i].z, sqrtf(vertex_buffer[cur_index][i].x*vertex_buffer[cur_index][i].x + vertex_buffer[cur_index][i].y*vertex_buffer[cur_index][i].y + vertex_buffer[cur_index][i].z*vertex_buffer[cur_index][i].z));
	}
	
	calculateVertexNormals();
}

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

/*
 * Изчислява нормален вектор на даден връх като средно аритметично на нормалните вектори на полигоните, в които участва.
 * Съседството на върховете е дадено по схемата:
 * i+1 j-1 | i+1 j | i+1 j+1
 * i   j-1 | i   j | i   j+1
 * i-1 j-1 | i-1 j | i-1 j+1
 */
void vertexNormal(size_t i, size_t j) {
	struct point normal1, normal2, normal3, normal4;
	
	if (i >= buffer_size-1 || j >= buffer_size-1)
		return;
		
	if (i == 0 || j == 0) {
		calculateNormal(vertex_buffer[i][j].coord, vertex_buffer[i+1][j].coord, vertex_buffer[i][j+1].coord, vertex_buffer[i][j].normal);
		return;
	}
		
	calculateNormal(vertex_buffer[i][j].coord, vertex_buffer[i][j-1].coord, vertex_buffer[i+1][j].coord, &normal1);
	calculateNormal(vertex_buffer[i][j].coord, vertex_buffer[i+1][j].coord, vertex_buffer[i][j+1].coord, &normal2);
	calculateNormal(vertex_buffer[i][j].coord, vertex_buffer[i][j+1].coord, vertex_buffer[i-1][j].coord, &normal3);
	calculateNormal(vertex_buffer[i][j].coord, vertex_buffer[i][j-1].coord, vertex_buffer[i-1][j].coord, &normal4);
	
	vertex_buffer[i][j].normal->x = (normal1.x + normal2.x + normal3.x + normal4.x) * .25f;
	vertex_buffer[i][j].normal->y = (normal1.y + normal2.y + normal3.y + normal4.y) * .25f;
	vertex_buffer[i][j].normal->z = (normal1.z + normal2.z + normal3.z + normal4.z) * .25f;
}

void calculateVertexNormals() {
	printf("Calculating vertex normals...");
	for (int i=0; i < buffer_size; i++)
		for (int j=0; j < buffer_size; j++)
			vertexNormal(i, j);
}

/* ----------========== Кодът за рендване ==========---------- */

/* lights() наглася източника на осветление */
void lights() {
	static float near_white[] = {.45f, .45f, .5f, 1.f};
	static float orange[] = {.4f, .3f, .1f, 1.f};

	static float black[] = {.0f, .0f, .0f, 1.f};
	static float white[] = {1.f, 1.f, 1.f, 1.f};
	static float yellow[] = {.99f, .8f, .0f, 1.f};

	static float pos[] = {0.f, 0.f, 0.f, 1.f};

	glEnable(GL_LIGHTING);
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, black ); 

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, near_white);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, orange);
	glLightfv(GL_LIGHT0, GL_SPECULAR, black);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glRotatef(28, 0, 0, 1);
	glRotatef(alpha_degrees,0,1,0);
	glTranslatef(-25,0,0);
	
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	glMaterialfv(GL_FRONT, GL_EMISSION, yellow);
	glutSolidSphere(3,20,20);
	glMaterialfv(GL_FRONT, GL_EMISSION, black);

	glPopMatrix();
}

/* drawpolygons() създава всички полигони, по които ще бъде нарисувано ротационното тяло */
void drawpolygons() {
	if (buffer_size < 1) /* Is the buffer empty? */
		return;
		
	#ifndef SMOOTH_SHADING
		struct point normal;
	#endif
	
	float model_color[] = {1, .2, .2};
	
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	
	#ifdef SMOOTH_SHADING
		glShadeModel(GL_SMOOTH);
	#else
		glShadeModel(GL_FLAT);
	#endif
	
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,0);
			
	for (int i=0; i < buffer_size-1; i++)
		for (int j=0; j < no_segments-1; j++) {
			/*
			 * Добавяме нов полигон (всъщност трапец) чрез следните 4 точки:
			 * vertex_buffer[i][j];
			 * vertex_buffer[i][j+1];
			 * vertex_buffer[i+1][j];
			 * vertex_buffer[i+1][j+1];
			 * Забележка: Функциите glBegin(), glColor3f() и glEnd() могат да бъдат извадени извън цикъла при използване на GL_QUADS
			 */
			glBegin(GL_TRIANGLE_STRIP);
				glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, model_color);
				
				/* Сега вече добавяме върховете на трапеца */
				#ifdef SMOOTH_SHADING
					/* В този случай изчисляваме нормален вектор за всеки връх и викаме glNormal3f преди всеки glVertex3f */
					glNormal3f(vertex_buffer[i][j].normal->x, vertex_buffer[i][j].normal->y, vertex_buffer[i][j].normal->z);
					glVertex3f(vertex_buffer[i][j].coord->x, vertex_buffer[i][j].coord->y, vertex_buffer[i][j].coord->z);
					
					glNormal3f(vertex_buffer[i+1][j].normal->x, vertex_buffer[i+1][j].normal->y, vertex_buffer[i+1][j].normal->z);
					glVertex3f(vertex_buffer[i+1][j].coord->x, vertex_buffer[i+1][j].coord->y, vertex_buffer[i+1][j].coord->z);
					
					glNormal3f(vertex_buffer[i][j+1].normal->x, vertex_buffer[i][j+1].normal->y, vertex_buffer[i][j+1].normal->z);
					glVertex3f(vertex_buffer[i][j+1].coord->x, vertex_buffer[i][j+1].coord->y, vertex_buffer[i][j+1].coord->z);
					
					glNormal3f(vertex_buffer[i+1][j+1].normal->x, vertex_buffer[i+1][j+1].normal->y, vertex_buffer[i+1][j+1].normal->z);
					glVertex3f(vertex_buffer[i+1][j+1].coord->x, vertex_buffer[i+1][j+1].coord->y, vertex_buffer[i+1][j+1].coord->z);
				#else
					/* Изчисляваме нормален вектор само за целия трапец */
					calculateNormal(vertex_buffer[i][j].coord, vertex_buffer[i+1][j].coord, vertex_buffer[i][j+1].coord, &normal);
					glNormal3f(normal.x, normal.y, normal.z);
					
					glVertex3f(vertex_buffer[i][j].coord->x, vertex_buffer[i][j].coord->y, vertex_buffer[i][j].coord->z);
					glVertex3f(vertex_buffer[i+1][j].coord->x, vertex_buffer[i+1][j].coord->y, vertex_buffer[i+1][j].coord->z);
					glVertex3f(vertex_buffer[i][j+1].coord->x, vertex_buffer[i][j+1].coord->y, vertex_buffer[i][j+1].coord->z);
					glVertex3f(vertex_buffer[i+1][j+1].coord->x, vertex_buffer[i+1][j+1].coord->y, vertex_buffer[i+1][j+1].coord->z);
				#endif
			glEnd();
		}
}

/* drawmodel() рисува ротационното тяло */
void drawmodel() {
	drawpolygons();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glRotatef(90,0,1,0);
		drawpolygons();
		glRotatef(90,0,1,0);
		drawpolygons();
		glRotatef(90,0,1,0);
		drawpolygons();
	glPopMatrix();
}

/* Callback за действията с мишката */
void mouse (int button, int state, int mx, int my) {
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (mx <= g_x/VIEWPORT_FACTOR) { /* Точката е във viewport-а на ротационното тяло */
			printf("Control point position is in viewport; ignoring!\n");
			return;
		}
		
		/* Целта на по-долния код е да проектира координатите (mx,my) в x и y, които са в интервали, избрани с PROJECT_INTERVAL_* */
		float x = (float)((mx-(g_x/VIEWPORT_FACTOR))*(PROJECT_INTERVAL_X/(g_x-g_x/VIEWPORT_FACTOR)));
		float y = PROJECT_INTERVAL_Y - (float)(my*(PROJECT_INTERVAL_Y/g_y));
		
		printf("Adding point %f %f %f (converted from %d %d %d)\n", x, y, 0.f, mx, my, 0);
		add_point(x, y, 0);
		
		glutPostRedisplay();
	}
}

/* Callback за смяна на размера на прозореца */
void reshape(int x, int y) {
	g_x = x; g_y = y;
	printf("Changed g_x=%d g_y=%d\n", g_x, g_y);
	glViewport(0, 0, g_x/VIEWPORT_FACTOR, g_y);
	glutPostRedisplay();
}

void timer(int foo) {
	alpha_degrees += 3;
	glutTimerFunc(30,timer,0);
	glutPostRedisplay();
}

/* Рендване на сцената */
void render() {
	printf("Call to render() with %lu points\n", buffer_size);
	glViewport(0, 0, g_x/VIEWPORT_FACTOR, g_y);
	
	glClearColor(0.f,0.f,0.f,0.f);
	glClearDepth(10);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if (buffer_size < 1)
		return;
	
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60,1,2,200);
	gluLookAt(1,1,+40,0,7,0,0,1,0);
	
	lights();
	drawmodel();
	
	glFlush();
	glutSwapBuffers();
}

int main(int argc, char **argv) {
	buffer_init();
	
	/* Като първи аргумент може да се зададе нова стойност на детайлността */
	if (argc > 1)
		no_segments = (unsigned) atoi(argv[1]);
	
	add_point(0.f, 0.f, 0.f);
	add_point(1.5f, 2.f, 0.f);
	add_point(2.f, 1.f, 0.f);
	add_point(3.f, 5.f, 0.f);
	add_point(4.f, 5.f, 0.f);
	add_point(7.f, 8.f, 0.f);
	add_point(9.f, 9.f, 0.f);
			
	glutInit(&argc, argv);
	
	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(g_x, g_y);
	
	glutCreateWindow("OKG");
	
	glutDisplayFunc(render);
	glutMouseFunc(mouse);
	glutTimerFunc(20, timer, 0);
	
	glutMainLoop();
	
	buffer_kill();
	
	return 0;
}
