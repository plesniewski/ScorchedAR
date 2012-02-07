#ifndef _BULLET_SHADER_H_
#define _BULLET_SHADER_H_

#ifndef USE_OPENGL_ES_1_1

static const char* bulletVertexShader = " \
  \
attribute vec4 vertexPosition; \
\
uniform mat4 modelViewProjectionMatrix; \
 \
void main() \
{ \
   gl_Position = modelViewProjectionMatrix * vertexPosition; \
} \
";


static const char* bulletFragmentShader = " \
 \
 \
void main() \
{ \
   gl_FragColor= vec4(1.0, 1.0, 0.0, 1.0); \
} \
";


#endif

#endif
