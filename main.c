#include <GL/freeglut.h>
#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>

int windowWidth = 800;
int windowHeight = 600;

void init() {
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

  glMatrixMode(GL_PROJECTION);
  gluOrtho2D(0.0f, windowWidth, 0.0f, windowHeight);

  // Para o antialiasing
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LINE_SMOOTH);
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);

  glColor3f(1.0f, 0.0f, 0.0f);

  glBegin(GL_POLYGON);
  glVertex2f((float)windowWidth / 2, 2 * (float)windowHeight / 3);
  glVertex2f((float)windowWidth / 3, (float)windowHeight / 3);
  glVertex2f(2 * (float)windowWidth / 3, (float)windowHeight / 3);
  glEnd();

  glFlush();
}

void displayStickman() {
  // Ativar antialiasing para linhas
  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClear(GL_COLOR_BUFFER_BIT);

  glColor3f(1.0, 0.0, 0.0);

  glBegin(GL_LINES);
  glVertex2i(100, 100);
  glVertex2i(100, 40);

  glVertex2i(100, 40);
  glVertex2i(120, 10);

  glVertex2i(100, 40);
  glVertex2i(80, 10);

  glVertex2i(100, 90);
  glVertex2i(120, 60);

  glVertex2i(100, 90);
  glVertex2i(80, 60);
  glEnd();

  glBegin(GL_POLYGON);
  glVertex2i(90, 120);
  glVertex2i(110, 120);
  glVertex2i(110, 100);
  glVertex2i(90, 100);
  glEnd();

  glFlush();

  // Desabilitar antialiasing depois que terminar de renderizar
  glDisable(GL_BLEND);
  glDisable(GL_LINE_SMOOTH);
}

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_MULTISAMPLE);
  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Guache - 2D Painter");

  init();
  glutDisplayFunc(displayStickman);
  glutMainLoop();

  return EXIT_SUCCESS;
}
