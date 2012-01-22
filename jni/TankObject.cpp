#include "TankObject.h"

#include <math.h>
#include <stdlib.h>

#include <QCAR/Renderer.h>

#include <QCAR/Trackable.h>
#include <QCAR/Tool.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "GLUtils.h"
#include "tankv2.h"
#include "tankTurret.h"

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
    GLint normalHandle, GLint textureCoordHandle, GLint mvpMatrixHandle,
    unsigned int texId)
{
  QCAR::Matrix44F turretViewMatrix = QCAR::Tool::convertPose2GLMatrix(
      trackable->getPose());
  GLUtils::scalePoseMatrix(scale, scale, scale, &turretViewMatrix.data[0]);
  //move turret up
  GLUtils::translatePoseMatrix(0.0f, 0.0f, scale * 0.01f,
      &turretViewMatrix.data[0]);
  //rotate
  GLUtils::rotatePoseMatrix(turret_angle, 0.0f, 0.0f, 1.0f,
      &turretViewMatrix.data[0]);
  GLUtils::rotatePoseMatrix(barrel_angle, 0.0f, 1.0f, 0.0f,
      &turretViewMatrix.data[0]);

  GLUtils::multiplyMatrix(&projectionMatrix->data[0], &turretViewMatrix.data[0],
      &modelViewProjectionScaled->data[0]);

  glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
      (const GLvoid*) &tankTurretVerts[0]);

  glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
      (const GLvoid*) &tankTurretNormals[0]);

  glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
      (const GLvoid*) &tankTurretTexCoords[0]);


  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texId);

  glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
      (GLfloat*) &modelViewProjectionScaled->data[0]);

  glDrawArrays(GL_TRIANGLES, 0, tankTurretNumVerts);

  //korpus
  QCAR::Matrix44F baseViewMatrix = QCAR::Tool::convertPose2GLMatrix(
      trackable->getPose());
  GLUtils::scalePoseMatrix(scale, scale, scale, &baseViewMatrix.data[0]);

  // Render 3D model
  GLUtils::multiplyMatrix(&projectionMatrix->data[0], &baseViewMatrix.data[0],
      &modelViewProjectionScaled->data[0]);
  //glUseProgram(shaderProgramID);

  glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
      (const GLvoid*) &tankv2Verts[0]);

  glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
      (const GLvoid*) &tankv2Normals[0]);

  glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
      (const GLvoid*) &tankv2TexCoords[0]);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texId);

  glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
      (GLfloat*) &modelViewProjectionScaled->data[0]);

  glDrawArrays(GL_TRIANGLES, 0, tankv2NumVerts);

  GLUtils::checkGlError("VirtualButtons renderFrame");

}

