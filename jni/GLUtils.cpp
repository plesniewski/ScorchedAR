#include "GLUtils.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

void
GLUtils::printMatrix(const float* mat)
{
  for (int r = 0; r < 4; r++, mat += 4)
    LOG("%7.3f %7.3f %7.3f %7.3f", mat[0], mat[1], mat[2], mat[3]);
}

void
GLUtils::checkGlError(const char* operation)
{
  for (GLint error = glGetError(); error; error = glGetError())
    LOG("after %s() glError (0x%x)", operation, error);
}

void
GLUtils::translatePoseMatrix(float x, float y, float z, float* matrix)
{
  // Sanity check
  if (!matrix)
    return;

  // matrix * translate_matrix
  matrix[12] += (matrix[0] * x + matrix[4] * y + matrix[8] * z);
  matrix[13] += (matrix[1] * x + matrix[5] * y + matrix[9] * z);
  matrix[14] += (matrix[2] * x + matrix[6] * y + matrix[10] * z);
  matrix[15] += (matrix[3] * x + matrix[7] * y + matrix[11] * z);
}

void
GLUtils::rotatePoseMatrix(float angle, float x, float y, float z, float* matrix)
{
  // Sanity check
  if (!matrix)
    return;

  float rotate_matrix[16];
  GLUtils::setRotationMatrix(angle, x, y, z, rotate_matrix);

  // matrix * scale_matrix
  GLUtils::multiplyMatrix(matrix, rotate_matrix, matrix);
}

void
GLUtils::scalePoseMatrix(float x, float y, float z, float* matrix)
{
  // Sanity check
  if (!matrix)
    return;

  // matrix * scale_matrix
  matrix[0] *= x;
  matrix[1] *= x;
  matrix[2] *= x;
  matrix[3] *= x;

  matrix[4] *= y;
  matrix[5] *= y;
  matrix[6] *= y;
  matrix[7] *= y;

  matrix[8] *= z;
  matrix[9] *= z;
  matrix[10] *= z;
  matrix[11] *= z;
}

void
GLUtils::multiplyMatrix(float *matrixA, float *matrixB, float *matrixC)
{
  int i, j, k;
  float aTmp[16];

  for (i = 0; i < 4; i++)
    {
      for (j = 0; j < 4; j++)
        {
          aTmp[j * 4 + i] = 0.0;

          for (k = 0; k < 4; k++)
            aTmp[j * 4 + i] += matrixA[k * 4 + i] * matrixB[j * 4 + k];
        }
    }

  for (i = 0; i < 16; i++)
    matrixC[i] = aTmp[i];
}

void
GLUtils::setRotationMatrix(float angle, float x, float y, float z,
    float *matrix)
{
  double radians, c, s, c1, u[3], length;
  int i, j;

  radians = (angle * M_PI) / 180.0;

  c = cos(radians);
  s = sin(radians);

  c1 = 1.0 - cos(radians);

  length = sqrt(x * x + y * y + z * z);

  u[0] = x / length;
  u[1] = y / length;
  u[2] = z / length;

  for (i = 0; i < 16; i++)
    matrix[i] = 0.0;

  matrix[15] = 1.0;

  for (i = 0; i < 3; i++)
    {
      matrix[i * 4 + (i + 1) % 3] = u[(i + 2) % 3] * s;
      matrix[i * 4 + (i + 2) % 3] = -u[(i + 1) % 3] * s;
    }

  for (i = 0; i < 3; i++)
    {
      for (j = 0; j < 3; j++)
        matrix[i * 4 + j] += c1 * u[i] * u[j] + (i == j ? c : 0.0);
    }
}

unsigned int
GLUtils::initShader(unsigned int shaderType, const char* source)
{
  GLuint shader = glCreateShader((GLenum) shaderType);
  if (shader)
    {
      glShaderSource(shader, 1, &source, NULL);
      glCompileShader(shader);
      GLint compiled = 0;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

      if (!compiled)
        {
          GLint infoLen = 0;
          glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
          if (infoLen)
            {
              char* buf = (char*) malloc(infoLen);
              if (buf)
                {
                  glGetShaderInfoLog(shader, infoLen, NULL, buf);
                  LOG("Could not compile shader %d: %s", shaderType, buf);
                  free(buf);
                }
              glDeleteShader(shader);
              shader = 0;
            }
        }
    }
  return shader;
}

unsigned int
GLUtils::createProgramFromBuffer(const char* vertexShaderBuffer,
    const char* fragmentShaderBuffer)
{
  GLuint vertexShader = initShader(GL_VERTEX_SHADER, vertexShaderBuffer);
  if (!vertexShader)
    return 0;

  GLuint fragmentShader = initShader(GL_FRAGMENT_SHADER, fragmentShaderBuffer);
  if (!fragmentShader)
    return 0;

  GLuint program = glCreateProgram();
  if (program)
    {
      glAttachShader(program, vertexShader);
      checkGlError("glAttachShader");

      glAttachShader(program, fragmentShader);
      checkGlError("glAttachShader");

      glLinkProgram(program);
      GLint linkStatus = GL_FALSE;
      glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

      if (linkStatus != GL_TRUE)
        {
          GLint bufLength = 0;
          glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
          if (bufLength)
            {
              char* buf = (char*) malloc(bufLength);
              if (buf)
                {
                  glGetProgramInfoLog(program, bufLength, NULL, buf);
                  LOG("Could not link program: %s", buf);
                  free(buf);
                }
            }
          glDeleteProgram(program);
          program = 0;
        }
    }
  return program;
}

inline double
GLUtils::findNoise(double x, double y, int st)
{
  int n = (int) x + (int) y * 57;
  n = (n << st) ^ n;
  int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
  return 1.0 - ((double) nn / 1073741824.0);
}

inline double
GLUtils::interpolate(double a, double b, double x)
{
  double ft = x * 3.1415927;
  double f = (1.0 - cos(ft)) * 0.5;
  return a * (1.0 - f) + b * f;
}

double
GLUtils::perlinNoise(double x, double y, int st)
{
  double floorx = (double) ((int) x); //This is kinda a cheap way to floor a double integer.
  double floory = (double) ((int) y);
  double s, t, u, v; //Integer declaration
  s = findNoise(floorx, floory, st);
  t = findNoise(floorx + 1, floory, st);
  u = findNoise(floorx, floory + 1, st); //Get the surrounding pixels to calculate the transition.
  v = findNoise(floorx + 1, floory + 1, st);
  double int1 = interpolate(s, t, x - floorx); //Interpolate between the values.
  double int2 = interpolate(u, v, x - floorx); //Here we use x-floorx, to get 1st dimension. Don't mind the x-floorx thingie, it's part of the cosine formula.
  return interpolate(int1, int2, y - floory); //Here we use y-floory, to get the 2nd dimension.
}

void
GLUtils::generateHeightMap(int map[TERRAIN_WIDTH][TERRAIN_HEIGHT], double zoom)
{
  srand((unsigned) time(NULL));
  int st = (rand() % 50);
  if (st % 2 == 0)
    st += 1;
  int octaves = 2;
  double p = 5.0f;
  for (int y = 0; y < TERRAIN_HEIGHT; y++)
    {
      map[0][y] = map[TERRAIN_WIDTH - 1][y] = 0;
      for (int x = 1; x < TERRAIN_WIDTH - 1; x++)
        {
          if (y == 0 || y == TERRAIN_HEIGHT - 1)
            {
              map[x][y] = 0;
              continue;
            }
          double getnoise = 0;

          for (int a = 0; a < octaves - 1; a++) //This loops trough the octaves.
            {
              double frequency = pow(10, a); //This increases the frequency with every loop of the octave.
              double amplitude = pow(p, a); //This decreases the amplitude with every loop of the octave.

              getnoise += perlinNoise(((double) x) * frequency / zoom,
                  ((double) y) / zoom * frequency, st) * amplitude; //This uses our perlin noise functions. It calculates all our zoom and frequency and amplitude

            } //                                          It gives a decimal value, you know, between the pixels. Like 4.2 or 5.1
          int value = (int) ((getnoise * 20.0) + 20.0); //Convert to 0-128 values.
          if (value > 20)
            value = 20;
          if (value < 0)
            value = 0;
          map[x][y] = value;
        }
    }
}

void
GLUtils::cross(int a[3], int b[3], int &cx, int &cy, int &cz)
{
  cx = a[1] * b[2] - a[2] * b[1];
  cy = a[2] * b[0] - a[0] * b[2];
  cz = a[0] * b[1] - a[1] * b[0];
}

void
GLUtils::createNormals(int map[TERRAIN_WIDTH][TERRAIN_HEIGHT], float norms[])
{
  /* int v1[3], v2[3], v3[3], v4[3], v5[3], v6[3];
   int i = 0;
   for (int y = 0; y < TERRAIN_HEIGHT; y++)
   {
   for (int x = 0; x < TERRAIN_WIDTH; x++)
   {
   if (y == 0 && x == 0)
   {
   // back left corner - 1 tri 2 vertices
   v1 =
   { 1, 0 , map[1][0] - map[0][0]};
   v2 =
   { 0, -1, map[0][1] - map[0][0]};
   cross(v1,v2, n[i], n[i+1], n[i+2]);
   i+=3;
   }
   }
   }*/
}

void
GLUtils::createNormalsForFaces(int map[TERRAIN_WIDTH][TERRAIN_HEIGHT],
    float n[])
{
  int i = 0;
  float nx, ny, m;
  for (int y = 0; y < TERRAIN_HEIGHT - 1; y++)
    {
      for (int x = 0; x < TERRAIN_WIDTH - 1; x++)
        {
          //first tri

          nx = (float) (map[x][y] - map[x + 1][y]);
          ny = (float) (map[x][y + 1] - map[x][y]);
          m = sqrt(nx * nx + ny * ny + 1.0f);
          n[i] = n[i + 3] = n[i + 6] = nx / m;
          n[i + 1] = n[i + 4] = n[i + 7] = ny / m;
          n[i + 2] = n[i + 5] = n[i + 8] = 1.0f / m;
          n += 9;
          //second tri
          nx = (float) (map[x][y + 1] - map[x + 1][y + 1]);
          ny = (float) (map[x + 1][y + 1] - map[x + 1][y]);
          m = sqrt(nx * nx + ny * ny + 1.0f);
          n[i] = n[i + 3] = n[i + 6] = nx / m;
          n[i + 1] = n[i + 4] = n[i + 7] = ny / m;
          n[i + 2] = n[i + 5] = n[i + 8] = 1.0f / m;
          n += 9;
        }
    }
}

void
GLUtils::produceArrays(float verts[], float texts[], float norms[],
    int map[TERRAIN_WIDTH][TERRAIN_HEIGHT])
{
  LOG("Updating terrain arrays");
  int v = 0;
  int t = 0;
  for (int y = 0; y < TERRAIN_HEIGHT - 1; y++)
    {
      verts[v++] = (float) (0);
      verts[v++] = (float) (-y);
      verts[v++] = (float) (map[0][y]);
      for (int x = 0; x < TERRAIN_WIDTH - 1; x++)
        {
          verts[v++] = (float) (x);
          verts[v++] = (float) (-y - 1);
          verts[v++] = (float) (map[x][y + 1]);

          verts[v++] = (float) (x + 1);
          verts[v++] = (float) (-y);
          verts[v++] = (float) (map[x + 1][y]);

          //text coords
          texts[t] = texts[t + 2] = texts[t + 3] = texts[t + 7] = 0.f;
          texts[t + 1] = texts[t + 4] = texts[t + 5] = texts[t + 6] = 1.f;
          t += 8;
        }
      verts[v++] = float(TERRAIN_WIDTH - 1);
      verts[v++] = float(-y - 1);
      verts[v++] = float(map[TERRAIN_WIDTH - 1][y + 1]);
    }
  createNormalsForFaces(map, norms);
  LOG("Creating arrays completed");
}

