#include <GL/freeglut.h>
#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_VERTICES 10

int windowWidth = 800;
int windowHeight = 600;

char saveFile[256] = ""; // Nome do arquivo para salvar
int shouldSave = 0; // Flag que indica se o programa vai salvar em um arquivo

typedef enum { DRAW_POINT, DRAW_LINE, DRAW_POLYGON, SELECT } Mode;
Mode currentMode = DRAW_POINT;

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

// Usado na criação de uma linha
float tempLineX0, tempLineY0;
float currentMouseX, currentMouseY;
int isDrawingLine =
    0; // Flag para marcar se estamos no processo de desenhar uma linha

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

void addPointNode(PointNode *newNode) {
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

void addLineNode(LineNode *newNode) {
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

void addPolygonNode(PolygonNode *newNode) {
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
    glColor3f(current->line.color[0], current->line.color[1],
              current->line.color[2]);

    glVertex2f(current->line.x0, current->line.y0);
    glVertex2f(current->line.x1, current->line.y1);

    current = current->next;
  }
  glEnd();
}

void renderAllPolygons() {
  PolygonNode *current = polygonList.head;

  while (current != NULL) {
    glColor3f(current->polygon.color[0], current->polygon.color[1],
              current->polygon.color[2]);

    glBegin(GL_POLYGON);

    for (int i = 0; i < current->polygon.vertexCount; i++) {
      glVertex2f(current->polygon.vertices[i][0],
                 current->polygon.vertices[i][1]);
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
    float worldY = (windowHeight - y); // y começa do outro lado

    if (currentMode == DRAW_POINT) {
      addPoint(worldX, worldY, 0.0f, 0.0f, 0.0f, 5.0f);
    } else if (currentMode == DRAW_LINE) {
      if (!isDrawingLine) {
        tempLineX0 = worldX;
        tempLineY0 = worldY;
        currentMouseX = worldX;
        currentMouseY = worldY;
        isDrawingLine = 1;
      } else {
        // Se chegou aqui está no segundo clique do desenho de linha
        addLine(tempLineX0, tempLineY0, worldX, worldY, 1.0f, 0.0f, 0.0f);
        isDrawingLine = 0;
      }
    }
    // Redesenhar a janela
    glutPostRedisplay();
  }
}

void mouseMoveCallback(int x, int y) {
  if (isDrawingLine) {
    currentMouseX = (float)x;
    currentMouseY = (float)(windowHeight - y); // y começa do outro lado
    glutPostRedisplay(); // Redesenha a cena para preview da linha
  }
}

void saveToFile(const char *filename);

void keyPress(unsigned char key, int x, int y) {
  if (key == 's' && shouldSave) {
    if (strlen(saveFile) > 0) {
      saveToFile(saveFile);
      printf("Salvo em %s\n", saveFile);
    } else {
      printf("Nenhum arquivo especificado. Use --save para especificar um "
             "arquivo.\n");
    }
  } else if (key == '1') {
    currentMode = DRAW_POINT;
    printf("Modo de desenho de ponto selecionado.\n");
  } else if (key == '2') {
    currentMode = DRAW_LINE;
    printf("Modo de desenho de linha selecionado.\n");
  }
}

void saveToFile(const char *filename) {
  FILE *file = fopen(filename, "w");
  if (file == NULL) {
    printf("Erro: Não foi possível abrir o arquivo para salvar.\n");
    return;
  }

  PointNode *currentPoint = pointList.head;
  while (currentPoint != NULL) {
    fprintf(file, "point %f %f %f %f %f %f\n", currentPoint->point.x,
            currentPoint->point.y, currentPoint->point.color[0],
            currentPoint->point.color[1], currentPoint->point.color[2],
            currentPoint->point.size);

    currentPoint = currentPoint->next;
  }

  LineNode *currentLine = lineList.head;
  while (currentLine != NULL) {
    fprintf(file, "line %f %f %f %f %f %f %f\n", currentLine->line.x0,
            currentLine->line.y0, currentLine->line.x1, currentLine->line.y1,
            currentLine->line.color[0], currentLine->line.color[1],
            currentLine->line.color[2]);

    currentLine = currentLine->next;
  }

  PolygonNode *currentPolygon = polygonList.head;
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

void loadFromFile(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    printf("Erro: Não foi possível carregar o arquivo %s.\n", filename);
    return;
  }

  char line[256];
  while (fgets(line, sizeof(line), file)) {
    if (strncmp(line, "point", 5) == 0) {
      PointNode *newPointNode = (PointNode *)malloc(sizeof(PointNode));
      sscanf(line, "point %f %f %f %f %f %f", &newPointNode->point.x,
             &newPointNode->point.y, &newPointNode->point.color[0],
             &newPointNode->point.color[1], &newPointNode->point.color[2],
             &newPointNode->point.size);
      addPointNode(newPointNode);
    } else if (strncmp(line, "line", 4) == 0) {
      LineNode *newLineNode = (LineNode *)malloc(sizeof(LineNode));
      sscanf(line, "line %f %f %f %f %f %f %f", &newLineNode->line.x0,
             &newLineNode->line.y0, &newLineNode->line.x1,
             &newLineNode->line.y1, &newLineNode->line.color[0],
             &newLineNode->line.color[1], &newLineNode->line.color[2]);
      addLineNode(newLineNode);
    } else if (strncmp(line, "polygon", 7) == 0) {
      PolygonNode *newPolygonNode = (PolygonNode *)malloc(sizeof(PolygonNode));
      int vertexCount;
      int offset = 0; // Número de caracters lido
      sscanf(line, "polygon %f %f %f %d%n", &newPolygonNode->polygon.color[0],
             &newPolygonNode->polygon.color[1],
             &newPolygonNode->polygon.color[2], &vertexCount, &offset);
      newPolygonNode->polygon.vertexCount = vertexCount;

      // Ler coordenadas de vértices
      char *vertexData =
          line + offset; // Pula a parte inicial para pegar os vértices

      for (int i = 0; i < vertexCount; i++) {
        sscanf(vertexData, "%f %f", &newPolygonNode->polygon.vertices[i][0],
               &newPolygonNode->polygon.vertices[i][1]);

        // strchr retorna a primeira ocorrência de um ' ' depois de vertexData
        vertexData = strchr(vertexData, ' ') + 1; // Move para o próximo vértice
        vertexData = strchr(vertexData, ' ') + 1;
      }
      addPolygonNode(newPolygonNode);
    }
  }

  fclose(file);
}

int main(int argc, char *argv[]) {
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

  // Opções para antialiasing
  // glEnable(GL_MULTISAMPLE);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glEnable(GL_LINE_SMOOTH);

  init();
  glutDisplayFunc(display);
  glutKeyboardFunc(keyPress);
  glutMouseFunc(onMouseClick);
  glutPassiveMotionFunc(mouseMoveCallback);
  glutMainLoop();

  return EXIT_SUCCESS;
}
