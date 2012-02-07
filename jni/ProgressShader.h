#ifndef _PROGRESS_SHADER_H_
#define _PROGRESS_SHADER_H_

#ifndef USE_OPENGL_ES_1_1

static const char* progressVertexShader = " \
  \
attribute vec4 vertexPosition; \
\
uniform mat4 modelViewProjectionMatrix; \
uniform float progress; \
varying float progr; \
 \
void main() \
{ \
   gl_Position = modelViewProjectionMatrix * vertexPosition; \
   progr = progress;\
} \
";


static const char* progressFragmentShader = " \
 \
 precision mediump float; \
 \
void main() \
{ \
    gl_FragColor= vec4(1.0, 0.0, 0.0, 1.0); \
} \
";


#endif

#endif
