#ifndef TANKOBJECT_H
#define TANKOBJECT_H

#include <QCAR/Renderer.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

class Tank
{
private:
  float pos_x;
  float pos_y;
  float pos_z;
  float start_angle;
  float scale;
  float turret_angle;
  float barrel_angle;
  Tank();

public:
  Tank(float posx, float posy, float posz, float sAngle, float tscale);
  ~Tank();

  void
  setPosition(float posx, float posy, float posz, float angle);
  void
  setTurretAngle(float angle);
  void
  setBarrelAngle(float angle);
  void
  render(const QCAR::Trackable* trackable, QCAR::Matrix44F* projectionMatrix,
      QCAR::Matrix44F* modelViewProjectionScaled, GLint vertexHandle,
      GLint normalHandle, GLint textureCoordHandle, GLint mvpMatrixHandle,
      unsigned int baseTexId, unsigned int headTexId);
};

#endif
