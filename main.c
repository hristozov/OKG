/*
 * В main.c се съхранява основният код за боравяне с GLUT
 * Тук се намират и callback функциите
 */

#include <stdio.h>

#include <GL/glut.h>
#include <GL/glu.h>

#include "buffer.h"
#include "globals.h"
#include "normal.h"

/* Градусите за ротиране на "слънцето" */
static int alpha_degrees_0;
static int alpha_degrees_1;

/* На колко градуса да се завърти относно x и y */
static int diff_x, diff_y;

/* Ниво на zoom-ване */
static int zoom_level = 40;

/* Флагови променливи, определящи дали са спрени въртенето и източниците на светлина */
static char rotation_paused, light0_disabled, light1_disabled;

void lights();
void drawpolygons();
void mouse(int, int, int, int);
void keyboard_special(int, int, int);
void keyboard(unsigned char, int, int);
void reshape(int, int);
void rotateSun(int);
void render();

/* light0() наглася първия източник на осветление */
void light0() {
	if (light0_disabled != 0) { /* Ако флагът е вдигнат, спираме източника */
		glDisable(GL_LIGHT0);
		return;
	}
	
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
		glRotatef(alpha_degrees_0,0,1,0);
		glTranslatef(-25,0,0);
	
		glLightfv(GL_LIGHT0, GL_POSITION, pos);

		glMaterialfv(GL_FRONT, GL_EMISSION, yellow);
		glutSolidSphere(3,20,20);
		glMaterialfv(GL_FRONT, GL_EMISSION, black);

	glPopMatrix();
}

/* light1() наглася втория източник на осветление */
void light1() {
	if (light1_disabled != 0) { /* Ако флагът е вдигнат, спираме източника */
		glDisable(GL_LIGHT1);
		return;
	}
	
	static float near_white[] = {.45f, .45f, .5f, 1.f};
	static float orange[] = {.4f, .3f, .1f, 1.f};

	static float black[] = {.0f, .0f, .0f, 1.f};
	/* static float white[] = {1.f, 1.f, 1.f, 1.f}; */
	static float yellow[] = {.99f, .8f, .0f, 1.f};

	static float pos[] = {0.f, 0.f, 0.f, 1.f};

	glEnable(GL_LIGHTING);
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, black ); 

	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, near_white);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, orange);
	glLightfv(GL_LIGHT1, GL_SPECULAR, black);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

		glRotatef(28, 0, 0, 1);
		glRotatef(alpha_degrees_1,0,1,0);
		glTranslatef(25,0,0);
	
		glLightfv(GL_LIGHT1, GL_POSITION, pos);

		glMaterialfv(GL_FRONT, GL_EMISSION, yellow);
		glutSolidSphere(3,20,20);
		glMaterialfv(GL_FRONT, GL_EMISSION, black);

	glPopMatrix();
}

/* drawpolygons() използва вече създадените полигони, за да нарисува ротационното тяло */
void drawpolygons() {
	if (buffer_size < 1) /* Празен ли е буферът? */
		return;
		
	struct polygon *cur;
		
	/* Цвят на полигоните, които ще нарисуваме */
	float model_color[] = {1, .2, .2};
	
	glEnable(GL_POLYGON_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	
	#if SMOOTH_SHADING == 1
		glShadeModel(GL_SMOOTH);
	#else
		glShadeModel(GL_FLAT);
	#endif
	
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,0);
			
	for (size_t i=0; i < P_SIZE; i++) 
		for (size_t j=0; j < no_segments; j++) {
			/*
			 * Добавяме нов полигон (всъщност трапец) от буфера
			 */
			cur = &polygon_buffer[i][j];
			struct vertex *ll = GET_LL_V(i, j), *ul = GET_UL_V(i, j), *lr = GET_LR_V(i, j), *ur = GET_UR_V(i, j);
			
			glBegin(GL_TRIANGLE_STRIP);
				glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, model_color);
				
				/* Сега вече добавяме върховете на трапеца. Подредбата е:
				 * 1. Долу вляво
				 * 2. Горе вляво
				 * 3. Долу вдясно
				 * 4. Горе вдясно */
				#if SMOOTH_SHADING == 1
					/* В този случай викаме glNormal3f преди всеки glVertex3f */
					glNormal3f(ll->normal.x, ll->normal.y, ll->normal.z);
					glVertex3f(ll->coord.x, ll->coord.y, ll->coord.z);
					glNormal3f(ul->normal.x, ul->normal.y, ul->normal.z);
					glVertex3f(ul->coord.x, ul->coord.y, ul->coord.z);
					glNormal3f(lr->normal.x, lr->normal.y, lr->normal.z);
					glVertex3f(lr->coord.x, lr->coord.y, lr->coord.z);
					glNormal3f(ur->normal.x, ur->normal.y, ur->normal.z);
					glVertex3f(ur->coord.x, ur->coord.y, ur->coord.z);
				#else
					/* Изчисляваме нормален вектор само за целия трапец */
					glNormal3f(cur->normal.x, cur->normal.y, cur->normal.z);
					
					glVertex3f(ll->coord.x, ll->coord.y, ll->coord.z);
					glVertex3f(ul->coord.x, ul->coord.y, ul->coord.z);
					glVertex3f(lr->coord.x, lr->coord.y, lr->coord.z);
					glVertex3f(ur->coord.x, ur->coord.y, ur->coord.z);
				#endif
			glEnd();
			
			/* Код за debug - показва нормалните вектори на всеки полигон */
			#if SHOW_POLYGON_NORMALS == 1
				float sum_x = ll->coord.x + ul->coord.x + lr->coord.x + ur->coord.x;
				float sum_y = ll->coord.y + ul->coord.y + lr->coord.y + ur->coord.y;
				float sum_z = ll->coord.z + ul->coord.z + lr->coord.z + ur->coord.z; 
				
				float len = VECTOR_LENGTH (polygon_buffer[i][j].normal);
				if (len < 0.99f) /* Загуба на точност */
					printf("WARNING: Polygon normal vector with length %f at index %lu %lu\n", len, i, j);
				
				glBegin (GL_LINES);
					glVertex3f(sum_x*.25f, sum_y*.25f, sum_z*.25f);
					glVertex3f((sum_x*.25f)+cur->normal.x, (sum_y*.25f)+cur->normal.y, (sum_z*.25f)+cur->normal.z);
				glEnd();
			#endif
		}
		
	/* Код за debug - показва нормалните вектори на всеки връх */
	#if SMOOTH_SHADING == 1 && SHOW_VERTEX_NORMALS == 1
		for (size_t i = 0; i < V_SIZE; i++)
			for (size_t j =0; j < no_segments; j++) {
				float len = VECTOR_LENGTH (vertex_buffer[i][j].normal);
				if (len < 0.99f) /* Загуба на точност */
					printf("WARNING: Vertex normal vector with length %f at index %lu %lu\n", len, i, j);
						
				glBegin (GL_LINES);
					glVertex3f(vertex_buffer[i][j].coord.x, vertex_buffer[i][j].coord.y, vertex_buffer[i][j].coord.z);
					glVertex3f(vertex_buffer[i][j].coord.x + vertex_buffer[i][j].normal.x,
					           vertex_buffer[i][j].coord.y + vertex_buffer[i][j].normal.y,
					           vertex_buffer[i][j].coord.z + vertex_buffer[i][j].normal.z);
				glEnd();
			}
	#endif

}

/* drawmodel() рисува ротационното тяло */
void drawmodel() {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glRotatef((float)diff_y, 0, 1.f, 0);
		glRotatef((float)diff_x, 1.f, 0, 0);
		drawpolygons();
	glPopMatrix();
}

/* Callback за действията с мишката */
void mouse (int button, int state, int mx, int my) {
	switch(button) {
		case GLUT_LEFT_BUTTON:
			if (IS_IN_RIGHT_VIEWPORT(mx) && state == GLUT_DOWN) {
				float x = PROJECT_IN_X(mx);
				float y = PROJECT_IN_Y(my);
		
				printf("Adding point %f %f %f (converted from %d %d %d)\n", x, y, 0.f, mx, my, 0);
				add_point(x, y, 0);
			}
			break;
		case 3: /* mouse wheel надолу */
			zoom_level -= 1;
			break;
		case 4: /* mouse wheel нагоре */
			zoom_level += 1;
			break;
	}
	glutPostRedisplay();
}

/* "Специален" callback за клавиатурата. Прихваща по-специални клавиши. Използва се за въртене на тялото. */
void keyboard_special (int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_UP:    diff_x -=15; break;
		case GLUT_KEY_DOWN:  diff_x +=15; break;
		case GLUT_KEY_LEFT:  diff_y -=15; break;
		case GLUT_KEY_RIGHT: diff_y +=15; break;
	}
	glutPostRedisplay();
}

/* Callback за клавиатурата. Прихваща "нормални клавиши"
 * В момента служи само за смяна на флагове при натискане на даден клавиш */
void keyboard (unsigned char key, int x, int y) {
	switch(key) {
		case ' ': rotation_paused = (rotation_paused == 0) ? 1:0; break;
		case '1': light0_disabled = (light0_disabled == 0) ? 1:0; break;
		case '2': light1_disabled = (light1_disabled == 0) ? 1:0; break;
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
void rotateSun0(int foo) {
	if (rotation_paused == 0) /* Изменяме ъгъла само ако въртенето не е спряно */
		alpha_degrees_0 += 3;
	glutTimerFunc(30,rotateSun0,0);
	glutPostRedisplay();
}

/* Таймер за смяна на позицията на слънцето */
void rotateSun1(int foo) {
	if (rotation_paused == 0) /* Изменяме ъгъла само ако въртенето не е спряно */
		alpha_degrees_1 -= 2;
	glutTimerFunc(30,rotateSun1,0);
	glutPostRedisplay();
}

/* Рендване на сцената */
void render() {
	/* printf("Call to render() with %lu points\n", buffer_size); */
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
	gluLookAt(1,1,zoom_level,0,7,0,0,1,0);
			
	light0();
	light1();
			
	drawmodel();
	
	glFlush();
	glutSwapBuffers();
}

int main(int argc, char **argv) {
	buffer_init();
	
	/* Като първи аргумент може да се зададе нова стойност на детайлността */
	if (argc > 1)
		no_segments = (unsigned) atoi(argv[1]);
	
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
	glutSpecialFunc(keyboard_special);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(20, rotateSun0, 0);
	glutTimerFunc(20, rotateSun1, 0);
	
	glutMainLoop();
	
	buffer_kill();
	
	return 0;
}
