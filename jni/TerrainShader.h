

#ifndef _TERRAIN_SHADERS_H_
#define _TERRAIN_SHADERS_H_

#ifndef USE_OPENGL_ES_1_1

static const char* terrainVertexShader = " \
  \
attribute vec4 vertexPosition; \
attribute vec4 vertexNormal; \
attribute vec2 vertexTexCoord; \
 \
varying vec2 texCoord; \
varying vec4 normal; \
varying vec4 vert; \
\
uniform mat4 modelViewProjectionMatrix; \
 \
void main() \
{ \
   gl_Position = modelViewProjectionMatrix * vertexPosition; \
   vert = vertexPosition; \
   texCoord = vertexTexCoord; \
} \
";


static const char* terrainFragmentShader = " \
 \
precision mediump float; \
 \
varying vec2 texCoord; \
varying vec4 vert; \
 \
uniform sampler2D texSampler2D; \
 \
void main() \
{ \
   vec3 normLight = vec3(0.0,0.0,1.0); \
   float factor =  vert.z / 60.0; \
   vec3 color = vec3(texture2D(texSampler2D, texCoord));\
   gl_FragColor= vec4(color.x * factor, color.y * factor, color.z * factor, 1.0); \
} \
";


#endif

#endif
