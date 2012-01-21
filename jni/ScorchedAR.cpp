#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

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
#include "CubeShaders.h"
#include "LineShaders.h"
#include "tankv2.h"
#include "tankTurret.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Textures:
  int textureCount = 0;
  Texture** textures = 0;

// OpenGL ES 2.0 specific (3D model):
  unsigned int shaderProgramID = 0;
  GLint vertexHandle = 0;
  GLint normalHandle = 0;
  GLint textureCoordHandle = 0;
  GLint mvpMatrixHandle = 0;

// Screen dimensions:
  unsigned int screenWidth = 0;
  unsigned int screenHeight = 0;

// Indicates whether screen is in portrait (true) or landscape (false) mode
  bool isActivityInPortraitMode = false;

// The projection matrix used for rendering virtual objects:
  QCAR::Matrix44F projectionMatrix;

// Constants:
  static const float kTeapotScale = 50.f;
  float rotAngle = 0.f;
  float rotAngleH = 0.f;

// Enumeration for masking button indices into single integer:
  enum BUTTONS
  {
    BUTTON_LEFT = 1,
    BUTTON_RIGHT = 2,
    BUTTON_UP = 4,
    BUTTON_DOWN = 8,
    BUTTON_CHANGE = 16,
    BUTTON_FIRE = 32
  };



  const char* virtualButtonNames[] =
    { "left", "right", "up", "down", "change", "fire" };
  const int NUM_BUTTONS = 6;

  JNIEXPORT void JNICALL
  Java_pl_gda_pg_eti_scorchedar_ScorchedAR_setActivityPortraitMode(JNIEnv *, jobject, jboolean isPortrait)
    {
      isActivityInPortraitMode = isPortrait;
    }

  JNIEXPORT void JNICALL
  Java_pl_gda_pg_eti_scorchedar_ScorchedARRenderer_renderFrame(JNIEnv *, jobject)
    {
      //LOG("Java_com_qualcomm_QCARSamples_VirtualButtons_GLRenderer_renderFrame");

      // Clear color and depth buffer
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // Render video background:
      QCAR::State state = QCAR::Renderer::getInstance().begin();

      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);

      // Did we find any trackables this frame?
      if (state.getNumActiveTrackables())
        {
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

          // Iterate through this targets virtual buttons:
          for (int i = 0; i < target->getNumVirtualButtons(); ++i)
            {
              const QCAR::VirtualButton* button = target->getVirtualButton(i);

              //Buttons handling
              if (button->isPressed())
                {
                  for (int j = 0; j < NUM_BUTTONS; ++j)
                    {
                      if (strcmp(button->getName(), virtualButtonNames[j]) == 0)
                        {

                          switch(j)
                            {
                              case 0: rotAngle += 1.f;break;
                              case 1: rotAngle -= 1.f;break;
                              case 2: if(rotAngleH < 80.f) rotAngleH += 1.f;break;
                              case 3: if(rotAngleH > -15.f) rotAngleH -= 1.f;break;
                            }

                          break;
                        }
                    }
                }

            }

          // We only render if there is something on the array

          int tutTexIndex = 0;
          int thisTexIndex = 1;
          // Assumptions:
          assert(tutTexIndex < textureCount);
          const Texture* const thisTexture = textures[tutTexIndex];

          // Scale 3D model
          QCAR::Matrix44F modelViewScaled = modelViewMatrix;

          GLUtils::scalePoseMatrix(kTeapotScale, kTeapotScale, kTeapotScale,
              &modelViewScaled.data[0]);

          //move turret up
          GLUtils::translatePoseMatrix(kTeapotScale * 0.0f, kTeapotScale * 0.0f, kTeapotScale * 0.01f, &modelViewScaled.data[0]);
          //rotate
          GLUtils::rotatePoseMatrix(rotAngle, 0.0f, 0.0f, 1.0f, &modelViewScaled.data[0]);
          GLUtils::rotatePoseMatrix(rotAngleH, 0.0f, 1.0f, 0.0f, &modelViewScaled.data[0]);

          QCAR::Matrix44F modelViewProjectionScaled;
          GLUtils::multiplyMatrix(&projectionMatrix.data[0],
              &modelViewScaled.data[0],
              &modelViewProjectionScaled.data[0]);

          // Render 3D model
          glUseProgram(shaderProgramID);

//turret
          glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) &tankTurretVerts[0]);

          glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
              (const GLvoid*) &tankTurretNormals[0]);

          glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
              (const GLvoid*) &tankTurretTexCoords[0]);

          glEnableVertexAttribArray(vertexHandle);
          glEnableVertexAttribArray(normalHandle);
          glEnableVertexAttribArray(textureCoordHandle);
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);

          glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
              (GLfloat*)&modelViewProjectionScaled.data[0] );

          glDrawArrays(GL_TRIANGLES, 0, tankTurretNumVerts);

//korpus
          modelViewMatrix = QCAR::Tool::convertPose2GLMatrix(trackable->getPose());
          GLUtils::scalePoseMatrix(kTeapotScale, kTeapotScale, kTeapotScale,
              &modelViewMatrix.data[0]);

          GLUtils::multiplyMatrix(&projectionMatrix.data[0],
              &modelViewMatrix.data[0],
              &modelViewProjectionScaled.data[0]);

          // Render 3D model
          glUseProgram(shaderProgramID);

          glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) &tankv2Verts[0]);

          glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0,
              (const GLvoid*) &tankv2Normals[0]);

          glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0,
              (const GLvoid*) &tankv2TexCoords[0]);

          glEnableVertexAttribArray(vertexHandle);
          glEnableVertexAttribArray(normalHandle);
          glEnableVertexAttribArray(textureCoordHandle);
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, thisTexture->mTextureID);

          glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE,
              (GLfloat*)&modelViewProjectionScaled.data[0] );

          glDrawArrays(GL_TRIANGLES, 0, tankv2NumVerts);

          GLUtils::checkGlError("VirtualButtons renderFrame");

        }

      glDisable(GL_DEPTH_TEST);

      glDisableVertexAttribArray(vertexHandle);
      glDisableVertexAttribArray(normalHandle);
      glDisableVertexAttribArray(textureCoordHandle);

      QCAR::Renderer::getInstance().end();
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
      shaderProgramID = GLUtils::createProgramFromBuffer(cubeMeshVertexShader,
          cubeFragmentShader);

      vertexHandle = glGetAttribLocation(shaderProgramID,
          "vertexPosition");
      normalHandle = glGetAttribLocation(shaderProgramID,
          "vertexNormal");
      textureCoordHandle = glGetAttribLocation(shaderProgramID,
          "vertexTexCoord");
      mvpMatrixHandle = glGetUniformLocation(shaderProgramID,
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