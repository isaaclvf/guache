#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

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
  float ax, ay;
  float bx, by;
  float cx, cy;
  Transformation transformation;
} Triangle;

Triangle t;

void initTriangle() {
  t.ax = -0.5f;
  t.ay = -0.5f;
  t.bx = 0.5f;
  t.by = -0.5f;
  t.cx = 0.0f;
  t.cy = 0.5f;

  t.transformation.tx = 0.0f;
  t.transformation.ty = 0.0f;
  t.transformation.angle = 0.0f;
  t.transformation.scale = 1.0f;
  t.transformation.shearX = 0.0f;
  t.transformation.shearY = 0.0f;
  t.transformation.reflectX = 0;
  t.transformation.reflectY = 0;
}

void updateTransformationMatrix() {
  glLoadIdentity();

  glScalef(t.transformation.scale, t.transformation.scale, 0.0f);
  glTranslatef(t.transformation.tx, t.transformation.ty, 0.0f);

  GLfloat shearMatrix[16] = {1.0f,
                             t.transformation.shearY,
                             0.0f,
                             0.0f,
                             t.transformation.shearX,
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

  if (t.transformation.reflectX) {
    GLfloat reflectXMatrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
                                  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                  0.0f, 0.0f, 0.0f, 1.0f};
    glMultMatrixf(reflectXMatrix);
  }

  if (t.transformation.reflectY) {
    GLfloat reflectYMatrix[16] = {-1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                  0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                  0.0f,  0.0f, 0.0f, 1.0f};
    glMultMatrixf(reflectYMatrix);
  }

  float centerX = (t.ax + t.bx + t.cx) / 3.0f;
  float centerY = (t.ay + t.by + t.cy) / 3.0f;

  glTranslatef(-centerX, -centerY, 0.0f);
  glRotatef(t.transformation.angle, 0.0, 0.0, 1.0);
  glTranslatef(centerX, centerY, 0.0f);

  // Save the updated transformation matrix
  glGetFloatv(GL_MODELVIEW_MATRIX, t.transformation.matrix);

  printf("Transformation Matrix (Angle: %.2f degrees)\n",
         t.transformation.angle);
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      printf("%.2f ", t.transformation.matrix[i * 4 + j]);
    }
    printf("\n");
  }
  printf("\n");
}

void drawTriangle() {
  glBegin(GL_TRIANGLES);
  glColor3f(1.0f, 1.0f, 1.0f);
  glVertex2f(t.ax, t.ay);
  glVertex2f(t.bx, t.by);
  glVertex2f(t.cx, t.cy);
  glEnd();

  float centerX = (t.ax + t.bx + t.cx) / 3.0f;
  float centerY = (t.ay + t.by + t.cy) / 3.0f;

  glPointSize(10.0f);
  glBegin(GL_POINTS);
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex2f(centerX, centerY);
  glEnd();
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT);

  // Apply the updated transformation matrix
  glLoadMatrixf(t.transformation.matrix);

  drawTriangle();

  glFlush();
}

void handleKeypress(unsigned char key, int x, int y) {
  switch (key) {
    case 'q':
      t.transformation.angle += 5.0f;
      break;
    case 'e':
      t.transformation.angle -= 5.0f;
      break;
    case 'w':
      t.transformation.ty += 0.03f;
      break;
    case 's':
      t.transformation.ty -= 0.03f;
      break;
    case 'a':
      t.transformation.tx -= 0.03f;
      break;
    case 'd':
      t.transformation.tx += 0.03f;
      break;
    case '+':
      t.transformation.scale += 0.1f;
      break;
    case '-':
      t.transformation.scale -= 0.1f;
      break;
    case 'i':
      t.transformation.shearX += 0.05f;
      break;
    case 'k':
      t.transformation.shearX -= 0.05f;
      break;
    case 'j':
      t.transformation.shearY += 0.05f;
      break;
    case 'l':
      t.transformation.shearY -= 0.05f;
      break;
    case 'x':
      t.transformation.reflectX = !t.transformation.reflectX;
      break;
    case 'y':
      t.transformation.reflectY = !t.transformation.reflectY;
      break;
  }

  updateTransformationMatrix();

  glutPostRedisplay();
}

int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(500, 500);
  glutCreateWindow("Triangle Rotation");

  glClearColor(0.0, 0.0, 0.0, 1.0);

  glutDisplayFunc(display);
  glutKeyboardFunc(handleKeypress);

  // Initialize the transformation matrix
  initTriangle();
  updateTransformationMatrix();

  glutMainLoop();
  return 0;
}
