/* gcc -pipe -g -Wall -std=c99 -D_GNU_SOURCE  -lm -lGLU -lglut -o project main.c */
#include <math.h>
#include <stdio.h>

#include <GL/glut.h>
#include <GL/glu.h>

#include "buffer.h"
#include "globals.h"
#include "normal.h"

#define SMOOTH_SHADING 1

/* Градусите за ротиране на "слънцето" */
int alpha_degrees = 0;

void lights();
void drawpolygons();
void mouse(int, int, int, int);
void reshape(int, int);
void rotateSun(int);
void render();

/* lights() наглася източника на осветление */
void lights() {
	static float near_white[] = {.45f, .45f, .5f, 1.f};
	static float orange[] = {.4f, .3f, .1f, 1.f};

	static float black[] = {.0f, .0f, .0f, 1.f};
	/* static float white[] = {1.f, 1.f, 1.f, 1.f}; */
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
	if (buffer_size < 1) /* Празен ли е буферът? */
		return;
		
	struct polygon *cur;
		
	/* Цвят на полигоните, които ще нарисуваме */
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
			
	for (int i=0; i < polygon_size; i++) {
			/*
			 * Добавяме нов полигон (всъщност трапец)
			 */
			cur = &polygon_buffer[i];
			
			glBegin(GL_TRIANGLE_STRIP);
				glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, model_color);
				
				/* Сега вече добавяме върховете на трапеца */
				#ifdef SMOOTH_SHADING
					/* В този случай викаме glNormal3f преди всеки glVertex3f */
					for (int j=0; j < 4; j++) {
						glNormal3f(cur->p[j]->normal.x, cur->p[j]->normal.y, cur->p[j]->normal.z);
						glVertex3f(cur->p[j]->coord.x, cur->p[j]->coord.y, cur->p[j]->coord.z);
					}
				#else
					/* Изчисляваме нормален вектор само за целия трапец */
					glNormal3f(cur->normal.x, cur->normal.y, cur->normal.z);
					
					for (int j=0; j < 4; j++)
						glVertex3f(cur->p[j]->coord.x, cur->p[j]->coord.y, cur->p[j]->coord.z);
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
		if (IS_IN_LEFT_VIEWPORT(mx)) { /* Точката е във viewport-а на ротационното тяло */
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
	glViewport(0, 0, VIEWPORT_BORDER, g_y);
	glutPostRedisplay();
}

/* Таймер за смяна на позицията на слънцето */
void rotateSun(int foo) {
	alpha_degrees += 3;
	glutTimerFunc(30,rotateSun,0);
	glutPostRedisplay();
}

/* Рендване на сцената */
void render() {
	printf("Call to render() with %lu points\n", buffer_size);
	glViewport(0, 0, VIEWPORT_BORDER, g_y);
	
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
	glutTimerFunc(20, rotateSun, 0);
	
	glutMainLoop();
	
	buffer_kill();
	
	return 0;
}
