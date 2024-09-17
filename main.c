#include <GL/freeglut.h>
#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_VERTICES 10

int windowWidth = 800;
int windowHeight = 600;

typedef struct {
  float x, y;
  float color[3]; // RGB
  float size;
} Point;

typedef struct {
  float x0, y0;   // Start
  float x1, y1;   // End
  float color[3]; // RGB
  // float thickness;
} Line;

typedef struct {
  int vertexCount;
  float vertices[MAX_VERTICES][2]; // Coordenadas dos vertices
  float color[3];                  // RGB
} Polygon;

typedef struct PointNode {
  Point point;
  struct PointNode *next;
} PointNode;

typedef struct LineNode {
  Line line;
  struct LineNode *next;
} LineNode;

typedef struct PolygonNode {
  Polygon polygon;
  struct PolygonNode *next;
} PolygonNode;

typedef struct PointList {
  PointNode *head;
} PointList;

typedef struct LineList {
  LineNode *head;
} LineList;

typedef struct PolygonList {
  PolygonNode *head;
} PolygonList;

PointList pointList = {NULL};
LineList lineList = {NULL};
PolygonList polygonList = {NULL};

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

void addLine(float x0, float y0, float x1, float y1, float red, float green,
             float blue) {
  LineNode *newNode = (LineNode *)malloc(sizeof(LineNode));

  newNode->line.x0 = x0;
  newNode->line.y0 = y0;
  newNode->line.x1 = x1;
  newNode->line.y1 = y1;
  newNode->line.color[0] = red;
  newNode->line.color[1] = green;
  newNode->line.color[2] = blue;

  newNode->next = lineList.head;
  lineList.head = newNode;
}

void addPolygon(float vertices[][2], int vertexCount, float red, float green,
                float blue) {
  if (vertexCount < 3) {
    printf("Erro: Um polígono precisa ter pelo menos 3 vertices.\n");
    return;
  }

  PolygonNode *newNode = (PolygonNode *)malloc(sizeof(PolygonNode));

  newNode->polygon.vertexCount = vertexCount;
  for (int i = 0; i < vertexCount; i++) {
    newNode->polygon.vertices[i][0] = vertices[i][0];
    newNode->polygon.vertices[i][1] = vertices[i][1];
  }

  newNode->polygon.color[0] = red;
  newNode->polygon.color[1] = green;
  newNode->polygon.color[2] = blue;

  newNode->next = polygonList.head;
  polygonList.head = newNode;
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

void renderAllLines() {
  LineNode *current = lineList.head;

  glBegin(GL_LINES);
  while (current != NULL) {
    glColor3f(current->line.color[0], current->line.color[1], current->line.color[2]);

    glVertex2f(current->line.x0, current->line.y0);
    glVertex2f(current->line.x1, current->line.y1);

    current = current->next;
  }
  glEnd();
}

void renderAllPolygons() {
  PolygonNode *current = polygonList.head;

  while (current != NULL) {
    glColor3f(current->polygon.color[0], current->polygon.color[1], current->polygon.color[2]);

    glBegin(GL_POLYGON);

    for (int i = 0; i < current->polygon.vertexCount; i++) {
      glVertex2f(current->polygon.vertices[i][0], current->polygon.vertices[i][1]);
    }

    glEnd();
    current = current->next;
  }
}

void freePointList() {
  PointNode *current = pointList.head;
  while (current != NULL) {
    PointNode *next = current->next;
    free(current);
    current = next;
  }
}

void freeLineList() {
  LineNode *current = lineList.head;
  while (current != NULL) {
    LineNode *next = current->next;
    free(current);
    current = next;
  }
}

void freePolygonList() {
  PolygonNode *current = polygonList.head;
  while (current != NULL) {
    PolygonNode *next = current->next;
    free(current);
    current = next;
  }
}

void init() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  glMatrixMode(GL_PROJECTION);
  gluOrtho2D(0.0f, windowWidth, 0.0f, windowHeight);
}

void display() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  renderAllPoints();
  renderAllLines();
  renderAllPolygons();

  glFlush();
  // glutSwapBuffers();
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

void onMouseClick(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    // Converte coordenadas da janela para coordenadas do OpenGL
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
