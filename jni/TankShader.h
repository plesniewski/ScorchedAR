#ifndef _TANK_SHADER_H_
#define _TANK_SHADER_H_

#ifndef USE_OPENGL_ES_1_1

static const char* tankVertexShader = " \
  \
attribute vec4 vertexPosition; \
attribute vec2 vertexTexCoord; \
 \
varying vec2 texCoord; \
\
uniform mat4 modelViewProjectionMatrix; \
 \
void main() \
{ \
   gl_Position = modelViewProjectionMatrix * vertexPosition; \
   texCoord = vertexTexCoord; \
} \
";


static const char* tankFragmentShader = " \
 \
precision mediump float; \
 \
varying vec2 texCoord; \
 \
uniform sampler2D texSampler2D; \
 \
void main() \
{ \
   gl_FragColor= texture2D(texSampler2D, texCoord); \
} \
";


#endif

#endif
