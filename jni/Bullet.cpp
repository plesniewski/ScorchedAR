#include "Bullet.h"

#include <math.h>
#include <stdlib.h>

#include <QCAR/Renderer.h>

#include <QCAR/Trackable.h>
#include <QCAR/Tool.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "GLUtils.h"
#include "TankBullet.h"

Bullet::Bullet(float posx, float posy, float posz, float sAngle, float xAngle,
    float velocity, float bScale)
{
  start_velocity = velocity;
  scale = bScale;
  gravity = 10.f;
  move_pos_x = 0.f;
  move_pos_y = 0.f;
  move_pos_z = 0.f;
  tpx = posx;
  tpy = posy;
  tpz = posz;
  setStartPositionAndAngles(posx, posy, posz, sAngle, xAngle);
  go = false;
  time = 0;
}

void
Bullet::setStartPositionAndAngles(float t_posx, float t_posy, float t_posz,
    float v_angle, float h_angle)
{
  vert_angle = v_angle * PI / 180;
  hori_angle = (h_angle - 180) * PI / 180;
  pos_x = t_posx + 12.0 * cos(hori_angle) * cos(vert_angle);
  pos_y = t_posy + 12.0 * sin(hori_angle) * cos(vert_angle);
  pos_z = t_posz + 12.0 * sin(vert_angle);

}

void
Bullet::setAngles(float vertical, float horizontal)
{
  setStartPositionAndAngles(tpx, tpy, tpz, vertical, horizontal);
}

void
Bullet::getPosition(float &px, float &py, float &pz)
{
  px = pos_x + move_pos_x;
  py = pos_y + move_pos_y;
  pz = pos_z + move_pos_z;
}

void
Bullet::printPosition()
{
  LOG("bullet pos: %7.3f %7.3f %7.3f", pos_x + move_pos_x, pos_y + move_pos_y,
      pos_z + move_pos_z);
  LOG("hangle: %7.3f   %7.3f", vert_angle, hori_angle);
}

void
Bullet::proceed(float step)
{
  go = true;
  time += step;
  move_pos_x = pos_x + start_velocity * time * cos(hori_angle);
  move_pos_y = pos_y + start_velocity * time * sin(hori_angle);
  move_pos_z = pos_z + start_velocity * time * sin(vert_angle)
      - gravity * time * time * 0.5;
  /*
   move_pos_x += step;
   //pos_y += tan(x_angle) * pos_x;


   move_pos_z = move_pos_x * tan(alpha)
   - (gravity * move_pos_x * move_pos_x)
   / (2 * start_velocity * start_velocity * cos(alpha)
   * cos(alpha));

   LOG("%7.3f   %7.3f", move_pos_x, move_pos_z);
   /*LOG("%7.3f  %7.3f   %7.3f ", move_pos_x, move_pos_z,(gravity * move_pos_x * move_pos_x)
   / (2 * start_velocity * start_velocity * cos(alpha)
   * cos(alpha)));*/
}

void
Bullet::render(const QCAR::Trackable* trackable,
    QCAR::Matrix44F* projectionMatrix,
    QCAR::Matrix44F* modelViewProjectionScaled, GLint vertexHandle,
    GLint mvpMatrixHandle)
{
  QCAR::Matrix44F bulletViewMatrix = QCAR::Tool::convertPose2GLMatrix(
      trackable->getPose());

  if (!go)
    GLUtils::translatePoseMatrix(pos_x, pos_y, pos_z,
        &bulletViewMatrix.data[0]);
  /*
   GLUtils::rotatePoseMatrix(x_angle - 180.f, 0.0f, 0.0f, 1.0f,
   &bulletViewMatrix.data[0]);
   GLUtils::rotatePoseMatrix(-start_angle, 0.0f, 1.0f, 0.0f,
   &bulletViewMatrix.data[0]);

   GLUtils::translatePoseMatrix(12.0f, 0.0f, 0.f, &bulletViewMatrix.data[0]);
   GLUtils::rotatePoseMatrix(start_angle, 0.0f, 1.0f, 0.0f,
   &bulletViewMatrix.data[0]);*/
  GLUtils::translatePoseMatrix(move_pos_x, move_pos_y, move_pos_z,
      &bulletViewMatrix.data[0]);
  GLUtils::scalePoseMatrix(scale, scale, scale, &bulletViewMatrix.data[0]);

  GLUtils::multiplyMatrix(&projectionMatrix->data[0], &bulletViewMatrix.data[0],
      &modelViewProjectionScaled->data[0]);
  //GLUtils::printPosition((GLfloat*) &modelViewProjectionScaled->data[0]);
  glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
      (const GLvoid*) &TankBulletVerts[0]);

  glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
      (GLfloat*) &modelViewProjectionScaled->data[0]);

  glDrawArrays(GL_TRIANGLES, 0, TankBulletNumVerts);
}
