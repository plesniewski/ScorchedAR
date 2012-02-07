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
GLUtils::printPosition(const float* mat)
{
  LOG("%7.3f %7.3f %7.3f", mat[12], mat[13], mat[14]);
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
GLUtils::findNoise(double x, double y, int st, int seed)
{
  int n = (int) x + (int) y * 57 + seed;
  n = (n << 13) ^ n;
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
GLUtils::perlinNoise(double x, double y, int st, int seed)
{
  double floorx = (double) ((int) x); //This is kinda a cheap way to floor a double integer.
  double floory = (double) ((int) y);
  double s, t, u, v; //Integer declaration
  s = findNoise(floorx, floory, st, seed);
  t = findNoise(floorx + 1, floory, st, seed);
  u = findNoise(floorx, floory + 1, st, seed); //Get the surrounding pixels to calculate the transition.
  v = findNoise(floorx + 1, floory + 1, st, seed);
  double int1 = interpolate(s, t, x - floorx); //Interpolate between the values.
  double int2 = interpolate(u, v, x - floorx); //Here we use x-floorx, to get 1st dimension. Don't mind the x-floorx thingie, it's part of the cosine formula.
  return interpolate(int1, int2, y - floory); //Here we use y-floory, to get the 2nd dimension.
}

void
GLUtils::generateHeightMap(float map[TERRAIN_WIDTH][TERRAIN_HEIGHT],
    double zoom)
{
  srand((unsigned) time(NULL));
  int st = (rand() % 50);
  int seed = (rand() % 500);
  if (st % 2 == 0)
    st += 1;
  int octaves = 2;
  double p = 3.0f;
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

          for (int a = 0; a < octaves - 1; a++)
            {
              double frequency = pow(2, a);
              double amplitude = pow(p, a);

              getnoise += perlinNoise(((double) x) * frequency / zoom,
                  ((double) y) / zoom * frequency, st, seed) * amplitude;

            } //
          int value = (int) ((getnoise * 70.0) + 70.0); //Convert to 0-60 values.
          if (value > 70)
            value = 70;
          if (value < 0)
            value = 0;
          map[x][y] = (float) value;
        }
    }
}

void
GLUtils::cross(float a[3], float b[3], float &cx, float &cy, float &cz)
{
  cx = a[1] * b[2] - a[2] * b[1];
  cy = a[2] * b[0] - a[0] * b[2];
  cz = a[0] * b[1] - a[1] * b[0];
}

void
GLUtils::createNormals(float map[TERRAIN_WIDTH][TERRAIN_HEIGHT], float n[])
{
  float v1[3], v2[3];
  float nx, ny, nz, m;
  int i = 0;
  for (int y = 0; y < TERRAIN_HEIGHT - 1; y++)
    {
      for (int x = 0; x < TERRAIN_WIDTH - 1; x++)
        {

          v1[0] = 1.0f;
          v1[1] = 0.0f;
          v1[2] = map[x + 1][y] - map[x][y];

          v2[0] = 0.0f;
          v2[1] = -1.0f;
          v2[2] = map[x][y + 1] - map[x][y];

          cross(v2, v1, nx, ny, nz);

          m = sqrt(nx * nx + ny * ny + nz * nz);

          n[i] = n[i + 3] = n[i + 6] = nx / m;
          n[i + 1] = n[i + 4] = n[i + 7] = ny / m;
          n[i + 2] = n[i + 5] = n[i + 8] = nz / m;
          n += 9;
          v1[0] = 0.0f;
          v1[1] = -1.0f;
          v1[2] = map[x + 1][y + 1] - map[x + 1][y];

          v2[0] = -1.0f;
          v2[1] = -1.0f;
          v2[2] = map[x][y + 1] - map[x + 1][y];

          cross(v2, v1, nx, ny, nz);
          m = sqrt(nx * nx + ny * ny + nz * nz);

          n[i] = n[i + 3] = n[i + 6] = nx / m;
          n[i + 1] = n[i + 4] = n[i + 7] = ny / m;
          n[i + 2] = n[i + 5] = n[i + 8] = nz / m;

        }
    }
}

void
GLUtils::createNormalsForFaces(float map[TERRAIN_WIDTH][TERRAIN_HEIGHT],
    float n[])
{
  int i = 0;
  float nx, ny, m;
  for (int y = 0; y < TERRAIN_HEIGHT - 1; y++)
    {
      for (int x = 0; x < TERRAIN_WIDTH - 1; x++)
        {
          //first tri

          nx = (map[x][y] - map[x + 1][y]);
          ny = (map[x][y + 1] - map[x][y]);

          m = sqrt(nx * nx + ny * ny + 1.0f);
          n[i] = n[i + 3] = n[i + 6] = nx / m;
          n[i + 1] = n[i + 4] = n[i + 7] = ny / m;
          n[i + 2] = n[i + 5] = n[i + 8] = 1.0f / m;
          n += 9;
          //second tri
          nx = (map[x][y + 1] - map[x + 1][y + 1]);
          ny = (map[x + 1][y + 1] - map[x + 1][y]);
          m = sqrt(nx * nx + ny * ny + 1.0f);
          n[i] = n[i + 3] = n[i + 6] = nx / m;
          n[i + 1] = n[i + 4] = n[i + 7] = ny / m;
          n[i + 2] = n[i + 5] = n[i + 8] = 1.0f / m;
          n += 9;
        }
    }
}

void
GLUtils::produceArrays(float verts[], float texts[],
    float map[TERRAIN_WIDTH][TERRAIN_HEIGHT])
{
  LOG("Updating terrain arrays");
  int v = 0;
  int t = 0;
  for (int y = 0; y < TERRAIN_HEIGHT - 1; y++)
    {
      verts[v++] = 0.0f;
      verts[v++] = -y;
      verts[v++] = map[0][y];
      for (int x = 0; x < TERRAIN_WIDTH - 1; x++)
        {
          verts[v++] = x;
          verts[v++] = -y - 1;
          verts[v++] = map[x][y + 1];

          verts[v++] = x + 1;
          verts[v++] = -y;
          verts[v++] = map[x + 1][y];

          //text coords
          texts[t] = texts[t + 2] = texts[t + 3] = texts[t + 7] = 0.f;
          texts[t + 1] = texts[t + 4] = texts[t + 5] = texts[t + 6] = 1.f;
          t += 8;
        }
      verts[v++] = TERRAIN_WIDTH - 1;
      verts[v++] = -y - 1;
      verts[v++] = map[TERRAIN_WIDTH - 1][y + 1];
    }

  LOG("Creating arrays completed");
}

bool
GLUtils::checkTerrainCollision(float map[TERRAIN_WIDTH][TERRAIN_HEIGHT],
    Bullet*& bullet, int &col_x, int &col_y, float &col_z)
{
  float bPos_x, bPos_y, bPos_z;
  bullet->getPosition(bPos_x, bPos_y, bPos_z);
  /**
   * terrain position is moved by half of trackable so we have to fix positions to match them in array
   * then we can check coordinates for 0-247 x and 0-125 y
   */
  bPos_x += ((float) TERRAIN_WIDTH) / 2;
  bPos_y -= 173.f / 2;
  LOG("bposit:  %7.2f %7.2f %7.2f", bPos_x, bPos_y, bPos_z);
  if (bPos_x >= 0 && bPos_x < 247 && -bPos_y >= 0 && -bPos_y < 125)
    {
      if (map[(int) bPos_x][(int) -bPos_y] >= bPos_z)
        {
          col_x = (int) bPos_x;
          col_y = (int) -bPos_y;
          col_z = map[(int) bPos_x][(int) -bPos_y];
          return true;
        }
      else
        {
          return false;
        }
    }
  else
    {
      return false;
    }

}

bool
GLUtils::checkOutOfMap(Bullet*& bullet)
{
  float bPos_x, bPos_y, bPos_z;
  bullet->getPosition(bPos_x, bPos_y, bPos_z);
  return !(bPos_x >= -131 && bPos_x < 131 && bPos_y >= -100 && bPos_y < 100);
}

void
GLUtils::destroyTerrain(float map[TERRAIN_WIDTH][TERRAIN_HEIGHT], int colx,
    int coly, float colz)
{
  int ax, bx, ay, by;
  //first we must make array bounds safe
  if (colx < 10)
    ax = -colx;
  else
    ax = -10;
  if (coly < 10)
    ay = -coly;
  else
    ay = -10;
  if (colx > TERRAIN_WIDTH - 10)
    bx = TERRAIN_WIDTH - colx;
  else
    bx = 10;
  if (coly > TERRAIN_HEIGHT - 11)
    by = TERRAIN_HEIGHT - 11;
  else
    by = 11;

  int f = 0.f;
  int g = 0.f;
  for (int j = ay; j < by; j++)
    {
      if (g > 12)
        g = 0;
      if (j % 2 == 0)
        g++;
      for (int i = ax; i < bx; i++)
        {
          if (f > 12)
            f = 0;
          if (i % 2 == 0)
            f++;

          if (map[colx + i][coly + j] >= colz - f - g)
            map[colx + i][coly + j] -= 1.0f;
        }
    }
}
