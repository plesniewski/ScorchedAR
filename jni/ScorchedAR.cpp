#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <QCAR/QCAR.h>
#include <QCAR/UpdateCallback.h>
#include <QCAR/CameraDevice.h>
#include <QCAR/Renderer.h>
#include <QCAR/Area.h>
#include <QCAR/Rectangle.h>
#include <QCAR/VideoBackgroundConfig.h>
#include <QCAR/Trackable.h>
#include <QCAR/Tool.h>
#include <QCAR/Tracker.h>
#include <QCAR/CameraCalibration.h>
#include <QCAR/ImageTarget.h>
#include <QCAR/VirtualButton.h>

#include "GLUtils.h"
#include "Texture.h"
#include "TankShader.h"
#include "TerrainShader.h"
#include "BulletShader.h"
#include "ProgressShader.h"

#include "TankObject.h"
#include "Bullet.h"
#include "Globals.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Textures:
  int textureCount = 0;
  Texture** textures = 0;
  int terrainTextureIndex = 4;
  Tank** tanks = 0;
  Bullet** bullet = 0;

  bool buttonsLocked = false;
  float terrain[TERRAIN_WIDTH][TERRAIN_HEIGHT];

  //fix terrain position
  float terrainFix_x = ((float) TERRAIN_WIDTH) / 2;
  float terrainFix_y = 173.f / 2;

  float pz = 0;
  int frames = 0;
  clock_t start, end;

  float *terrainVerts = 0; //[(TERRAIN_WIDTH - 1) * (TERRAIN_HEIGHT - 1) * 18];
  float *terrainTexts = 0; //[(TERRAIN_WIDTH - 1) * (TERRAIN_HEIGHT - 1) * 12];

// OpenGL ES 2.0 specific (3D model):
  unsigned int shaderProgramID = 0;
  GLint vertexHandle = 0;
  GLint textureCoordHandle = 0;
  GLint mvpMatrixHandle = 0;

  unsigned int terrainShaderProgramID = 0;
  GLint terrainVertexHandle = 0;
  GLint terrainTextureCoordHandle = 0;
  GLint terrainMvpMatrixHandle = 0;

  unsigned int bulletShaderProgramID = 0;
  GLint bulletVertexHandle = 0;
  GLint bulletMvpMatrixHandle = 0;

  unsigned int progressShaderProgramID = 0;
  GLint progressVertexHandle = 0;
  GLint progressMvpMatrixHandle = 0;

// Screen dimensions:
  unsigned int screenWidth = 0;
  unsigned int screenHeight = 0;

// Indicates whether screen is in portrait (true) or landscape (false) mode
  bool isActivityInPortraitMode = false;

  bool shot = false;
// The projection matrix used for rendering virtual objects:
  QCAR::Matrix44F projectionMatrix;

// Constants:
  const float tankScale = 8.f;
  const float bulletScale = 1.5f;

  float tankTurretAngleH[2];
  float tankTurretAngleV[2];

  //terrain collision coords
  int col_x;
  int col_y;
  float col_z;

  enum TanksEnums
  {
    TANK_RED = 0, TANK_BLUE = 1
  };
  int currentTank = TANK_BLUE;
  enum TextureEnums
  {
    TANK_RED_BASE = 0,
    TANK_BLUE_BASE = 1,
    TANK_RED_HEAD = 2,
    TANK_BLUE_HEAD = 3,
    TERRAIN_01 = 4,
    TERRAIN_02 = 5,
    TERRAIN_03 = 6,
    TERRAIN_04 = 7
  };

  const char* virtualButtonNames[] =
    { "left", "right", "up", "down", "change", "fire" };
  const int NUM_BUTTONS = 6;

  JNIEXPORT void JNICALL
  Java_pl_gda_pg_eti_scorchedar_ScorchedAR_setActivityPortraitMode(JNIEnv *, jobject, jboolean isPortrait)
    {
      isActivityInPortraitMode = isPortrait;
    }

  void
  handleButton(const QCAR::VirtualButton* button)
  {
    if (button->isPressed())
      {
        for (int j = 0; j < NUM_BUTTONS; ++j)
          {
            if (strcmp(button->getName(), virtualButtonNames[j]) == 0)
              {

                switch (j)
                  {
                case 0:
                  if (tankTurretAngleH[currentTank] == 360.f)
                    tankTurretAngleH[currentTank] = 0.f;
                  tankTurretAngleH[currentTank] += 1.f;
                  break;
                case 1:
                  if (tankTurretAngleH[currentTank] == 0.0f)
                    tankTurretAngleH[currentTank] = 360.f;
                  tankTurretAngleH[currentTank] -= 1.f;
                  break;
                case 2:
                  if (tankTurretAngleV[currentTank] < 80.f)
                    tankTurretAngleV[currentTank] += 1.f;
                  break;
                case 3:
                  if (tankTurretAngleV[currentTank] > -15.f)
                    tankTurretAngleV[currentTank] -= 1.f;
                  break;
                case 4:

                  break;
                case 5:
                  shot = true;
                  buttonsLocked = true;
                  LOG("fire pressed");

                  break;

                  }

                break;
              }
          }
      }
  }

  void
  updateArrays()
  {
    GLUtils::produceArrays(&terrainVerts[0], &terrainTexts[0], terrain);
  }

  float
  clearTerrainCollisionCoords()
  {
    col_x = 0;
    col_y = 0;
    col_z = 0.0f;
  }

  //tank gravity
  float
  fixTankPosition(int tankNr)
  {
    float z = tanks[tankNr]->getZ();
    float max = 0;
    int tx = (int) tanks[tankNr]->getX() + ((float) TERRAIN_WIDTH) / 2;
    int ty = -(int) tanks[tankNr]->getY() + 173.f / 2;
    LOG("tx ty tank:  %d %d %d", tx, ty, tankNr);
    for (int b = -3; b < 4; b++)
      for (int a = -3; a < 4; a++)
        {
          if (*&terrain[tx + a][ty + b] > max)
            max = *&terrain[tx + a][ty + b];
        }
    LOG("z i max:  %7.3f %7.3f", z, max);
    if (z > max)
      tanks[tankNr]->setZ(max);
    //returns difference
    return z - max;
  }

  void
  tanksGravityFix()
  {
    for (int i = 0; i < 2; i++)
      {
        float zdiff = fixTankPosition(i);
        if (zdiff > 0)
          bullet[i]->setZ(zdiff);
      }
  }

  void
  resetShot(int tank, bool terrainCollision)
  {
    shot = false;

    float npx = tanks[tank]->getX();
    float npy = tanks[tank]->getY();
    float npz = tanks[tank]->getZ();
    //destroy terrain and reload arrays
    if (terrainCollision)
      {
        GLUtils::destroyTerrain(terrain, col_x, col_y, col_z);
        clearTerrainCollisionCoords();
        updateArrays();
        //fix tanks positions in case terrain was destroyed under any of them
        tanksGravityFix();
      }
    //create new bullet for tank
    bullet[tank] = new Bullet(npx, npy, npz + 4.f, tankTurretAngleV[tank],
        tankTurretAngleH[tank], 35.0f, bulletScale);

  }

  JNIEXPORT long JNICALL
  Java_pl_gda_pg_eti_scorchedar_ScorchedARRenderer_renderFrame(JNIEnv *, jobject)
    {

      long wasTracked = 0;
      // Clear color and depth buffer
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Render video background:
      QCAR::State state = QCAR::Renderer::getInstance().begin();

      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);

      // checking if trackable is found
      if (state.getNumActiveTrackables())
        {
          wasTracked = 1;
          // Get the trackable:
          const QCAR::Trackable* trackable = state.getActiveTrackable(0);
          QCAR::Matrix44F modelViewMatrix =
          QCAR::Tool::convertPose2GLMatrix(trackable->getPose());

          // The image target:
          assert(trackable->getType() == QCAR::Trackable::IMAGE_TARGET);
          const QCAR::ImageTarget* target =
          static_cast<const QCAR::ImageTarget*>(trackable);

          // Set transformations:
          QCAR::Matrix44F modelViewProjection;
          GLUtils::multiplyMatrix(&projectionMatrix.data[0],
              &modelViewMatrix.data[0],
              &modelViewProjection.data[0]);

          // Iterate through virtual buttons:
          if (!buttonsLocked)
            {
              for (int i = 0; i < target->getNumVirtualButtons(); ++i)
                {
                  const QCAR::VirtualButton* button = target->getVirtualButton(i);
                  //Buttons handling
                  handleButton(button);
                }
            }
          // We only render if there is something on the array
          glEnableVertexAttribArray(vertexHandle);

          glEnableVertexAttribArray(textureCoordHandle);

          int tutTexIndex = 0;

          // Assumptions:
          assert(tutTexIndex < textureCount);
          const Texture* const redBaseTexture = textures[TANK_RED_BASE];
          const Texture* const blueBaseTexture = textures[TANK_BLUE_BASE];
          const Texture* const redHeadTexture = textures[TANK_RED_HEAD];
          const Texture* const blueHeadTexture = textures[TANK_BLUE_HEAD];

          const Texture* const terrainTexture = textures[terrainTextureIndex];

          QCAR::Matrix44F modelViewProjectionScaled;

          tanks[TANK_RED]->setTurretAngle(tankTurretAngleH[TANK_RED]);
          tanks[TANK_RED]->setBarrelAngle(tankTurretAngleV[TANK_RED]);

          glUseProgram(shaderProgramID);
          tanks[TANK_RED]->render(trackable, &projectionMatrix, &modelViewProjectionScaled, vertexHandle,
              textureCoordHandle, mvpMatrixHandle,
              redBaseTexture->mTextureID,redHeadTexture->mTextureID);

          tanks[TANK_BLUE]->setTurretAngle(tankTurretAngleH[TANK_BLUE]);
          tanks[TANK_BLUE]->setBarrelAngle(tankTurretAngleV[TANK_BLUE]);

          glUseProgram(shaderProgramID);
          tanks[TANK_BLUE]->render(trackable, &projectionMatrix, &modelViewProjectionScaled, vertexHandle,
              textureCoordHandle, mvpMatrixHandle,
              blueBaseTexture->mTextureID,blueHeadTexture->mTextureID);

          glUseProgram(bulletShaderProgramID);
          bullet[TANK_BLUE]->setAngles(tankTurretAngleV[TANK_BLUE], tankTurretAngleH[TANK_BLUE]);
          bullet[TANK_BLUE]->render(trackable, &projectionMatrix, &modelViewProjectionScaled, bulletVertexHandle,
              bulletMvpMatrixHandle);

          bullet[TANK_RED]->setAngles(tankTurretAngleV[TANK_RED], tankTurretAngleH[TANK_RED]);
          bullet[TANK_RED]->render(trackable, &projectionMatrix, &modelViewProjectionScaled, bulletVertexHandle,
              bulletMvpMatrixHandle);

          if (shot)
            {
              bullet[currentTank]->printPosition();
              if (!GLUtils::checkOutOfMap(bullet[currentTank]))
                {
                  if (!GLUtils::checkTerrainCollision(terrain, bullet[currentTank], col_x, col_y, col_z))
                    {
                      bullet[currentTank]->proceed(0.1f);
                    }
                  else
                    {
                      LOG("collision detected");
                      resetShot(currentTank, true);
                      currentTank = 1- currentTank;
                      buttonsLocked = false;
                    }
                }
              else
                {
                  LOG("bullet out of map");
                  resetShot(currentTank, false);
                  currentTank = 1- currentTank;
                  buttonsLocked = false;
                }
            }

          //draw terrain
          glUseProgram(terrainShaderProgramID);
          int terrainNumVerts = 2 * TERRAIN_WIDTH * (TERRAIN_HEIGHT - 1);

          modelViewMatrix =
          QCAR::Tool::convertPose2GLMatrix(trackable->getPose());

          GLUtils::translatePoseMatrix(-terrainFix_x, terrainFix_y, 0.0f,
              &modelViewMatrix.data[0]);

          GLUtils::multiplyMatrix(&projectionMatrix.data[0], &modelViewMatrix.data[0],
              &modelViewProjectionScaled.data[0]);

          glVertexAttribPointer(terrainVertexHandle, 3, GL_FLOAT, GL_FALSE, 0,
              (const GLvoid*) &terrainVerts[0]);

          glVertexAttribPointer(terrainTextureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
              (const GLvoid*) &terrainTexts[0]);

          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, terrainTexture->mTextureID);

          glUniformMatrix4fv(terrainMvpMatrixHandle, 1, GL_FALSE,
              (GLfloat*) &modelViewProjectionScaled.data[0]);

          glDrawArrays(GL_TRIANGLE_STRIP, 0, terrainNumVerts);

          GLUtils::checkGlError("ScorchedAR: renderFrame");
        }

      glDisable(GL_DEPTH_TEST);

      glDisableVertexAttribArray(vertexHandle);

      glDisableVertexAttribArray(textureCoordHandle);
      glDisableVertexAttribArray(terrainVertexHandle);
      glDisableVertexAttribArray(terrainTextureCoordHandle);

      QCAR::Renderer::getInstance().end();
      frames++;
      if (frames == 100)
        {
          end = clock();
          float b = (float(end - start) /100.f);
          LOG("s: %7.3f     %d", b / 100.f, frames);
          float a = (100.f / (b /100.f));
          LOG("fps: %7.3f ", a);
          frames=0; start = clock();
        }

      return wasTracked;
    }

  void
  configureVideoBackground()
  {
    // Get the default video mode:
    QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();
    QCAR::VideoMode videoMode = cameraDevice.getVideoMode(
        QCAR::CameraDevice::MODE_DEFAULT);

    // Configure the video background
    QCAR::VideoBackgroundConfig config;
    config.mEnabled = true;
    config.mSynchronous = true;
    config.mPosition.data[0] = 0.0f;
    config.mPosition.data[1] = 0.0f;

    if (isActivityInPortraitMode)
      {
        //LOG("configureVideoBackground PORTRAIT");
        config.mSize.data[0] = videoMode.mHeight
            * (screenHeight / (float) videoMode.mWidth);
        config.mSize.data[1] = screenHeight;
      }
    else
      {
        //LOG("configureVideoBackground LANDSCAPE");
        config.mSize.data[0] = screenWidth;
        config.mSize.data[1] = videoMode.mHeight
            * (screenWidth / (float) videoMode.mWidth);
      }

    // Set the config:
    QCAR::Renderer::getInstance().setVideoBackgroundConfig(config);
  }

  void
  gameInit()
  {
    clearTerrainCollisionCoords();
    tankTurretAngleH[TANK_RED] = 20.0f;
    tankTurretAngleV[TANK_RED] = 0.0f;
    tankTurretAngleH[TANK_BLUE] = 200.0f;
    tankTurretAngleV[TANK_BLUE] = 0.0f;
    buttonsLocked = false;
    //load Tanks Objects
    tanks = new Tank*[2];

    terrainVerts = new float[6 * TERRAIN_WIDTH * (TERRAIN_HEIGHT - 1)];
    terrainTexts = new float[(TERRAIN_WIDTH - 1) * (TERRAIN_HEIGHT - 1) * 8];
    GLUtils::generateHeightMap(terrain, 45.0f);

    tanks[TANK_RED] = new Tank(100.0f, 60.0f, 70.f, tankTurretAngleH[TANK_RED],
        tankScale);
    tanks[TANK_BLUE] = new Tank(-100.f, -25.f, 70.f,
        tankTurretAngleH[TANK_BLUE], tankScale);

    bullet = new Bullet*[2];

    tanksGravityFix();
    resetShot(TANK_BLUE, false);
    resetShot(TANK_RED, false);
    srand((unsigned) time(NULL));

    terrainTextureIndex = 4 + (rand() % 4);
    updateArrays();
    start = clock();
    frames = 0;
    shot = false;
  }

  void
  gameDestoy()
  {
    delete[] tanks;
    delete[] terrainTexts;
    delete[] terrainVerts;
    delete[] bullet;

  }

  JNIEXPORT void JNICALL
  Java_pl_gda_pg_eti_scorchedar_ScorchedAR_initApplicationNative(
      JNIEnv* env, jobject obj, jint width, jint height)
    {
      LOG("Java_pl_gda_pg_eti_scorchedar_ScorchedAR_initApplicationNative");

      // Store screen dimensions
      screenWidth = width;
      screenHeight = height;

      // Handle to the activity class:
      jclass activityClass = env->GetObjectClass(obj);

      jmethodID getTextureCountMethodID = env->GetMethodID(activityClass,
          "getTextureCount", "()I");
      if (getTextureCountMethodID == 0)
        {
          LOG("Function getTextureCount() not found.");
          return;
        }

      textureCount = env->CallIntMethod(obj, getTextureCountMethodID);
      if (!textureCount)
        {
          LOG("getTextureCount() returned zero.");
          return;
        }

      textures = new Texture*[textureCount];

      jmethodID getTextureMethodID = env->GetMethodID(activityClass,
          "getTexture", "(I)Lpl/gda/pg/eti/scorchedar/Texture;");

      if (getTextureMethodID == 0)
        {
          LOG("Function getTexture() not found.");
          return;
        }

      // Register the textures
      for (int i = 0; i < textureCount; ++i)
        {

          jobject textureObject = env->CallObjectMethod(obj, getTextureMethodID, i);
          if (textureObject == NULL)
            {
              LOG("GetTexture() returned zero pointer");
              return;
            }

          textures[i] = Texture::create(env, textureObject);
        }
      gameInit();
    }

  JNIEXPORT void JNICALL
  Java_pl_gda_pg_eti_scorchedar_ScorchedAR_deinitApplicationNative(
      JNIEnv* env, jobject obj)
    {
      LOG("Java_pl_gda_pg_eti_scorchedar_ScorchedAR_deinitApplicationNative");

      // Release texture resources
      if (textures != 0)
        {
          for (int i = 0; i < textureCount; ++i)
            {
              delete textures[i];
              textures[i] = NULL;
            }

          delete[]textures;
          textures = NULL;

          textureCount = 0;
        }
    }

  JNIEXPORT void JNICALL
  Java_pl_gda_pg_eti_scorchedar_ScorchedAR_startCamera(JNIEnv *,
      jobject)
    {
      LOG("Java_pl_gda_pg_eti_scorchedar_ScorchedAR_startCamera");

      // Initialize the camera:
      if (!QCAR::CameraDevice::getInstance().init())
      return;

      // Configure the video background
      configureVideoBackground();

      // Select the default mode:
      if (!QCAR::CameraDevice::getInstance().selectVideoMode(
              QCAR::CameraDevice::MODE_DEFAULT))
      return;

      // Start the camera:
      if (!QCAR::CameraDevice::getInstance().start())
      return;

      // Start the tracker:
      QCAR::Tracker::getInstance().start();

      // Cache the projection matrix:
      const QCAR::Tracker& tracker = QCAR::Tracker::getInstance();
      const QCAR::CameraCalibration& cameraCalibration =
      tracker.getCameraCalibration();
      projectionMatrix = QCAR::Tool::getProjectionGL(cameraCalibration, 2.0f,
          2000.0f);
    }

  JNIEXPORT void JNICALL
  Java_pl_gda_pg_eti_scorchedar_ScorchedAR_stopCamera(JNIEnv *,
      jobject)
    {
      LOG("Java_pl_gda_pg_eti_scorchedar_ScorchedAR_stopCamera");

      QCAR::Tracker::getInstance().stop();

      QCAR::CameraDevice::getInstance().stop();
      QCAR::CameraDevice::getInstance().deinit();
    }

  JNIEXPORT jboolean JNICALL
  Java_pl_gda_pg_eti_scorchedar_ScorchedAR_toggleFlash(JNIEnv*, jobject, jboolean flash)
    {
      return QCAR::CameraDevice::getInstance().setFlashTorchMode((flash==JNI_TRUE)) ? JNI_TRUE : JNI_FALSE;
    }

  JNIEXPORT jboolean JNICALL
  Java_pl_gda_pg_eti_scorchedar_ScorchedAR_autofocus(JNIEnv*, jobject)
    {
      return QCAR::CameraDevice::getInstance().startAutoFocus()?JNI_TRUE:JNI_FALSE;
    }

  JNIEXPORT jboolean JNICALL
  Java_pl_gda_pg_eti_scorchedar_ScorchedAR_setFocusMode(JNIEnv*, jobject, jint mode)
    {
      return QCAR::CameraDevice::getInstance().setFocusMode(mode)?JNI_TRUE:JNI_FALSE;
    }

  JNIEXPORT void JNICALL
  Java_pl_gda_pg_eti_scorchedar_ScorchedARRenderer_initRendering(
      JNIEnv* env, jobject obj)
    {
      LOG("Java_pl_gda_pg_eti_scorchedar_ScorchedARRenderer_initRendering");

      // Define clear color
      glClearColor(0.0f, 0.0f, 0.0f, QCAR::requiresAlpha() ? 0.0f : 1.0f);

      // Now generate the OpenGL texture objects and add settings
      for (int i = 0; i < textureCount; ++i)
        {
          glGenTextures(1, &(textures[i]->mTextureID));
          glBindTexture(GL_TEXTURE_2D, textures[i]->mTextureID);
          glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textures[i]->mWidth,
              textures[i]->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
              (GLvoid*) textures[i]->mData);
        }

      // OpenGL setup for 3D model
      shaderProgramID = GLUtils::createProgramFromBuffer(tankVertexShader,
          tankFragmentShader);

      vertexHandle = glGetAttribLocation(shaderProgramID,
          "vertexPosition");
      textureCoordHandle = glGetAttribLocation(shaderProgramID,
          "vertexTexCoord");
      mvpMatrixHandle = glGetUniformLocation(shaderProgramID,
          "modelViewProjectionMatrix");

      //openGL setup for terrain
      terrainShaderProgramID = GLUtils::createProgramFromBuffer(terrainVertexShader,
          terrainFragmentShader);

      terrainVertexHandle = glGetAttribLocation(terrainShaderProgramID,
          "vertexPosition");
      terrainTextureCoordHandle = glGetAttribLocation(terrainShaderProgramID,
          "vertexTexCoord");
      terrainMvpMatrixHandle = glGetUniformLocation(terrainShaderProgramID,
          "modelViewProjectionMatrix");

      bulletShaderProgramID = GLUtils::createProgramFromBuffer(bulletVertexShader, bulletFragmentShader);
      bulletVertexHandle = glGetAttribLocation(bulletShaderProgramID,
          "vertexPosition");
      bulletMvpMatrixHandle = glGetUniformLocation(bulletShaderProgramID,
          "modelViewProjectionMatrix");

      progressShaderProgramID = GLUtils::createProgramFromBuffer(progressVertexShader, progressFragmentShader);
      progressVertexHandle = glGetAttribLocation(progressShaderProgramID,
          "vertexPosition");
      progressMvpMatrixHandle = glGetUniformLocation(progressShaderProgramID,
          "modelViewProjectionMatrix");
    }

  JNIEXPORT void JNICALL
Java_pl_gda_pg_eti_scorchedar_ScorchedARRenderer_updateRendering(
    JNIEnv* env, jobject obj, jint width, jint height)
  {
    LOG("Java_pl_gda_pg_eti_scorchedar_ScorchedARRenderer_updateRendering");

    // Update screen dimensions
    screenWidth = width;
    screenHeight = height;

    // Reconfigure the video background
    configureVideoBackground();
  }

#ifdef __cplusplus
}
#endif
