/* gcc -pipe -g -Wall -std=c99 -D_GNU_SOURCE  -lm -lGLU -lglut -o project main.c */
#include <math.h>
#include <stdio.h>

#include <GL/glut.h>
#include <GL/glu.h>

#include "buffer.h"
#include "globals.h"
#include "normal.h"

#define SMOOTH_SHADING 1

/* g_x и g_y регулират размера на прозореца */
int g_x = 800;
int g_y = 600;

/* VIEWPORT_FACTOR се използва за определяне на това в каква пропорция ще разделяме екрана на две части */
#define VIEWPORT_FACTOR 1.5

/* Големина на числовия интервал, в който да се проектират точките, избрани с мишката */
#define PROJECT_INTERVAL_X 10.f
#define PROJECT_INTERVAL_Y 10.f

/* Целта на по-долните макроси е да проектира координатите (x,y) в интервали, избрани с PROJECT_INTERVAL_* */
#define PROJECT_IN_X(x) ((float)(((x)-(g_x/VIEWPORT_FACTOR))*(PROJECT_INTERVAL_X/(g_x-g_x/VIEWPORT_FACTOR))))
#define PROJECT_IN_Y(y) (PROJECT_INTERVAL_Y - (float)((y)*(PROJECT_INTERVAL_Y/g_y)))

/* Градусите за ротиране на "слънцето" */
int alpha_degrees = 0;

void drawpolygons();
void mouse(int, int, int, int);
void reshape(int, int);
void render();

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
		
		float x = PROJECT_IN_X(mx);
		float y = PROJECT_IN_Y(my);
		
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
