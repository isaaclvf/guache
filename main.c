#include <GL/freeglut.h>
#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>

int windowWidth = 800;
int windowHeight = 600;

typedef struct {
  float x, y;
  float color[3]; // RGB
  float size;
} Point;

typedef struct PointNode {
  Point point;
  struct PointNode *next;
} PointNode;

typedef struct PointList {
  PointNode *head;
} PointList;

PointList pointList;

void addPoint(float x, float y, float red, float green, float blue,
              float size) {
  PointNode *newNode = (PointNode *)malloc(sizeof(PointNode));

  newNode->point.x = x;
  newNode->point.y = y;
  newNode->point.color[0] = red;
  newNode->point.color[1] = green;
  newNode->point.color[2] = blue;
  newNode->point.size = size;

  newNode->next = pointList.head;
  pointList.head = newNode;
}

void renderAllPoints() {
  PointNode *current = pointList.head;

  // TODO: Encontrar uma forma de renderizar botões
  // de tamanhos diferentes. Pelo visto, não dá pra usar
  // glPointSize(current->point.size) dentro do glBegin.
  glPointSize(5.0f);

  glBegin(GL_POINTS);
  while (current != NULL) {
    glColor3f(current->point.color[0], current->point.color[1],
              current->point.color[2]);

    glVertex2f(current->point.x, current->point.y);
    current = current->next;
  }
  glEnd();
}

void freePointList() {
  PointNode *current = pointList.head;
  while (current != NULL) {
    PointNode *next = current->next;
    free(current);
    current = next;
  }
}

void initLists() { pointList.head = NULL; }

void init() {
  initLists();
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  glMatrixMode(GL_PROJECTION);
  gluOrtho2D(0.0f, windowWidth, 0.0f, windowHeight);
}

void display() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  renderAllPoints();
  glFlush();
}

// Demo de como usar antialiasing
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

// Mouse callback function to handle mouse clicks
void onMouseClick(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    // Convert window coordinates to OpenGL coordinates
    float worldX = (float)x;
    float worldY = (float)(windowHeight - y); // Flip Y-coordinate
    addPoint(worldX, worldY, 0.0f, 0.0f, 0.0f, 15.0f);
    // Redesenhar a janela
    glutPostRedisplay();
  }
}

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  // Mudar para GLUT_DOUBLE depois
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_MULTISAMPLE);
  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Guache - 2D Painter");
  // TODO: Ver glutReshapeFunc();

  // Opções para antialiasing
  // glEnable(GL_MULTISAMPLE);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glEnable(GL_LINE_SMOOTH);

  init();
  glutDisplayFunc(display);
  glutMouseFunc(onMouseClick);
  glutMainLoop();

  return EXIT_SUCCESS;
}
