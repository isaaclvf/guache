#include <GL/freeglut.h>
#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "color.h"

#define MAX_VERTICES 10

int   windowWidth  = 800;
int   windowHeight = 600;
float tolerance    = 5.0;

char saveFile[256] = "";  // Nome do arquivo para salvar
int  shouldSave    = 0;   // Flag que indica se o programa vai salvar em um arquivo
int  shouldLoad    = 0;

typedef enum { DRAW_POINT, DRAW_LINE, DRAW_POLYGON, SELECT } Mode;
Mode currentMode = DRAW_POINT;

float currentColor[] = {0.0f, 0.0f, 0.0f};

float rotationAngle = 0.0f;
int   shouldRotate  = 0;

typedef struct {
  float matrix[16];
  float tx, ty;
  float angle;
  float scale;
  float shearX, shearY;
  int   reflectX;
  int   reflectY;
} Transformation;

typedef struct {
  float x, y;
  float color[3];  // RGB
  float size;

  Transformation transformation;
} Point;

typedef struct {
  float x0, y0;    // Start
  float x1, y1;    // End
  float color[3];  // RGB
  float width;

  Transformation transformation;
} Line;

typedef struct {
  int   vertexCount;
  float vertices[MAX_VERTICES][2];  // Coordenadas dos vertices
  float color[3];                   // RGB

  Transformation transformation;
} Polygon;

// Usado na criação de objetos
float   tempLineX0, tempLineY0;
float   currentMouseX, currentMouseY;
Polygon tempPolygon = {
    .color       = {0.5f, 0.5f, 0.5f},
    .vertexCount = 0,
};

// Flags para marcar se estamos no processo de desenhar
int isDrawingLine    = 0;
int isDrawingPolygon = 0;

typedef struct PointNode {
  Point             point;
  struct PointNode *next;
  struct PointNode *prev;
} PointNode;

typedef struct LineNode {
  Line             line;
  struct LineNode *next;
  struct LineNode *prev;
} LineNode;

typedef struct PolygonNode {
  Polygon             polygon;
  struct PolygonNode *next;
  struct PolygonNode *prev;
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

PointList   pointList   = {NULL};
LineList    lineList    = {NULL};
PolygonList polygonList = {NULL};

PointNode   *selectedPoint   = {NULL};
LineNode    *selectedLine    = {NULL};
PolygonNode *selectedPolygon = {NULL};

int isAnythingSelected = 0;

int   isDragging = 0;
float dragStartX, dragStartY;

float previousColor[3] = {0.0f, 0.0f, 0.0f};

void clearSelection() {
  if (selectedPoint != NULL) {
    for (int i = 0; i < 3; i++)
      selectedPoint->point.color[i] = previousColor[i];
  }

  if (selectedLine != NULL) {
    for (int i = 0; i < 3; i++) {
      selectedLine->line.color[i] = previousColor[i];
    }
  }

  if (selectedPolygon != NULL) {
    for (int i = 0; i < 3; i++) {
      selectedPolygon->polygon.color[i] = previousColor[i];
    }
  }

  isAnythingSelected = 0;
  selectedPoint      = NULL;
  selectedLine       = NULL;
  selectedPolygon    = NULL;
}

void addPoint(float x, float y, float red, float green, float blue, float size) {
  PointNode *newNode = (PointNode *)malloc(sizeof(PointNode));

  newNode->point.x        = x;
  newNode->point.y        = y;
  newNode->point.color[0] = red;
  newNode->point.color[1] = green;
  newNode->point.color[2] = blue;
  newNode->point.size     = size;
  newNode->prev           = NULL;

  newNode->point.transformation.tx       = 0.0f;
  newNode->point.transformation.ty       = 0.0f;
  newNode->point.transformation.angle    = 0.0f;
  newNode->point.transformation.scale    = 1.0f;
  newNode->point.transformation.shearX   = 0.0f;
  newNode->point.transformation.shearY   = 0.0f;
  newNode->point.transformation.reflectX = 0;
  newNode->point.transformation.reflectY = 0;

  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetFloatv(GL_MODELVIEW_MATRIX, newNode->point.transformation.matrix);
  glPopMatrix();

  PointNode *oldHead = pointList.head;

  if (oldHead) {
    oldHead->prev = newNode;
  }

  newNode->next  = oldHead;
  pointList.head = newNode;
}

void addPointNode(PointNode *newNode) {
  PointNode *oldHead = pointList.head;

  if (oldHead) {
    oldHead->prev = newNode;
  }

  newNode->next  = oldHead;
  pointList.head = newNode;
}

void addLine(float x0, float y0, float x1, float y1, float red, float green, float blue) {
  LineNode *newNode = (LineNode *)malloc(sizeof(LineNode));

  newNode->line.x0       = x0;
  newNode->line.y0       = y0;
  newNode->line.x1       = x1;
  newNode->line.y1       = y1;
  newNode->line.color[0] = red;
  newNode->line.color[1] = green;
  newNode->line.color[2] = blue;
  newNode->prev          = NULL;

  newNode->line.transformation.tx       = 0.0f;
  newNode->line.transformation.ty       = 0.0f;
  newNode->line.transformation.angle    = 0.0f;
  newNode->line.transformation.scale    = 1.0f;
  newNode->line.transformation.shearX   = 0.0f;
  newNode->line.transformation.shearY   = 0.0f;
  newNode->line.transformation.reflectX = 0;
  newNode->line.transformation.reflectY = 0;

  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetFloatv(GL_MODELVIEW_MATRIX, newNode->line.transformation.matrix);
  glPopMatrix();

  LineNode *oldHead = lineList.head;

  if (oldHead) {
    oldHead->prev = newNode;
  }

  newNode->next = oldHead;
  lineList.head = newNode;
}

void addLineNode(LineNode *newNode) {
  LineNode *oldHead = lineList.head;

  if (oldHead) {
    oldHead->prev = newNode;
  }

  newNode->next = oldHead;
  lineList.head = newNode;
}

void addPolygon(float vertices[][2], int vertexCount, float red, float green, float blue) {
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
  newNode->prev             = NULL;

  newNode->polygon.transformation.tx       = 0.0f;
  newNode->polygon.transformation.ty       = 0.0f;
  newNode->polygon.transformation.angle    = 0.0f;
  newNode->polygon.transformation.scale    = 1.0f;
  newNode->polygon.transformation.shearX   = 0.0f;
  newNode->polygon.transformation.shearY   = 0.0f;
  newNode->polygon.transformation.reflectX = 0;
  newNode->polygon.transformation.reflectY = 0;

  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetFloatv(GL_MODELVIEW_MATRIX, newNode->polygon.transformation.matrix);
  glPopMatrix();

  PolygonNode *oldHead = polygonList.head;

  if (oldHead) {
    oldHead->prev = newNode;
  }

  newNode->next    = oldHead;
  polygonList.head = newNode;
}

void addPolygonNode(PolygonNode *newNode) {
  PolygonNode *oldHead = polygonList.head;

  if (oldHead) {
    oldHead->prev = newNode;
  }

  newNode->next    = oldHead;
  polygonList.head = newNode;
}

void debugTransformation(Transformation *t) {
  if (!t) {
    printf("Erro: A transformação é NULL.\n");
    return;
  }

  printf("Transformação:\n");
  printf("  Matriz de Transformação (4x4):\n");
  for (int i = 0; i < 4; i++) {
    printf("  [");
    for (int j = 0; j < 4; j++) {
      printf(" %.2f ", t->matrix[i * 4 + j]);
    }
    printf("]\n");
  }

  printf("  Translação: (tx = %.2f, ty = %.2f)\n", t->tx, t->ty);
  printf("  Rotação: %.2f graus\n", t->angle);
  printf("  Escala: %.2f\n", t->scale);
  printf("  Cisalhamento: (shearX = %.2f, shearY = %.2f)\n", t->shearX, t->shearY);
  printf("  Reflexão: (reflectX = %d, reflectY = %d)\n", t->reflectX, t->reflectY);
}

void debugPoint(PointNode *pointNode) {
  if (!pointNode) {
    printf("Erro: O PointNode é NULL.\n");
    return;
  }

  printf("Informações do Ponto:\n");
  printf("  Coordenadas: (%.2f, %.2f)\n", pointNode->point.x, pointNode->point.y);
  printf("  Cor (RGB): (%.2f, %.2f, %.2f)\n", pointNode->point.color[0], pointNode->point.color[1],
         pointNode->point.color[2]);
  printf("  Tamanho: %.2f\n", pointNode->point.size);

  debugTransformation(&pointNode->point.transformation);
  printf("\n");
}

void debugLine(LineNode *lineNode) {
  if (!lineNode) {
    printf("Erro: O LineNode é NULL.\n");
    return;
  }

  printf("Informações da Linha:\n");
  printf("  Ponto Inicial: (%.2f, %.2f)\n", lineNode->line.x0, lineNode->line.y0);
  printf("  Ponto Final: (%.2f, %.2f)\n", lineNode->line.x1, lineNode->line.y1);
  printf("  Cor (RGB): (%.2f, %.2f, %.2f)\n", lineNode->line.color[0], lineNode->line.color[1],
         lineNode->line.color[2]);

  debugTransformation(&lineNode->line.transformation);
  printf("\n");
}

void debugPolygon(PolygonNode *polygonNode) {
  if (!polygonNode) {
    printf("Erro: O PolygonNode é NULL.\n");
    return;
  }

  printf("Informações do Polígono:\n");
  printf("  Cor (RGB): (%.2f, %.2f, %.2f)\n", polygonNode->polygon.color[0],
         polygonNode->polygon.color[1], polygonNode->polygon.color[2]);
  printf("  Número de Vértices: %d\n", polygonNode->polygon.vertexCount);

  printf("  Vértices:\n");
  for (int i = 0; i < polygonNode->polygon.vertexCount; i++) {
    printf("    Vértice %d: (%.2f, %.2f)\n", i, polygonNode->polygon.vertices[i][0],
           polygonNode->polygon.vertices[i][1]);
  }

  debugTransformation(&polygonNode->polygon.transformation);
  printf("\n");
}

// Função auxiliar para debug
void verifyLoadedObjects() {
  PointNode *currentPoint = pointList.head;
  while (currentPoint) {
    debugPoint(currentPoint);
    currentPoint = currentPoint->next;
  }

  LineNode *currentLine = lineList.head;
  while (currentLine) {
    debugLine(currentLine);
    currentLine = currentLine->next;
  }

  PolygonNode *currentPolygon = polygonList.head;
  while (currentPolygon) {
    debugPolygon(currentPolygon);
    currentPolygon = currentPolygon->next;
  }
}

void updatePointTransformationMatrix(PointNode *pointNode) {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // glScalef(pointNode->point.transformation.scale,
  //          pointNode->point.transformation.scale, 0.0f);

  GLfloat shearMatrix[16] = {1.0f,
                             pointNode->point.transformation.shearY,
                             0.0f,
                             0.0f,
                             pointNode->point.transformation.shearX,
                             1.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             1.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             1.0f};
  glMultMatrixf(shearMatrix);

  if (pointNode->point.transformation.reflectX) {
    GLfloat reflectXMatrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
                                  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f};
    glMultMatrixf(reflectXMatrix);
  }

  if (pointNode->point.transformation.reflectY) {
    GLfloat reflectYMatrix[16] = {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    glMultMatrixf(reflectYMatrix);
  }

  glRotatef(pointNode->point.transformation.angle, 0.0, 0.0, 1.0);
  glTranslatef(pointNode->point.transformation.tx, pointNode->point.transformation.ty, 0.0f);

  // Save the updated transformation matrix
  glGetFloatv(GL_MODELVIEW_MATRIX, pointNode->point.transformation.matrix);
}

void updateLineTransformationMatrix(LineNode *lineNode) {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(lineNode->line.transformation.tx, lineNode->line.transformation.ty, 0.0f);

  GLfloat shearMatrix[16] = {1.0f,
                             lineNode->line.transformation.shearY,
                             0.0f,
                             0.0f,
                             lineNode->line.transformation.shearX,
                             1.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             1.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             1.0f};
  glMultMatrixf(shearMatrix);

  if (lineNode->line.transformation.reflectX) {
    GLfloat reflectXMatrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
                                  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f};
    glMultMatrixf(reflectXMatrix);
  }

  if (lineNode->line.transformation.reflectY) {
    GLfloat reflectYMatrix[16] = {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    glMultMatrixf(reflectYMatrix);
  }

  float centerX = (lineNode->line.x0 + lineNode->line.x1) / 2.0f;
  float centerY = (lineNode->line.y0 + lineNode->line.y1) / 2.0f;

  glTranslatef(centerX, centerY, 0.0f);
  glScalef(lineNode->line.transformation.scale, lineNode->line.transformation.scale, 0.0f);
  glRotatef(lineNode->line.transformation.angle, 0.0, 0.0, 1.0);
  glTranslatef(-centerX, -centerY, 0.0f);

  // Save the updated transformation matrix
  glGetFloatv(GL_MODELVIEW_MATRIX, lineNode->line.transformation.matrix);
}

void updatePolygonTransformationMatrix(PolygonNode *polygonNode) {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(polygonNode->polygon.transformation.tx, polygonNode->polygon.transformation.ty,
               0.0f);

  GLfloat shearMatrix[16] = {1.0f,
                             polygonNode->polygon.transformation.shearY,
                             0.0f,
                             0.0f,
                             polygonNode->polygon.transformation.shearX,
                             1.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             1.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             1.0f};
  glMultMatrixf(shearMatrix);

  if (polygonNode->polygon.transformation.reflectX) {
    GLfloat reflectXMatrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
                                  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f};
    glMultMatrixf(reflectXMatrix);
  }

  if (polygonNode->polygon.transformation.reflectY) {
    GLfloat reflectYMatrix[16] = {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    glMultMatrixf(reflectYMatrix);
  }

  float centerX = 0.0f;
  float centerY = 0.0f;

  for (int i = 0; i < polygonNode->polygon.vertexCount; i++) {
    centerX += polygonNode->polygon.vertices[i][0];
    centerY += polygonNode->polygon.vertices[i][1];
  }

  centerX = centerX / polygonNode->polygon.vertexCount;
  centerY = centerY / polygonNode->polygon.vertexCount;

  glTranslatef(centerX, centerY, 0.0f);
  glScalef(polygonNode->polygon.transformation.scale, polygonNode->polygon.transformation.scale,
           0.0f);
  glRotatef(polygonNode->polygon.transformation.angle, 0.0, 0.0, 1.0);
  glTranslatef(-centerX, -centerY, 0.0f);

  // Save the updated transformation matrix
  glGetFloatv(GL_MODELVIEW_MATRIX, polygonNode->polygon.transformation.matrix);
}

void removePointNode(PointNode *node) {
  if (!node) {
    return;
  }

  if (node == pointList.head) {
    pointList.head = node->next;
    if (pointList.head) {
      pointList.head->prev = NULL;
    }
  } else {
    if (node->prev) {
      node->prev->next = node->next;
    }
  }

  if (!node->next) {
    if (node->prev) {
      node->prev->next = NULL;
    }
  } else {
    if (node->next) {
      node->next->prev = node->prev;
    }
  }

  free(node);
  selectedPoint = NULL;
}

void removeLineNode(LineNode *node) {
  if (!node) {
    return;
  }

  if (node == lineList.head) {
    lineList.head = node->next;
    if (lineList.head) {
      lineList.head->prev = NULL;
    }
  } else {
    if (node->prev) {
      node->prev->next = node->next;
    }
  }

  if (!node->next) {
    if (node->prev) {
      node->prev->next = NULL;
    }
  } else {
    if (node->next) {
      node->next->prev = node->prev;
    }
  }

  free(node);
  selectedLine = NULL;
}

void removePolygonNode(PolygonNode *node) {
  if (!node) {
    return;
  }

  if (node == polygonList.head) {
    polygonList.head = node->next;
    if (polygonList.head) {
      polygonList.head->prev = NULL;
    }
  } else {
    if (node->prev) {
      node->prev->next = node->next;
    }
  }

  if (!node->next) {
    if (node->prev) {
      node->prev->next = NULL;
    }
  } else {
    if (node->next) {
      node->next->prev = node->prev;
    }
  }

  free(node);
  selectedPolygon = NULL;
}

void renderAllPoints() {
  PointNode *current = pointList.head;

  while (current != NULL) {
    glPointSize(5.0f);
    glLoadMatrixf(current->point.transformation.matrix);

    glBegin(GL_POINTS);

    glColor3f(current->point.color[0], current->point.color[1], current->point.color[2]);

    glVertex2f(current->point.x, current->point.y);
    glEnd();

    current = current->next;
  }
}

void renderAllLines() {
  LineNode *current = lineList.head;

  while (current != NULL) {
    glLoadMatrixf(current->line.transformation.matrix);
    glLineWidth(3.0f);
    glBegin(GL_LINES);

    glColor3f(current->line.color[0], current->line.color[1], current->line.color[2]);

    glVertex2f(current->line.x0, current->line.y0);
    glVertex2f(current->line.x1, current->line.y1);

    glEnd();
    current = current->next;
  }
}

void renderAllPolygons() {
  PolygonNode *current = polygonList.head;

  while (current != NULL) {
    // Solução temporária para não desenhar o polígono selecionado
    // se estiver no modo animação. Se não tiver isso, é desenhado
    // um polígono a mais (que fica parado) e outro que fica girando.
    if (shouldRotate && current == selectedPolygon) {
      current = current->next;
      continue;
    }

    glPushMatrix();
    glColor3f(current->polygon.color[0], current->polygon.color[1], current->polygon.color[2]);

    glLoadMatrixf(current->polygon.transformation.matrix);
    glBegin(GL_POLYGON);

    for (int i = 0; i < current->polygon.vertexCount; i++) {
      glVertex2f(current->polygon.vertices[i][0], current->polygon.vertices[i][1]);
    }

    glEnd();
    glPopMatrix();
    current = current->next;
  }

  if (isDrawingPolygon && tempPolygon.vertexCount > 0) {
    glPushMatrix();
    glColor3f(tempPolygon.color[0], tempPolygon.color[1], tempPolygon.color[2]);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < tempPolygon.vertexCount; i++) {
      glVertex2f(tempPolygon.vertices[i][0], tempPolygon.vertices[i][1]);
    }
    glEnd();
    glPopMatrix();
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

// selection algorithms

PointNode *selectPoint(int sx, int sy) {
  PointNode *current = pointList.head;

  while (current != NULL) {
    if ((current->point.x <= sx + tolerance) && (current->point.x >= sx - tolerance) &&
        (current->point.y <= sy + tolerance) && (current->point.y >= sy - tolerance)) {
      previousColor[0] = current->point.color[0];
      previousColor[1] = current->point.color[1];
      previousColor[2] = current->point.color[2];

      current->point.color[0] = 1.0;
      current->point.color[1] = 0.0;
      current->point.color[2] = 0.0;
      isAnythingSelected      = 1;
      return current;
    }
    current = current->next;
  }
  return NULL;
}

int auxSelectLine(float x, float y, float X, float Y) {
  int xmax = x + tolerance, xmin = x - tolerance, ymax = y + tolerance, ymin = y - tolerance;

  if ((X <= xmax) && (X >= xmin) && (Y <= ymax) && (Y >= ymin)) {
    return 1;
  }

  return 0;
}

LineNode *selectLine(int sx, int sy) {
  LineNode *current = lineList.head;

  int   xmax = sx + tolerance, xmin = sx - tolerance, ymax = sy + tolerance, ymin = sy - tolerance;
  int   select = 0;
  float tempVertice;

  while (current != NULL) {
    if (auxSelectLine(sx, sy, current->line.x0, current->line.y0) ||
        auxSelectLine(sx, sy, current->line.x1, current->line.y1)) {
      select = 1;
    } else {
      //[TO DO] conferir para x0 E x1, y0 E y1!!!
      if (current->line.x0 < xmin) {
        tempVertice = current->line.y0 +
            ((xmin - current->line.x0) * (current->line.y1 - current->line.y0) /
             (current->line.x1 - current->line.x0));
        if (auxSelectLine(sx, sy, xmin, tempVertice)) {
          select = 1;
        }
      } else if (current->line.x0 > xmax) {
        tempVertice = current->line.y0 +
            ((xmax - current->line.x0) * (current->line.y1 - current->line.y0) /
             (current->line.x1 - current->line.x0));
        if (auxSelectLine(sx, sy, xmax, tempVertice)) {
          select = 1;
        }
      } else if (current->line.y0 < ymin) {
        tempVertice = current->line.x0 +
            ((ymin - current->line.y0) * (current->line.x1 - current->line.x0) /
             (current->line.y1 - current->line.y0));
        if (auxSelectLine(sx, sy, tempVertice, ymin)) {
          select = 1;
        }
      } else if (current->line.y0 > ymax) {
        tempVertice = current->line.x0 +
            ((ymax - current->line.y0) * (current->line.x1 - current->line.x0) /
             (current->line.y1 - current->line.y0));
        if (auxSelectLine(sx, sy, tempVertice, ymax)) {
          select = 1;
        }
      } else if (current->line.x1 < xmin) {
        tempVertice = current->line.y1 +
            ((xmin - current->line.x1) * (current->line.y1 - current->line.y0) /
             (current->line.x1 - current->line.x0));
        if (auxSelectLine(sx, sy, xmin, tempVertice)) {
          select = 1;
        }
      } else if (current->line.x1 > xmax) {
        tempVertice = current->line.y1 +
            ((xmax - current->line.x1) * (current->line.y1 - current->line.y0) /
             (current->line.x1 - current->line.x0));
        if (auxSelectLine(sx, sy, xmax, tempVertice)) {
          select = 1;
        }
      } else if (current->line.y1 < ymin) {
        tempVertice = current->line.x1 +
            ((ymin - current->line.y1) * (current->line.x1 - current->line.x0) /
             (current->line.y1 - current->line.y0));
        if (auxSelectLine(sx, sy, tempVertice, ymin)) {
          select = 1;
        }
      } else if (current->line.y1 > ymax) {
        tempVertice = current->line.x1 +
            ((ymax - current->line.y1) * (current->line.x1 - current->line.x0) /
             (current->line.y1 - current->line.y0));
        if (auxSelectLine(sx, sy, tempVertice, ymax)) {
          select = 1;
        }
      }
    }
    if (select) {
      previousColor[0] = current->line.color[0];
      previousColor[1] = current->line.color[1];
      previousColor[2] = current->line.color[2];

      current->line.color[0] = 1.0;
      current->line.color[1] = 0.0;
      current->line.color[2] = 0.0;

      isAnythingSelected = 1;
      return current;
    }
    current = current->next;
  }
  return NULL;
}

PolygonNode *selectPolygon(int sx, int sy) {
  PolygonNode *current = polygonList.head;
  float        x1, y1, x0, y0;
  int          walls;

  while (current != NULL) {
    walls = 0;
    for (int i = 0; i < current->polygon.vertexCount; i++) {
      if (i == 0) {
        x1 = current->polygon.vertices[i][0];
        y1 = current->polygon.vertices[i][1];
        x0 = current->polygon.vertices[current->polygon.vertexCount - 1][0];
        y0 = current->polygon.vertices[current->polygon.vertexCount - 1][1];
      } else {
        x1 = current->polygon.vertices[i][0];
        y1 = current->polygon.vertices[i][1];
        x0 = current->polygon.vertices[i - 1][0];
        y0 = current->polygon.vertices[i - 1][1];
      }

      if (((x0 > sx) && (x1 > sx)) && (((y0 > sy) && (y1 < sy)) || ((y1 > sy) && (y0 < sy)))) {
        walls++;
      } else if ((((x0 < sx) && (x1 > sx)) || ((x1 < sx) && (x0 > sx))) &&
                 (((y0 > sy) && (y1 < sy)) || ((y1 > sy) && (y0 < sy)))) {
        int xi = x0 + (((sy - y0) * (x1 - x0)) / (y1 - y0));
        if (xi > sx) {
          walls++;
        }
      }
    }

    if (walls % 2 != 0) {
      previousColor[0] = current->polygon.color[0];
      previousColor[1] = current->polygon.color[1];
      previousColor[2] = current->polygon.color[2];

      current->polygon.color[0] = 1.0;
      current->polygon.color[1] = 0.0;
      current->polygon.color[2] = 0.0;
      isAnythingSelected        = 1;

      return current;
    }

    current = current->next;
  }

  return NULL;
}

void loadFromFile(const char *filename);

void init() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  glMatrixMode(GL_PROJECTION);
  gluOrtho2D(-400.0f, 400.0f, -300.0f, 300.0f);

  if (shouldLoad) {
    loadFromFile(saveFile);
    shouldLoad = 0;  // Carrega apenas uma vez
  }
}

void renderTestPolygon() {
  glColor3f(1.0f, 0.0f, 0.0f);  // Vermelho
  glBegin(GL_TRIANGLES);
  glVertex2f(0.0f, 100.0f);
  glVertex2f(-50.0f, -50.0f);
  glVertex2f(50.0f, -50.0f);
  glEnd();
}

void updateAnimation(int value) {
  rotationAngle += 2.0f;

  if (rotationAngle > 360.0f) {
    rotationAngle -= 360.0f;
  }

  glutPostRedisplay();

  // Chama novamente essa função após 16ms (~60fps)
  glutTimerFunc(16, updateAnimation, 0);
}

void renderPolygonWithAnimation(Polygon *polygon) {
  glPushMatrix();  // Salva a matriz atual

  float centerX = 0.0f;
  float centerY = 0.0f;

  for (int i = 0; i < polygon->vertexCount; i++) {
    centerX += polygon->vertices[i][0];
    centerY += polygon->vertices[i][1];
  }

  centerX = centerX / polygon->vertexCount;
  centerY = centerY / polygon->vertexCount;
  glTranslatef(centerX, centerY, 0);

  polygon->transformation.angle = rotationAngle;
  glRotatef(polygon->transformation.angle, 0.0f, 0.0f, 1.0f);

  glTranslatef(-centerX, -centerY, 0);

  glColor3f(previousColor[0], previousColor[1], previousColor[2]);
  glBegin(GL_POLYGON);
  for (int i = 0; i < polygon->vertexCount; i++) {
    glVertex2f(polygon->vertices[i][0], polygon->vertices[i][1]);
  }
  glEnd();

  glPopMatrix();
}

void display() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  renderAllPolygons();
  renderAllLines();
  renderAllPoints();

  // Se estiver no processo de desenhar uma linha, mostrar um preview
  if (isDrawingLine) {
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    glVertex2f(tempLineX0, tempLineY0);
    glVertex2f(currentMouseX, currentMouseY);
    glEnd();
  }

  if (selectedPolygon != NULL && shouldRotate) {
    renderPolygonWithAnimation(&selectedPolygon->polygon);
  }

  glutSwapBuffers();
}

// Usado para fechar o polígono
int isCloseToFirstPoint(float x, float y) {
  if (tempPolygon.vertexCount == 0)
    return 0;  // Nenhum vértice ainda
  float dx = x - tempPolygon.vertices[0][0];
  float dy = y - tempPolygon.vertices[0][1];
  return sqrt(dx * dx + dy * dy) < tolerance;
}

void onMouseClick(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    // Converte coordenadas da janela para coordenadas do OpenGL
    float worldX = x - ((float)windowWidth / 2);
    float worldY = -y + ((float)windowHeight / 2);

    if (selectedPoint || selectedLine || selectedPolygon) {
      isDragging = 1;
      dragStartX = worldX;
      dragStartY = worldY;
    }

    if (currentMode == DRAW_POINT) {
      addPoint(worldX, worldY, currentColor[0], currentColor[1], currentColor[2], 5.0f);
    } else if (currentMode == DRAW_LINE) {
      if (!isDrawingLine) {
        tempLineX0    = worldX;
        tempLineY0    = worldY;
        currentMouseX = worldX;
        currentMouseY = worldY;
        isDrawingLine = 1;
      } else {
        // Se chegou aqui está no segundo clique do desenho de linha
        addLine(tempLineX0, tempLineY0, worldX, worldY, currentColor[0], currentColor[1],
                currentColor[2]);
        isDrawingLine = 0;
      }
    } else if (currentMode == DRAW_POLYGON) {
      if (!isDrawingPolygon) {
        isDrawingPolygon        = 1;
        tempPolygon.vertexCount = 0;
      }

      if (tempPolygon.vertexCount < MAX_VERTICES) {
        if (isCloseToFirstPoint(worldX, worldY) && tempPolygon.vertexCount > 2) {
          glPushMatrix();
          addPolygon(tempPolygon.vertices, tempPolygon.vertexCount, currentColor[0],
                     currentColor[1], currentColor[2]);
          isDrawingPolygon = 0;
          glPopMatrix();
        } else {
          // Adiciona vértice ao polígono
          tempPolygon.vertices[tempPolygon.vertexCount][0] = worldX;
          tempPolygon.vertices[tempPolygon.vertexCount][1] = worldY;
          tempPolygon.vertexCount++;
        }
      }

      if (tempPolygon.vertexCount == MAX_VERTICES) {
        addPolygon(tempPolygon.vertices, tempPolygon.vertexCount, currentColor[0], currentColor[1],
                   currentColor[1]);
        // Começa a desenhar um novo
        tempPolygon.vertexCount = 0;
      }
    } else if (currentMode == SELECT) {
      if (isAnythingSelected != 0 || selectedPoint != NULL) {
        clearSelection();
      }

      selectedPoint = selectPoint(worldX, worldY);

      if (isAnythingSelected == 0) {
        selectedLine = selectLine(worldX, worldY);
        if (isAnythingSelected == 0) {
          selectedPolygon = selectPolygon(worldX, worldY);
        }
      }
    }
  } else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
    isDragging = 0;
  }
  glutPostRedisplay();
}

void mouseMoveCallback(int x, int y) {
  if (isDrawingLine) {
    currentMouseX = x - ((float)windowWidth / 2);
    currentMouseY = -y + ((float)windowHeight / 2);
    glutPostRedisplay();  // Redesenha a cena para preview da linha
  }
}

void mouseDragCallback(int x, int y) {
  if (isDragging) {
    float mouseX = x - ((float)windowWidth / 2);
    float mouseY = -y + ((float)windowHeight / 2);

    float dx = mouseX - dragStartX;
    float dy = mouseY - dragStartY;

    if (selectedPoint) {
      selectedPoint->point.transformation.tx += dx;
      selectedPoint->point.transformation.ty += dy;
      updatePointTransformationMatrix(selectedPoint);
    } else if (selectedLine) {
      selectedLine->line.transformation.tx += dx;
      selectedLine->line.transformation.ty += dy;
      updateLineTransformationMatrix(selectedLine);
    } else if (selectedPolygon) {
      selectedPolygon->polygon.transformation.tx += dx;
      selectedPolygon->polygon.transformation.ty += dy;
      updatePolygonTransformationMatrix(selectedPolygon);
    }

    dragStartX = mouseX;
    dragStartY = mouseY;

    glutPostRedisplay();
  }
}

void saveToFile(const char *filename);

void updateCurrentColor(unsigned char key) {
  switch (key) {
    case '1':
      currentColor[0] = 0.0f;
      currentColor[1] = 0.0f;
      currentColor[2] = 0.0f;
      printf("%s%sPreto%s\n", BLKB, UWHT, CRESET);
      break;
    case '2':
      currentColor[0] = 1.0f;
      currentColor[1] = 0.0f;
      currentColor[2] = 0.0f;
      printf("%s%sVermelho%s\n", REDB, UWHT, CRESET);
      break;
    case '3':
      currentColor[0] = 0.0f;
      currentColor[1] = 1.0f;
      currentColor[2] = 0.0f;
      printf("%s%sVerde%s\n", GRNB, UWHT, CRESET);
      break;
    case '4':
      currentColor[0] = 0.0f;
      currentColor[1] = 0.0f;
      currentColor[2] = 1.0f;
      printf("%s%sAzul%s\n", BLUB, UWHT, CRESET);
      break;
    case '5':
      currentColor[0] = 1.0f;
      currentColor[1] = 1.0f;
      currentColor[2] = 0.0f;
      printf("%s%sAmarelo%s\n", YELB, UWHT, CRESET);
      break;
    case '6':
      currentColor[0] = 1.0f;
      currentColor[1] = 0.0f;
      currentColor[2] = 1.0f;
      printf("%s%sMagenta%s\n", MAGB, UWHT, CRESET);
      break;
    case '7':
      currentColor[0] = 0.0f;
      currentColor[1] = 1.0f;
      currentColor[2] = 1.0f;
      printf("%s%sCiano%s\n", CYNB, UWHT, CRESET);
      break;
  }
}

void updateTransformation(Transformation *transformation, unsigned char key) {
  switch (key) {
    case 'q':
      glPushMatrix();
      transformation->angle += 5.0f;
      break;
    case 'e':
      transformation->angle -= 5.0f;
      break;
    case 'w':
      transformation->ty += 5.0f;
      break;
    case 's':
      transformation->ty -= 5.0f;
      break;
    case 'a':
      transformation->tx -= 5.0f;
      break;
    case 'd':
      transformation->tx += 5.0f;
      break;
    case '+':
      transformation->scale += 0.1f;
      break;
    case '-':
      transformation->scale -= 0.1f;
      break;
    case 'i':
      transformation->shearX += 0.05f;
      break;
    case 'k':
      transformation->shearX -= 0.05f;
      break;
    case 'j':
      transformation->shearY += 0.05f;
      break;
    case 'l':
      transformation->shearY -= 0.05f;
      break;
    case 'x':
      transformation->reflectX = !transformation->reflectX;
      break;
    case 'y':
      transformation->reflectY = !transformation->reflectY;
      break;
  }
}

void keyPress(unsigned char key, int x, int y) {
  if (key == '.' && shouldSave) {
    if (strlen(saveFile) > 0) {
      saveToFile(saveFile);
      printf("Salvo em %s\n", saveFile);
    } else {
      printf(
          "Nenhum arquivo especificado. Use --save para especificar um "
          "arquivo.\n");
    }

  } else if (key == 'v') {
    clearSelection();
    currentMode = DRAW_POINT;
    printf("Desenho de \e[1;32mponto\e[0m selecionado.\n");

  } else if (key == 'b') {
    clearSelection();
    currentMode = DRAW_LINE;
    printf("Desenho de \e[1;32mlinha\e[0m selecionado.\n");

  } else if (key == 'n') {
    clearSelection();
    currentMode = DRAW_POLYGON;
    printf("Desenho de \e[1;32mpolígono\e[0m selecionado.\n");

  } else if (key == 'm') {
    currentMode = SELECT;
    printf("Modo de \e[1;32mseleção\e[0m selecionado.\n");

  } else if (key == '0') {
    if (selectedPoint) {
      removePointNode(selectedPoint);
      isAnythingSelected = 0;
    }

    if (selectedLine) {
      removeLineNode(selectedLine);
      isAnythingSelected = 0;
    }

    if (selectedPolygon) {
      removePolygonNode(selectedPolygon);
      isAnythingSelected = 0;
    }
  } else if (key == 'z') {
    if (shouldRotate) {
      shouldRotate = 0;
      printf("\e[1;32mAnimação\e[0m desligada.\n");
    } else {
      shouldRotate = 1;
      printf("\e[1;32mAnimação\e[0m ligada.\n");
    }
  } else if (key != 'z') {
    if (shouldRotate) {
      printf("\e[1;32mAnimação\e[0m desligada.\n");
      shouldRotate = 0;
    }
  }

  updateCurrentColor(key);

  if (selectedPoint) {
    updateTransformation(&selectedPoint->point.transformation, key);
    updatePointTransformationMatrix(selectedPoint);
  }

  if (selectedLine) {
    updateTransformation(&selectedLine->line.transformation, key);
    updateLineTransformationMatrix(selectedLine);
  }

  if (selectedPolygon) {
    updateTransformation(&selectedPolygon->polygon.transformation, key);
    glPushMatrix();
    updatePolygonTransformationMatrix(selectedPolygon);
    glPopMatrix();
  }

  if (currentMode == DRAW_POINT || currentMode == SELECT || currentMode == DRAW_LINE) {
    tempPolygon.vertexCount = 0;
  }
  if (currentMode == DRAW_POINT || currentMode == SELECT || currentMode == DRAW_POLYGON) {
    isDrawingLine = 0;
  }
  glutPostRedisplay();  // Redesenha a cena para preview da linha
}

void saveToFile(const char *filename) {
  FILE *file = fopen(filename, "w");
  if (file == NULL) {
    printf("Erro: Não foi possível abrir o arquivo para salvar.\n");
    return;
  }

  PointNode *currentPoint = pointList.head;
  while (currentPoint != NULL) {
    float xn = currentPoint->point.x / 400.0f;
    float yn = currentPoint->point.y / 300.0f;

    fprintf(file, "point %f %f %f %f %f %f ", xn, yn, currentPoint->point.color[0],
            currentPoint->point.color[1], currentPoint->point.color[2], currentPoint->point.size);

    for (int i = 0; i < 16; i++) {
      fprintf(file, "%f ", currentPoint->point.transformation.matrix[i]);
    }

    fprintf(file, "%f %f %f %f %f %f %d %d\n", currentPoint->point.transformation.tx,
            currentPoint->point.transformation.ty, currentPoint->point.transformation.angle,
            currentPoint->point.transformation.scale, currentPoint->point.transformation.shearX,
            currentPoint->point.transformation.shearY, currentPoint->point.transformation.reflectX,
            currentPoint->point.transformation.reflectY);

    currentPoint = currentPoint->next;
  }

  LineNode *currentLine = lineList.head;
  while (currentLine != NULL) {
    float xn0 = currentLine->line.x0 / 400.0f;
    float yn0 = currentLine->line.y0 / 300.0f;
    float xn1 = currentLine->line.x1 / 400.0f;
    float yn1 = currentLine->line.y1 / 300.0f;

    fprintf(file, "line %f %f %f %f %f %f %f ", xn0, yn0, xn1, yn1, currentLine->line.color[0],
            currentLine->line.color[1], currentLine->line.color[2]);

    for (int i = 0; i < 16; i++) {
      fprintf(file, "%f ", currentLine->line.transformation.matrix[i]);
    }

    fprintf(file, "%f %f %f %f %f %f %d %d\n", currentLine->line.transformation.tx,
            currentLine->line.transformation.ty, currentLine->line.transformation.angle,
            currentLine->line.transformation.scale, currentLine->line.transformation.shearX,
            currentLine->line.transformation.shearY, currentLine->line.transformation.reflectX,
            currentLine->line.transformation.reflectY);

    currentLine = currentLine->next;
  }

  PolygonNode *currentPolygon = polygonList.head;
  while (currentPolygon != NULL) {
    fprintf(file, "polygon %f %f %f %d ", currentPolygon->polygon.color[0],
            currentPolygon->polygon.color[1], currentPolygon->polygon.color[2],
            currentPolygon->polygon.vertexCount);

    for (int i = 0; i < currentPolygon->polygon.vertexCount; i++) {
      float normalizedX = currentPolygon->polygon.vertices[i][0] / 400.0f;
      float normalizedY = currentPolygon->polygon.vertices[i][1] / 300.0f;

      fprintf(file, "%f %f ", normalizedX, normalizedY);
    }

    for (int i = 0; i < 16; i++) {
      fprintf(file, "%f ", currentPolygon->polygon.transformation.matrix[i]);
    }

    fprintf(file, "%f %f %f %f %f %f %d %d\n", currentPolygon->polygon.transformation.tx,
            currentPolygon->polygon.transformation.ty, currentPolygon->polygon.transformation.angle,
            currentPolygon->polygon.transformation.scale,
            currentPolygon->polygon.transformation.shearX,
            currentPolygon->polygon.transformation.shearY,
            currentPolygon->polygon.transformation.reflectX,
            currentPolygon->polygon.transformation.reflectY);

    currentPolygon = currentPolygon->next;
  }

  fclose(file);
  printf("Projeto salvo com sucesso.\n");
}

// Função auxiliar para carregar a transformação
void loadTransformation(Transformation *transformation, char *line) {
  sscanf(line, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d %d",
         &transformation->matrix[0], &transformation->matrix[1], &transformation->matrix[2],
         &transformation->matrix[3], &transformation->matrix[4], &transformation->matrix[5],
         &transformation->matrix[6], &transformation->matrix[7], &transformation->matrix[8],
         &transformation->matrix[9], &transformation->matrix[10], &transformation->matrix[11],
         &transformation->matrix[12], &transformation->matrix[13], &transformation->matrix[14],
         &transformation->matrix[15], &transformation->tx, &transformation->ty,
         &transformation->angle, &transformation->scale, &transformation->shearX,
         &transformation->shearY, &transformation->reflectX, &transformation->reflectY);
}

void recalculateAllTransformations() {
  PointNode *pointNode = pointList.head;
  while (pointNode != NULL) {
    updatePointTransformationMatrix(pointNode);
    pointNode = pointNode->next;
  }

  LineNode *lineNode = lineList.head;
  while (lineNode != NULL) {
    updateLineTransformationMatrix(lineNode);
    lineNode = lineNode->next;
  }

  PolygonNode *polygonNode = polygonList.head;
  while (polygonNode != NULL) {
    updatePolygonTransformationMatrix(polygonNode);
    polygonNode = polygonNode->next;
  }
}

void resetProjection() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(-400.0f, 400.0f, -300.0f, 300.0f);
  glMatrixMode(GL_MODELVIEW);
}

void loadFromFile(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    printf("Erro: Não foi possível carregar o arquivo %s.\n", filename);
    return;
  }

  char line[512];
  while (fgets(line, sizeof(line), file)) {
    if (strncmp(line, "point", 5) == 0) {
      PointNode *newPointNode = (PointNode *)malloc(sizeof(PointNode));
      int        offset;
      float      x_normalized, y_normalized;

      sscanf(line, "point %f %f %f %f %f %f%n", &x_normalized, &y_normalized,
             &newPointNode->point.color[0], &newPointNode->point.color[1],
             &newPointNode->point.color[2], &newPointNode->point.size, &offset);

      newPointNode->point.x = x_normalized * 400.0f;
      newPointNode->point.y = y_normalized * 300.0f;
      // Carregar a transformação usando o offset retornado pelo sscanf
      loadTransformation(&newPointNode->point.transformation, line + offset);

      newPointNode->prev = NULL;
      addPointNode(newPointNode);

    } else if (strncmp(line, "line", 4) == 0) {
      LineNode *newLineNode = (LineNode *)malloc(sizeof(LineNode));
      int       offset;
      float     xn0, yn0, xn1, yn1;

      sscanf(line, "line %f %f %f %f %f %f %f%n", &xn0, &yn0, &xn1, &yn1,
             &newLineNode->line.color[0], &newLineNode->line.color[1], &newLineNode->line.color[2],
             &offset);

      newLineNode->line.x0 = xn0 * 400.0f;
      newLineNode->line.y0 = yn0 * 300.0f;
      newLineNode->line.x1 = xn1 * 400.0f;
      newLineNode->line.y1 = yn1 * 300.0f;
      // Carregar a transformação
      loadTransformation(&newLineNode->line.transformation, line + offset);

      newLineNode->prev = NULL;
      addLineNode(newLineNode);

    } else if (strncmp(line, "polygon", 7) == 0) {
      PolygonNode *newPolygonNode = (PolygonNode *)malloc(sizeof(PolygonNode));
      int          vertexCount;
      int          offset = 0;  // Número de caracteres lidos

      sscanf(line, "polygon %f %f %f %d%n", &newPolygonNode->polygon.color[0],
             &newPolygonNode->polygon.color[1], &newPolygonNode->polygon.color[2], &vertexCount,
             &offset);
      newPolygonNode->polygon.vertexCount = vertexCount;

      // Ler coordenadas de vértices
      char *vertexData = line + offset;  // Pula a parte inicial para pegar os vértices
      for (int i = 0; i < vertexCount; i++) {
        float x_normalized, y_normalized;
        int   charsRead = 0;

        sscanf(vertexData, "%f %f%n", &x_normalized, &y_normalized, &charsRead);

        // Desnormalizar as coordenadas para o sistema de coordenadas da janela
        newPolygonNode->polygon.vertices[i][0] = x_normalized * 400.0f;  // Aplicar fator de escala
        newPolygonNode->polygon.vertices[i][1] = y_normalized * 300.0f;  // Aplicar fator de escala

        // Avançar no vertexData com base no número de caracteres lidos
        vertexData += charsRead;
      }

      // Carregar a transformação
      vertexData = strchr(vertexData, ' ') + 1;
      loadTransformation(&newPolygonNode->polygon.transformation, vertexData);

      newPolygonNode->prev = NULL;
      addPolygonNode(newPolygonNode);
    }
  }

  fclose(file);
  resetProjection();
  recalculateAllTransformations();
  glutPostRedisplay();
}

void printInitialInfo() {
  printf("\nGuache - 2D Painter\n");
  printf("\n");
  printf("%s%s  1  %s", BLKB, UWHT, CRESET);
  printf("%s%s  2  %s", REDB, UWHT, CRESET);
  printf("%s%s  3  %s", GRNB, UWHT, CRESET);
  printf("%s%s  4  %s", BLUB, UWHT, CRESET);
  printf("%s%s  5  %s", YELB, UWHT, CRESET);
  printf("%s%s  6  %s", MAGB, UWHT, CRESET);
  printf("%s%s  7  %s", CYNB, UWHT, CRESET);
  printf("\n\n");

  printf("v      -> Ponto\n");
  printf("b      -> Linha\n");
  printf("n      -> Polígono\n");
  printf("m      -> Seleção\n");
  printf("0      -> Remover objeto selecionado\n");
  printf("\n");

  printf("  w  \n");
  printf("a s d  -> Translação\n");
  printf("q e    -> Rotatação\n");
  printf("- +    -> Escala\n");
  printf("  i  \n");
  printf("j k l  -> Cisalhamento\n");
  printf("x y    -> Reflexão\n");
  printf("\n");

  printf("z      -> Rotacionar polígono selecionado\n");
  printf("\n");

  printf(".      -> Salvar em %s\n", saveFile);
  printf("\n");
}

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Guache - 2D Painter");

  if (argc > 1) {
    if (strcmp(argv[1], "save") == 0 && argc == 3) {
      strncpy(saveFile, argv[2], sizeof(saveFile));
      saveFile[sizeof(saveFile) - 1] = '\0';
      shouldSave                     = 1;
    } else if (strcmp(argv[1], "load") == 0 && argc == 3) {
      strncpy(saveFile, argv[2], sizeof(saveFile));
      saveFile[sizeof(saveFile) - 1] = '\0';
      shouldSave                     = 1;
      shouldLoad                     = 1;
    } else {
      printf("Uso:\n");
      printf("  %s save projeto.guache  (para salvar objetos)\n", argv[0]);
      printf("  %s load projeto.guache  (para carregar objetos)\n", argv[0]);
      return 1;
    }
  } else {
    strncpy(saveFile, "projeto.guache", sizeof(saveFile));
    saveFile[sizeof(saveFile) - 1] = '\0';
    shouldSave                     = 1;
  }

  init();

  printInitialInfo();
  glutDisplayFunc(display);

  glutKeyboardFunc(keyPress);
  glutMouseFunc(onMouseClick);
  glutPassiveMotionFunc(mouseMoveCallback);
  glutMotionFunc(mouseDragCallback);
  glutTimerFunc(0, updateAnimation, 0);
  glutMainLoop();

  return EXIT_SUCCESS;
}
