#ifndef GLUTILS_H_
#define GLUTILS_H_

#include <stdio.h>
#include <android/log.h>
#include "Globals.h"

// Utility for logging:
#define LOG_TAG    "QCAR"
#define LOG(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

class GLUtils
{

private:
  static inline double
  findNoise(double x, double y, int st, int seed);

  static inline double
  interpolate(double a, double b, double x);

  static double
  perlinNoise(double x, double y, int st, int seed);

  static void
  cross(float a[3], float b[3], float &cx, float &cy, float &cz);

public:

  /// Prints a 4x4 matrix.
  static void
  printMatrix(const float* matrix);

  /// Prints GL error information.
  static void
  checkGlError(const char* operation);

  /// Set the rotation components of this 4x4 matrix.
  static void
  setRotationMatrix(float angle, float x, float y, float z, float *nMatrix);

  /// Set the translation components of this 4x4 matrix.
  static void
  translatePoseMatrix(float x, float y, float z, float* nMatrix = NULL);

  /// Applies a rotation.
  static void
  rotatePoseMatrix(float angle, float x, float y, float z,
      float* nMatrix = NULL);

  /// Applies a scaling transformation.
  static void
  scalePoseMatrix(float x, float y, float z, float* nMatrix = NULL);

  /// Multiplies the two matrices A and B and writes the result to C.
  static void
  multiplyMatrix(float *matrixA, float *matrixB, float *matrixC);

  /// Initialize a shader.
  static unsigned int
  initShader(unsigned int shaderType, const char* source);

  /// Create a shader program.
  static unsigned int
  createProgramFromBuffer(const char* vertexShaderBuffer,
      const char* fragmentShaderBuffer);

  /// Generate terrain height map.
  static void
  generateHeightMap(float map[TERRAIN_WIDTH][TERRAIN_HEIGHT], double zoom);

  /// Produce OpenGL arrays of verticies, normals and texture coords
  static void
  produceArrays(float verts[], float texts[], float norms[],
      float map[TERRAIN_WIDTH][TERRAIN_HEIGHT]);

  static void
  createNormals(float map[TERRAIN_WIDTH][TERRAIN_HEIGHT], float norms[]);

  /// Creates array of faces normals
  static void
  createNormalsForFaces(float [TERRAIN_WIDTH][TERRAIN_HEIGHT], float norms[]);
};

#endif
