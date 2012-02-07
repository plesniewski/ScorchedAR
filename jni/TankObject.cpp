#include "TankObject.h"

#include <math.h>
#include <stdlib.h>

#include <QCAR/Renderer.h>

#include <QCAR/Trackable.h>
#include <QCAR/Tool.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "GLUtils.h"
#include "TankBase.h"
#include "TankTurret.h"

Tank::Tank(float posx, float posy, float posz, float sAngle, float tscale)
{
  setPosition(posx, posy, posz, sAngle);
  scale = tscale;
}

void
Tank::setPosition(float posx, float posy, float posz, float angle)
{
  pos_x = posx;
  pos_y = posy;
  pos_z = posz;
  turret_angle = angle;
  start_angle = angle;

}

void
Tank::setTurretAngle(float angle)
{
  turret_angle = angle;
}

void
Tank::setBarrelAngle(float angle)
{
  barrel_angle = angle;
}

void
Tank::render(const QCAR::Trackable* trackable,
    QCAR::Matrix44F* projectionMatrix,
    QCAR::Matrix44F* modelViewProjectionScaled, GLint vertexHandle,
    GLint textureCoordHandle, GLint mvpMatrixHandle, unsigned int baseTexId,
    unsigned int headTexId)
{
  QCAR::Matrix44F turretViewMatrix = QCAR::Tool::convertPose2GLMatrix(
      trackable->getPose());
  //move turret up
  GLUtils::translatePoseMatrix(pos_x, pos_y, pos_z + 4.0f,
      &turretViewMatrix.data[0]);

  GLUtils::scalePoseMatrix(scale, scale, scale, &turretViewMatrix.data[0]);
  //rotate
  GLUtils::rotatePoseMatrix(turret_angle, 0.0f, 0.0f, 1.0f,
      &turretViewMatrix.data[0]);
  GLUtils::rotatePoseMatrix(barrel_angle, 0.0f, 1.0f, 0.0f,
      &turretViewMatrix.data[0]);

  GLUtils::multiplyMatrix(&projectionMatrix->data[0], &turretViewMatrix.data[0],
      &modelViewProjectionScaled->data[0]);

  glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
      (const GLvoid*) &tankTurretVerts[0]);

  glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
      (const GLvoid*) &tankTurretTexCoords[0]);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, baseTexId);

  glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
      (GLfloat*) &modelViewProjectionScaled->data[0]);

  glDrawArrays(GL_TRIANGLES, 0, tankTurretNumVerts);

  //korpus
  QCAR::Matrix44F baseViewMatrix = QCAR::Tool::convertPose2GLMatrix(
      trackable->getPose());
  GLUtils::translatePoseMatrix(pos_x, pos_y, pos_z, &baseViewMatrix.data[0]);
  GLUtils::scalePoseMatrix(scale, scale, scale, &baseViewMatrix.data[0]);
  GLUtils::rotatePoseMatrix(start_angle, 0.0f, 0.0f, 1.0f,
      &baseViewMatrix.data[0]);

  // Render 3D model
  GLUtils::multiplyMatrix(&projectionMatrix->data[0], &baseViewMatrix.data[0],
      &modelViewProjectionScaled->data[0]);
  //glUseProgram(shaderProgramID);

  glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
      (const GLvoid*) &tankBaseVerts[0]);

  glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
      (const GLvoid*) &tankBaseTexCoords[0]);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, baseTexId);

  glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
      (GLfloat*) &modelViewProjectionScaled->data[0]);

  glDrawArrays(GL_TRIANGLES, 0, tankBaseNumVerts);

  GLUtils::checkGlError("VirtualButtons renderFrame");

}

float
Tank::getX()
{
  return pos_x;
}

float
Tank::getY()
{
  return pos_y;
}

float
Tank::getZ()
{
  return pos_z;
}

void
Tank::setZ(float pz)
{
  pos_z = pz;
}
