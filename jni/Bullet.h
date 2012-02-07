#ifndef BULLET_H
#define BULLET_H

#include <QCAR/Renderer.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

class Bullet
{
private:
  float tpx;
  float tpy;
  float tpz;
  float pos_x;
  float pos_y;
  float pos_z;
  float scale;
  float start_velocity;
  float gravity;
  float move_pos_x;
  float move_pos_y;
  float move_pos_z;
  float vert_angle;
  float hori_angle;
  bool go;
  float time;
  const static float PI = 3.141;
  Bullet();

  /**
   * Sets start position from tank position and angles, translate angles to radians
   */
  void
  setStartPositionAndAngles(float t_posx, float t_posy, float t_posz,
      float v_angle, float h_angle);
public:
  /**
   * Takes tank position and turret angles as a parameter, scale is for bullet size
   */
  Bullet(float t_posx, float t_posy, float t_posz, float sAngle, float xAngle,
      float velocity, float scale);
  ~Bullet();

  void
  getPosition(float &px, float &py, float &pz);

  void
  setAngles(float vertical, float horizontal);

  void
  proceed(float timeStep);

  void
  printPosition();

  void
  render(const QCAR::Trackable* trackable, QCAR::Matrix44F* projectionMatrix,
      QCAR::Matrix44F* modelViewProjectionScaled, GLint vertexHandle,
      GLint mvpMatrixHandle);
};

#endif
