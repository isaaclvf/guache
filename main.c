#include <GL/freeglut.h>
#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VERTICES 10

int windowWidth = 800;
int windowHeight = 600;
float tolerance = 5.0;

char saveFile[256] = "";  // Nome do arquivo para salvar
int shouldSave = 0;  // Flag que indica se o programa vai salvar em um arquivo

typedef enum { DRAW_POINT, DRAW_LINE, DRAW_POLYGON, SELECT } Mode;
Mode currentMode = DRAW_POINT;

float currentColor[] = {0.0f, 0.0f, 0.0f};

typedef struct {
  float matrix[16];
  float tx, ty;
  float angle;
  float scale;
  float shearX, shearY;
  int reflectX;
  int reflectY;
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
  int vertexCount;
  float vertices[MAX_VERTICES][2];  // Coordenadas dos vertices
  float color[3];                   // RGB

  Transformation transformation;
} Polygon;

// Usado na criação de objetos
float tempLineX0, tempLineY0;
float currentMouseX, currentMouseY;
Polygon tempPolygon = {
    .color = {0.0f, 0.0f, 1.0f},
    .vertexCount = 0,
};

// Flags para marcar se estamos no processo de desenhar
int isDrawingLine = 0;
int isDrawingPolygon = 0;

typedef struct PointNode {
  Point point;
  struct PointNode* next;
  struct PointNode* prev;
} PointNode;

typedef struct LineNode {
  Line line;
  struct LineNode* next;
  struct LineNode* prev;
} LineNode;

typedef struct PolygonNode {
  Polygon polygon;
  struct PolygonNode* next;
  struct PolygonNode* prev;
} PolygonNode;

typedef struct PointList {
  PointNode* head;
} PointList;

typedef struct LineList {
  LineNode* head;
} LineList;

typedef struct PolygonList {
  PolygonNode* head;
} PolygonList;

PointList pointList = {NULL};
LineList lineList = {NULL};
PolygonList polygonList = {NULL};

PointNode* selectedPoint = {NULL};
LineNode* selectedLine = {NULL};
PolygonNode* selectedPolygon = {NULL};
int isAnythingSelected = 0;

void clearSelection() {
  if (selectedPoint != NULL) {
    for (int i = 0; i < 2; i++) {
      selectedPoint->point.color[i] = 0.0;
    }
  }

  if (selectedLine != NULL) {
    for (int i = 0; i < 2; i++) {
      selectedLine->line.color[i] = 0.0;
    }
  }

  if (selectedPolygon != NULL) {
    for (int i = 0; i < 2; i++) {
      selectedPolygon->polygon.color[i] = 0.0;
    }
  }

  isAnythingSelected = 0;
  selectedPoint = NULL;
  selectedLine = NULL;
  selectedPolygon = NULL;
}

void addPoint(float x, float y, float red, float green, float blue,
              float size) {
  PointNode* newNode = (PointNode*)malloc(sizeof(PointNode));

  newNode->point.x = x;
  newNode->point.y = y;
  newNode->point.color[0] = red;
  newNode->point.color[1] = green;
  newNode->point.color[2] = blue;
  newNode->point.size = size;
  newNode->prev = NULL;

  newNode->point.transformation.tx = 0.0f;
  newNode->point.transformation.ty = 0.0f;
  newNode->point.transformation.angle = 0.0f;
  newNode->point.transformation.scale = 1.0f;
  newNode->point.transformation.shearX = 0.0f;
  newNode->point.transformation.shearY = 0.0f;
  newNode->point.transformation.reflectX = 0;
  newNode->point.transformation.reflectY = 0;

  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetFloatv(GL_MODELVIEW_MATRIX, newNode->point.transformation.matrix);
  glPopMatrix();

  PointNode* oldHead = pointList.head;

  if (oldHead) {
    oldHead->prev = newNode;
  }

  newNode->next = oldHead;
  pointList.head = newNode;
}

void addPointNode(PointNode* newNode) {
  PointNode* oldHead = pointList.head;

  if (oldHead) {
    oldHead->prev = newNode;
  }

  newNode->next = oldHead;
  pointList.head = newNode;
}

void addLine(float x0, float y0, float x1, float y1, float red, float green,
             float blue) {
  LineNode* newNode = (LineNode*)malloc(sizeof(LineNode));

  newNode->line.x0 = x0;
  newNode->line.y0 = y0;
  newNode->line.x1 = x1;
  newNode->line.y1 = y1;
  newNode->line.color[0] = red;
  newNode->line.color[1] = green;
  newNode->line.color[2] = blue;
  newNode->prev = NULL;

  newNode->line.transformation.tx = 0.0f;
  newNode->line.transformation.ty = 0.0f;
  newNode->line.transformation.angle = 0.0f;
  newNode->line.transformation.scale = 1.0f;
  newNode->line.transformation.shearX = 0.0f;
  newNode->line.transformation.shearY = 0.0f;
  newNode->line.transformation.reflectX = 0;
  newNode->line.transformation.reflectY = 0;

  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetFloatv(GL_MODELVIEW_MATRIX, newNode->line.transformation.matrix);
  glPopMatrix();

  LineNode* oldHead = lineList.head;

  if (oldHead) {
    oldHead->prev = newNode;
  }

  newNode->next = oldHead;
  lineList.head = newNode;
}

void addLineNode(LineNode* newNode) {
  LineNode* oldHead = lineList.head;

  if (oldHead) {
    oldHead->prev = newNode;
  }

  newNode->next = oldHead;
  lineList.head = newNode;
}

void addPolygon(float vertices[][2], int vertexCount, float red, float green,
                float blue) {
  if (vertexCount < 3) {
    printf("Erro: Um polígono precisa ter pelo menos 3 vertices.\n");
    return;
  }

  PolygonNode* newNode = (PolygonNode*)malloc(sizeof(PolygonNode));

  newNode->polygon.vertexCount = vertexCount;
  for (int i = 0; i < vertexCount; i++) {
    newNode->polygon.vertices[i][0] = vertices[i][0];
    newNode->polygon.vertices[i][1] = vertices[i][1];
  }

  newNode->polygon.color[0] = red;
  newNode->polygon.color[1] = green;
  newNode->polygon.color[2] = blue;
  newNode->prev = NULL;

  newNode->polygon.transformation.tx = 0.0f;
  newNode->polygon.transformation.ty = 0.0f;
  newNode->polygon.transformation.angle = 0.0f;
  newNode->polygon.transformation.scale = 1.0f;
  newNode->polygon.transformation.shearX = 0.0f;
  newNode->polygon.transformation.shearY = 0.0f;
  newNode->polygon.transformation.reflectX = 0;
  newNode->polygon.transformation.reflectY = 0;

  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetFloatv(GL_MODELVIEW_MATRIX, newNode->polygon.transformation.matrix);
  glPopMatrix();

  PolygonNode* oldHead = polygonList.head;

  if (oldHead) {
    oldHead->prev = newNode;
  }

  newNode->next = oldHead;
  polygonList.head = newNode;
}

void addPolygonNode(PolygonNode* newNode) {
  PolygonNode* oldHead = polygonList.head;

  if (oldHead) {
    oldHead->prev = newNode;
  }

  newNode->next = oldHead;
  polygonList.head = newNode;
}

void updatePointTransformationMatrix(PointNode* pointNode) {
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
    GLfloat reflectXMatrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
                                  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 1.0f};
    glMultMatrixf(reflectXMatrix);
  }

  if (pointNode->point.transformation.reflectY) {
    GLfloat reflectYMatrix[16] = {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                  0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                  0.0f,  0.0f, 0.0f, 1.0f};
    glMultMatrixf(reflectYMatrix);
  }

  glRotatef(pointNode->point.transformation.angle, 0.0, 0.0, 1.0);
  glTranslatef(pointNode->point.transformation.tx,
               pointNode->point.transformation.ty, 0.0f);

  // Save the updated transformation matrix
  glGetFloatv(GL_MODELVIEW_MATRIX, pointNode->point.transformation.matrix);
}

void updateLineTransformationMatrix(LineNode* lineNode) {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(lineNode->line.transformation.tx,
               lineNode->line.transformation.ty, 0.0f);

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
    GLfloat reflectXMatrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
                                  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 1.0f};
    glMultMatrixf(reflectXMatrix);
  }

  if (lineNode->line.transformation.reflectY) {
    GLfloat reflectYMatrix[16] = {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                  0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                  0.0f,  0.0f, 0.0f, 1.0f};
    glMultMatrixf(reflectYMatrix);
  }

  float centerX = (lineNode->line.x0 + lineNode->line.x1) / 2.0f;
  float centerY = (lineNode->line.y0 + lineNode->line.y1) / 2.0f;

  glTranslatef(centerX, centerY, 0.0f);
  glScalef(lineNode->line.transformation.scale,
           lineNode->line.transformation.scale, 0.0f);
  glRotatef(lineNode->line.transformation.angle, 0.0, 0.0, 1.0);
  glTranslatef(-centerX, -centerY, 0.0f);

  // Save the updated transformation matrix
  glGetFloatv(GL_MODELVIEW_MATRIX, lineNode->line.transformation.matrix);
}

void updatePolygonTransformationMatrix(PolygonNode* polygonNode) {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(polygonNode->polygon.transformation.tx,
               polygonNode->polygon.transformation.ty, 0.0f);

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
    GLfloat reflectXMatrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
                                  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 1.0f};
    glMultMatrixf(reflectXMatrix);
  }

  if (polygonNode->polygon.transformation.reflectY) {
    GLfloat reflectYMatrix[16] = {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                  0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                  0.0f,  0.0f, 0.0f, 1.0f};
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
  glScalef(polygonNode->polygon.transformation.scale,
           polygonNode->polygon.transformation.scale, 0.0f);
  glRotatef(polygonNode->polygon.transformation.angle, 0.0, 0.0, 1.0);
  glTranslatef(-centerX, -centerY, 0.0f);

  // Save the updated transformation matrix
  glGetFloatv(GL_MODELVIEW_MATRIX, polygonNode->polygon.transformation.matrix);
}

void removePointNode(PointNode* node) {
  PointNode* prevNode = node->prev;
  PointNode* nextNode = node->next;

  if (prevNode) {
    prevNode->next = node->next;
  }

  if (nextNode) {
    nextNode->prev = node->prev;
  }

  free(node);
  selectedPoint = NULL;
}

void removeLineNode(LineNode* node) {
  LineNode* prevNode = node->prev;
  LineNode* nextNode = node->next;

  if (prevNode) {
    prevNode->next = node->next;
  }

  if (nextNode) {
    nextNode->prev = node->prev;
  }

  free(node);
  selectedLine = NULL;
}

void removePolygonNode(PolygonNode* node) {
  PolygonNode* prevNode = node->prev;
  PolygonNode* nextNode = node->next;

  if (prevNode) {
    prevNode->next = node->next;
  }

  if (nextNode) {
    nextNode->prev = node->prev;
  }

  free(node);
  selectedPolygon = NULL;
}

void renderAllPoints() {
  PointNode* current = pointList.head;

  while (current != NULL) {
    glPointSize(5.0f);
    glLoadMatrixf(current->point.transformation.matrix);

    glBegin(GL_POINTS);

    glColor3f(current->point.color[0], current->point.color[1],
              current->point.color[2]);

    glVertex2f(current->point.x, current->point.y);
    glEnd();

    current = current->next;
  }
}

void renderAllLines() {
  LineNode* current = lineList.head;

  while (current != NULL) {
    glLoadMatrixf(current->line.transformation.matrix);
    glBegin(GL_LINES);

    glColor3f(current->line.color[0], current->line.color[1],
              current->line.color[2]);

    glVertex2f(current->line.x0, current->line.y0);
    glVertex2f(current->line.x1, current->line.y1);

    glEnd();
    current = current->next;
  }
}

void renderAllPolygons() {
  PolygonNode* current = polygonList.head;

  while (current != NULL) {
    glColor3f(current->polygon.color[0], current->polygon.color[1],
              current->polygon.color[2]);

    glLoadMatrixf(current->polygon.transformation.matrix);
    glBegin(GL_POLYGON);

    for (int i = 0; i < current->polygon.vertexCount; i++) {
      glVertex2f(current->polygon.vertices[i][0],
                 current->polygon.vertices[i][1]);
    }

    glEnd();
    current = current->next;
  }

  if (isDrawingPolygon && tempPolygon.vertexCount > 0) {
    glColor3f(tempPolygon.color[0], tempPolygon.color[1], tempPolygon.color[2]);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < tempPolygon.vertexCount; i++) {
      glVertex2f(tempPolygon.vertices[i][0], tempPolygon.vertices[i][1]);
    }
    glEnd();
  }
}

void freePointList() {
  PointNode* current = pointList.head;
  while (current != NULL) {
    PointNode* next = current->next;
    free(current);
    current = next;
  }
}

void freeLineList() {
  LineNode* current = lineList.head;
  while (current != NULL) {
    LineNode* next = current->next;
    free(current);
    current = next;
  }
}

void freePolygonList() {
  PolygonNode* current = polygonList.head;
  while (current != NULL) {
    PolygonNode* next = current->next;
    free(current);
    current = next;
  }
}

// selection algorithms

PointNode* selectPoint(int sx, int sy) {
  PointNode* current = pointList.head;

  while (current != NULL) {
    if ((current->point.x <= sx + tolerance) &&
        (current->point.x >= sx - tolerance) &&
        (current->point.y <= sy + tolerance) &&
        (current->point.y >= sy - tolerance)) {
      current->point.color[0] = 1.0;
      current->point.color[1] = 0.0;
      current->point.color[2] = 0.0;
      isAnythingSelected = 1;
      return current;
    }
    current = current->next;
  }
  return NULL;
}

int auxSelectLine(float x, float y, float X, float Y) {
  int xmax = x + tolerance, xmin = x - tolerance, ymax = y + tolerance,
      ymin = y - tolerance;

  if ((X <= xmax) && (X >= xmin) && (Y <= ymax) && (Y >= ymin)) {
    return 1;
  }

  return 0;
}

LineNode* selectLine(int sx, int sy) {
  LineNode* current = lineList.head;

  int xmax = sx + tolerance, xmin = sx - tolerance, ymax = sy + tolerance,
      ymin = sy - tolerance;
  int select = 0;
  float tempVertice;

  while (current != NULL) {
    if (auxSelectLine(sx, sy, current->line.x0, current->line.y0) ||
        auxSelectLine(sx, sy, current->line.x1, current->line.y1)) {
      select = 1;
    } else {
      //[TO DO] conferir para x0 E x1, y0 E y1!!!
      if (current->line.x0 < xmin) {
        tempVertice =
            current->line.y0 +
            ((xmin - current->line.x0) * (current->line.y1 - current->line.y0) /
             (current->line.x1 - current->line.x0));
        if (auxSelectLine(sx, sy, xmin, tempVertice)) {
          select = 1;
        }
      } else if (current->line.x0 > xmax) {
        tempVertice =
            current->line.y0 +
            ((xmax - current->line.x0) * (current->line.y1 - current->line.y0) /
             (current->line.x1 - current->line.x0));
        if (auxSelectLine(sx, sy, xmax, tempVertice)) {
          select = 1;
        }
      } else if (current->line.y0 < ymin) {
        tempVertice =
            current->line.x0 +
            ((ymin - current->line.y0) * (current->line.x1 - current->line.x0) /
             (current->line.y1 - current->line.y0));
        if (auxSelectLine(sx, sy, tempVertice, ymin)) {
          select = 1;
        }
      } else if (current->line.y0 > ymax) {
        tempVertice =
            current->line.x0 +
            ((ymax - current->line.y0) * (current->line.x1 - current->line.x0) /
             (current->line.y1 - current->line.y0));
        if (auxSelectLine(sx, sy, tempVertice, ymax)) {
          select = 1;
        }
      } else if (current->line.x1 < xmin) {
        tempVertice =
            current->line.y1 +
            ((xmin - current->line.x1) * (current->line.y1 - current->line.y0) /
             (current->line.x1 - current->line.x0));
        if (auxSelectLine(sx, sy, xmin, tempVertice)) {
          select = 1;
        }
      } else if (current->line.x1 > xmax) {
        tempVertice =
            current->line.y1 +
            ((xmax - current->line.x1) * (current->line.y1 - current->line.y0) /
             (current->line.x1 - current->line.x0));
        if (auxSelectLine(sx, sy, xmax, tempVertice)) {
          select = 1;
        }
      } else if (current->line.y1 < ymin) {
        tempVertice =
            current->line.x1 +
            ((ymin - current->line.y1) * (current->line.x1 - current->line.x0) /
             (current->line.y1 - current->line.y0));
        if (auxSelectLine(sx, sy, tempVertice, ymin)) {
          select = 1;
        }
      } else if (current->line.y1 > ymax) {
        tempVertice =
            current->line.x1 +
            ((ymax - current->line.y1) * (current->line.x1 - current->line.x0) /
             (current->line.y1 - current->line.y0));
        if (auxSelectLine(sx, sy, tempVertice, ymax)) {
          select = 1;
        }
      }
    }
    if (select) {
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

PolygonNode* selectPolygon(int sx, int sy) {
  PolygonNode* current = polygonList.head;
  float x1, y1, x0, y0;
  int walls;

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

      printf("verificando reta (%.0f,%.0f)->(%.0f,%.0f)", x0, y0, x1, y1);
      if (((x0 > sx) && (x1 > sx)) &&
          (((y0 > sy) && (y1 < sy)) || ((y1 > sy) && (y0 < sy)))) {
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
      current->polygon.color[0] = 1.0;
      current->polygon.color[1] = 0.0;
      current->polygon.color[2] = 0.0;
      isAnythingSelected = 1;
      return current;
    }

    current = current->next;
  }

  return NULL;
}

void init() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  glMatrixMode(GL_PROJECTION);
  gluOrtho2D(-400.0f, 400.0f, -300.0f, 300.0f);
}

void display() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  renderAllPoints();
  renderAllLines();
  renderAllPolygons();

  // Se estiver no processo de desenhar uma linha, mostrar um preview
  if (isDrawingLine) {
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_LINES);
    glVertex2f(tempLineX0, tempLineY0);
    glVertex2f(currentMouseX, currentMouseY);
    glEnd();
  }

  glFlush();
  // glutSwapBuffers();
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
    float worldX = (float)x - (windowWidth / 2);
    float worldY = (float)-y + (windowHeight / 2);

    if (currentMode == DRAW_POINT) {
      addPoint(worldX, worldY, currentColor[0], currentColor[1],
               currentColor[2], 5.0f);
    } else if (currentMode == DRAW_LINE) {
      if (!isDrawingLine) {
        tempLineX0 = worldX;
        tempLineY0 = worldY;
        currentMouseX = worldX;
        currentMouseY = worldY;
        isDrawingLine = 1;
      } else {
        // Se chegou aqui está no segundo clique do desenho de linha
        addLine(tempLineX0, tempLineY0, worldX, worldY, currentColor[0],
                currentColor[1], currentColor[2]);
        isDrawingLine = 0;
      }
    } else if (currentMode == DRAW_POLYGON) {
      if (!isDrawingPolygon) {
        isDrawingPolygon = 1;
        tempPolygon.vertexCount = 0;
      }

      if (tempPolygon.vertexCount < MAX_VERTICES) {
        if (isCloseToFirstPoint(worldX, worldY) &&
            tempPolygon.vertexCount > 2) {
          addPolygon(tempPolygon.vertices, tempPolygon.vertexCount,
                     currentColor[0], currentColor[1], currentColor[2]);
          isDrawingPolygon = 0;
        } else {
          // Adiciona vértice ao polígono
          tempPolygon.vertices[tempPolygon.vertexCount][0] = worldX;
          tempPolygon.vertices[tempPolygon.vertexCount][1] = worldY;
          tempPolygon.vertexCount++;
        }
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
    // Redesenhar a janela
    glutPostRedisplay();
  }
}

void mouseMoveCallback(int x, int y) {
  if (isDrawingLine) {
    currentMouseX = (float)x - (windowWidth / 2);
    currentMouseY = (float)-y + (windowHeight / 2);
    glutPostRedisplay();  // Redesenha a cena para preview da linha
  }
}

void saveToFile(const char* filename);

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
    printf("Modo de desenho de \e[1;32mponto\e[0m selecionado.\n");

  } else if (key == 'b') {
    clearSelection();
    currentMode = DRAW_LINE;
    printf("Modo de desenho de \e[1;32mlinha\e[0m selecionado.\n");

  } else if (key == 'n') {
    clearSelection();
    currentMode = DRAW_POLYGON;
    printf("Modo de desenho de \e[1;32mpolígono\e[0m selecionado.\n");

  } else if (key == 'm') {
    currentMode = SELECT;
    printf("Modo de \e[1;32mseleção\e[0m ativo.\n");

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
  }

  switch (key) {
    case '1':
      currentColor[0] = 0.0f;
      currentColor[1] = 0.0f;
      currentColor[2] = 0.0f;
      break;
    case '2':
      currentColor[0] = 1.0f;
      currentColor[1] = 0.0f;
      currentColor[2] = 0.0f;
      break;
    case '3':
      currentColor[0] = 0.0f;
      currentColor[1] = 1.0f;
      currentColor[2] = 0.0f;
      break;
    case '4':
      currentColor[0] = 0.0f;
      currentColor[1] = 0.0f;
      currentColor[2] = 1.0f;
      break;
    case '5':
      currentColor[0] = 1.0f;
      currentColor[1] = 1.0f;
      currentColor[2] = 0.0f;
      break;
    case '6':
      currentColor[0] = 1.0f;
      currentColor[1] = 0.0f;
      currentColor[2] = 1.0f;
      break;
    case '7':
      currentColor[0] = 0.0f;
      currentColor[1] = 1.0f;
      currentColor[2] = 1.0f;
      break;
  }

  if (selectedPoint) {
    switch (key) {
      case 'q':
        selectedPoint->point.transformation.angle += 5.0f;
        break;
      case 'e':
        selectedPoint->point.transformation.angle -= 5.0f;
        break;
      case 'w':
        selectedPoint->point.transformation.ty += 5.0f;
        break;
      case 's':
        selectedPoint->point.transformation.ty -= 5.0f;
        break;
      case 'a':
        selectedPoint->point.transformation.tx -= 5.0f;
        break;
      case 'd':
        selectedPoint->point.transformation.tx += 5.0f;
        break;
      case '+':
        selectedPoint->point.transformation.scale += 0.1f;
        break;
      case '-':
        selectedPoint->point.transformation.scale -= 0.1f;
        break;
      case 'i':
        selectedPoint->point.transformation.shearX += 0.05f;
        break;
      case 'k':
        selectedPoint->point.transformation.shearX -= 0.05f;
        break;
      case 'j':
        selectedPoint->point.transformation.shearY += 0.05f;
        break;
      case 'l':
        selectedPoint->point.transformation.shearY -= 0.05f;
        break;
      case 'x':
        selectedPoint->point.transformation.reflectX =
            !selectedPoint->point.transformation.reflectX;
        break;
      case 'y':
        selectedPoint->point.transformation.reflectY =
            !selectedPoint->point.transformation.reflectY;
        break;
    }

    updatePointTransformationMatrix(selectedPoint);
  }

  if (selectedLine) {
    switch (key) {
      case 'q':
        selectedLine->line.transformation.angle += 5.0f;
        break;
      case 'e':
        selectedLine->line.transformation.angle -= 5.0f;
        break;
      case 'w':
        selectedLine->line.transformation.ty += 5.0f;
        break;
      case 's':
        selectedLine->line.transformation.ty -= 5.0f;
        break;
      case 'a':
        selectedLine->line.transformation.tx -= 5.0f;
        break;
      case 'd':
        selectedLine->line.transformation.tx += 5.0f;
        break;
      case '+':
        selectedLine->line.transformation.scale += 0.1f;
        break;
      case '-':
        selectedLine->line.transformation.scale -= 0.1f;
        break;
      case 'i':
        selectedLine->line.transformation.shearX += 0.05f;
        break;
      case 'k':
        selectedLine->line.transformation.shearX -= 0.05f;
        break;
      case 'j':
        selectedLine->line.transformation.shearY += 0.05f;
        break;
      case 'l':
        selectedLine->line.transformation.shearY -= 0.05f;
        break;
      case 'x':
        selectedLine->line.transformation.reflectX =
            !selectedLine->line.transformation.reflectX;
        break;
      case 'y':
        selectedLine->line.transformation.reflectY =
            !selectedLine->line.transformation.reflectY;
        break;
    }
    updateLineTransformationMatrix(selectedLine);
  }

  if (selectedPolygon) {
    switch (key) {
      case 'q':
        selectedPolygon->polygon.transformation.angle += 5.0f;
        break;
      case 'e':
        selectedPolygon->polygon.transformation.angle -= 5.0f;
        break;
      case 'w':
        selectedPolygon->polygon.transformation.ty += 5.0f;
        break;
      case 's':
        selectedPolygon->polygon.transformation.ty -= 5.0f;
        break;
      case 'a':
        selectedPolygon->polygon.transformation.tx -= 5.0f;
        break;
      case 'd':
        selectedPolygon->polygon.transformation.tx += 5.0f;
        break;
      case '+':
        selectedPolygon->polygon.transformation.scale += 0.1f;
        break;
      case '-':
        selectedPolygon->polygon.transformation.scale -= 0.1f;
        break;
      case 'i':
        selectedPolygon->polygon.transformation.shearX += 0.05f;
        break;
      case 'k':
        selectedPolygon->polygon.transformation.shearX -= 0.05f;
        break;
      case 'j':
        selectedPolygon->polygon.transformation.shearY += 0.05f;
        break;
      case 'l':
        selectedPolygon->polygon.transformation.shearY -= 0.05f;
        break;
      case 'x':
        selectedPolygon->polygon.transformation.reflectX =
            !selectedPolygon->polygon.transformation.reflectX;
        break;
      case 'y':
        selectedPolygon->polygon.transformation.reflectY =
            !selectedPolygon->polygon.transformation.reflectY;
        break;
    }
    updatePolygonTransformationMatrix(selectedPolygon);
  }

  glutPostRedisplay();  // Redesenha a cena para preview da linha
}

void saveToFile(const char* filename) {
  FILE* file = fopen(filename, "w");
  if (file == NULL) {
    printf("Erro: Não foi possível abrir o arquivo para salvar.\n");
    return;
  }

  PointNode* currentPoint = pointList.head;
  while (currentPoint != NULL) {
    fprintf(file, "point %f %f %f %f %f %f\n", currentPoint->point.x,
            currentPoint->point.y, currentPoint->point.color[0],
            currentPoint->point.color[1], currentPoint->point.color[2],
            currentPoint->point.size);

    currentPoint = currentPoint->next;
  }

  LineNode* currentLine = lineList.head;
  while (currentLine != NULL) {
    fprintf(file, "line %f %f %f %f %f %f %f\n", currentLine->line.x0,
            currentLine->line.y0, currentLine->line.x1, currentLine->line.y1,
            currentLine->line.color[0], currentLine->line.color[1],
            currentLine->line.color[2]);

    currentLine = currentLine->next;
  }

  PolygonNode* currentPolygon = polygonList.head;
  while (currentPolygon != NULL) {
    fprintf(file, "polygon %f %f %f %d", currentPolygon->polygon.color[0],
            currentPolygon->polygon.color[1], currentPolygon->polygon.color[2],
            currentPolygon->polygon.vertexCount);

    for (int i = 0; i < currentPolygon->polygon.vertexCount; i++) {
      fprintf(file, " %f %f", currentPolygon->polygon.vertices[i][0],
              currentPolygon->polygon.vertices[i][1]);
    }

    fprintf(file, "\n");

    currentPolygon = currentPolygon->next;
  }

  fclose(file);
  printf("Projeto salvo com sucesso.\n");
}

void loadFromFile(const char* filename) {
  FILE* file = fopen(filename, "r");
  if (!file) {
    printf("Erro: Não foi possível carregar o arquivo %s.\n", filename);
    return;
  }

  char line[256];
  while (fgets(line, sizeof(line), file)) {
    if (strncmp(line, "point", 5) == 0) {
      PointNode* newPointNode = (PointNode*)malloc(sizeof(PointNode));
      sscanf(line, "point %f %f %f %f %f %f", &newPointNode->point.x,
             &newPointNode->point.y, &newPointNode->point.color[0],
             &newPointNode->point.color[1], &newPointNode->point.color[2],
             &newPointNode->point.size);
      addPointNode(newPointNode);
    } else if (strncmp(line, "line", 4) == 0) {
      LineNode* newLineNode = (LineNode*)malloc(sizeof(LineNode));
      sscanf(line, "line %f %f %f %f %f %f %f", &newLineNode->line.x0,
             &newLineNode->line.y0, &newLineNode->line.x1,
             &newLineNode->line.y1, &newLineNode->line.color[0],
             &newLineNode->line.color[1], &newLineNode->line.color[2]);
      addLineNode(newLineNode);
    } else if (strncmp(line, "polygon", 7) == 0) {
      PolygonNode* newPolygonNode = (PolygonNode*)malloc(sizeof(PolygonNode));
      int vertexCount;
      int offset = 0;  // Número de caracters lido
      sscanf(line, "polygon %f %f %f %d%n", &newPolygonNode->polygon.color[0],
             &newPolygonNode->polygon.color[1],
             &newPolygonNode->polygon.color[2], &vertexCount, &offset);
      newPolygonNode->polygon.vertexCount = vertexCount;

      // Ler coordenadas de vértices
      char* vertexData =
          line + offset;  // Pula a parte inicial para pegar os vértices

      for (int i = 0; i < vertexCount; i++) {
        sscanf(vertexData, "%f %f", &newPolygonNode->polygon.vertices[i][0],
               &newPolygonNode->polygon.vertices[i][1]);

        // strchr retorna a primeira ocorrência de um ' ' depois de vertexData
        vertexData =
            strchr(vertexData, ' ') + 1;  // Move para o próximo vértice
        vertexData = strchr(vertexData, ' ') + 1;
      }
      addPolygonNode(newPolygonNode);
    }
  }

  fclose(file);
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    if (strcmp(argv[1], "--save") == 0 && argc == 3) {
      strncpy(saveFile, argv[2], sizeof(saveFile));
      saveFile[sizeof(saveFile) - 1] = '\0';
      shouldSave = 1;
    } else if (strcmp(argv[1], "--load") == 0 && argc == 3) {
      loadFromFile(argv[2]);
    } else {
      printf("Uso:\n");
      printf("  %s --save filename.txt  (para salvar objetos)\n", argv[0]);
      printf("  %s --load filename.txt  (para carregar objetos)\n", argv[0]);
      return 1;
    }
  }

  glutInit(&argc, argv);
  // Mudar para GLUT_DOUBLE depois
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_MULTISAMPLE);
  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Guache - 2D Painter");
  // TODO: Ver glutReshapeFunc();

  init();
  glutDisplayFunc(display);
  glutKeyboardFunc(keyPress);
  glutMouseFunc(onMouseClick);
  glutPassiveMotionFunc(mouseMoveCallback);
  glutMainLoop();

  return EXIT_SUCCESS;
}
