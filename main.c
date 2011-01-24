/* gcc -pipe -g -Wall -std=c99 -D_GNU_SOURCE  -lm -lGLU -lglut -o project main.c */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glut.h>
#include <GL/glu.h>

/* NO_SEGMENTS определя колко страни трябва да имат многоъгълниците, с които апроксимираме окръжностите около оста */
#define NO_SEGMENTS 360

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

/* Глобалният буфер, в който съхраняваме всички vertex-и */
size_t buffer_size;
struct point **vertex_buffer;

void buffer_init();
void buffer_resize(size_t);
void buffer_kill();
void add_point(float, float, float);
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
		for (size_t i = buffer_size; i > new_size; i--)
			free (vertex_buffer[i-1]);
			
		if ((vertex_buffer = (struct point**) realloc(vertex_buffer, new_size * sizeof(struct point*))) == NULL)
			exit(-1);
			
		buffer_size = new_size;
	}
	
	if (new_size > buffer_size) {
		if ((vertex_buffer = (struct point**) realloc(vertex_buffer, new_size * sizeof(struct point*))) == NULL)
			exit(-1);
		
		for (size_t i = buffer_size; i < new_size; i++)
			if ((vertex_buffer[i] = (struct point*) calloc(NO_SEGMENTS, sizeof(struct point))) == NULL)
				exit(-1);
		
		buffer_size = new_size;
	}
}

/* Освобождаване на паметта */
void buffer_kill() {
	for (int i=0; i < buffer_size; i++)
		free(vertex_buffer[i]);
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
	
	for (size_t i = 0; i < NO_SEGMENTS; i++) {
		/* Ъгълът на текущия сегмент */
		float angle = (2.0f * M_PI * (float)i) / (float)NO_SEGMENTS;
		
		/* 
		 * Изчисляваме координатите на текущата точка
		 * Променяме само x и z, защото апроксимираме фигура около оста с координати (0, y, 0)
		 */
		vertex_buffer[cur_index][i].x = r * cosf(angle);
		vertex_buffer[cur_index][i].y = y;
		vertex_buffer[cur_index][i].z = r * sinf(angle);
		
		//printf("POINT: %f %f %f DISTANCE FROM 0: %f\n", vertex_buffer[cur_index][i].x, vertex_buffer[cur_index][i].y, vertex_buffer[cur_index][i].z, sqrtf(vertex_buffer[cur_index][i].x*vertex_buffer[cur_index][i].x + vertex_buffer[cur_index][i].y*vertex_buffer[cur_index][i].y + vertex_buffer[cur_index][i].z*vertex_buffer[cur_index][i].z));
	}
}

/* ----------========== Кодът за рендване ==========---------- */

/* drawpolygons() създава всички полигони, по които ще бъде нарисувано ротационното тяло */
void drawpolygons() {
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	
	if (buffer_size < 1) /* Is the buffer empty? */
		return;
		
	for (int i=0; i < buffer_size-1; i++)
		for (int j=0; j < NO_SEGMENTS-1; j++) {
			/*
			 * Добавяме нов полигон (всъщност правоъгълник) чрез следните 4 точки:
			 * vertex_buffer[i][j];
			 * vertex_buffer[i][j+1];
			 * vertex_buffer[i+1][j];
			 * vertex_buffer[i+1][j+1];
			 * Забележка: Функциите glBegin(), glColor3f() и glEnd() могат да бъдат извадени извън цикъла при използване на GL_QUADS
			 */
			glBegin(GL_TRIANGLE_STRIP);
				glColor3f(1,.2,.2);
				glVertex3f(vertex_buffer[i][j].x,vertex_buffer[i][j].y,vertex_buffer[i][j].z);
				glVertex3f(vertex_buffer[i+1][j].x,vertex_buffer[i+1][j].y,vertex_buffer[i+1][j].z);
				glVertex3f(vertex_buffer[i][j+1].x,vertex_buffer[i][j+1].y,vertex_buffer[i][j+1].z);
				glVertex3f(vertex_buffer[i+1][j+1].x,vertex_buffer[i+1][j+1].y,vertex_buffer[i+1][j+1].z);
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
	glViewport(0, 0, g_x/VIEWPORT_FACTOR, g_y);
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
	gluLookAt(1,1,+20,0,7,0,0,1,0);
	
	drawmodel();
	
	glFlush();
}

int main(int argc, char **argv) {
	buffer_init();
	
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
	
	glutMainLoop();
	
	buffer_kill();
	
	return 0;
}
