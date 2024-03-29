/*
created with obj2opengl.pl

source file    : ./tankv2.obj
vertices       : 8
faces          : 12
normals        : 6
texture coords : 14


// include generated arrays
#import "./tankv2.h"

// set input data to arrays
glVertexPointer(3, GL_FLOAT, 0, tankv2Verts);
glNormalPointer(GL_FLOAT, 0, tankv2Normals);
glTexCoordPointer(2, GL_FLOAT, 0, tankv2TexCoords);

// draw data
glDrawArrays(GL_TRIANGLES, 0, tankv2NumVerts);
*/

unsigned int tankBaseNumVerts = 36;

float tankBaseVerts [] = {
  // f 1/1/1 2/2/1 3/3/1
  1.25, -0.6, 0,
  -1.25, -0.6, 0,
  -1.25, 0.6, 0,
  // f 1/1/1 3/3/1 4/4/1
  1.25, -0.6, 0,
  -1.25, 0.6, 0,
  1.25, 0.6, 0,
  // f 5/5/2 8/6/2 7/7/2
  1.0375, -0.5, 0.4,
  1.0375, 0.5, 0.4,
  -0.7875, 0.5, 0.4,
  // f 5/5/2 7/7/2 6/8/2
  1.0375, -0.5, 0.4,
  -0.7875, 0.5, 0.4,
  -0.7875, -0.5, 0.4,
  // f 1/9/3 5/5/3 6/8/3
  1.25, -0.6, 0,
  1.0375, -0.5, 0.4,
  -0.7875, -0.5, 0.4,
  // f 1/9/3 6/8/3 2/10/3
  1.25, -0.6, 0,
  -0.7875, -0.5, 0.4,
  -1.25, -0.6, 0,
  // f 2/11/4 6/8/4 7/7/4
  -1.25, -0.6, 0,
  -0.7875, -0.5, 0.4,
  -0.7875, 0.5, 0.4,
  // f 2/11/4 7/7/4 3/12/4
  -1.25, -0.6, 0,
  -0.7875, 0.5, 0.4,
  -1.25, 0.6, 0,
  // f 3/3/5 7/7/5 8/6/5
  -1.25, 0.6, 0,
  -0.7875, 0.5, 0.4,
  1.0375, 0.5, 0.4,
  // f 3/3/5 8/6/5 4/4/5
  -1.25, 0.6, 0,
  1.0375, 0.5, 0.4,
  1.25, 0.6, 0,
  // f 5/5/6 1/13/6 4/14/6
  1.0375, -0.5, 0.4,
  1.25, -0.6, 0,
  1.25, 0.6, 0,
  // f 5/5/6 4/14/6 8/6/6
  1.0375, -0.5, 0.4,
  1.25, 0.6, 0,
  1.0375, 0.5, 0.4,
};


float tankBaseTexCoords [] = {
  // f 1/1/1 2/2/1 3/3/1
  1.000000, 0.920505,
  1.000000, 0.093955,
  0.603256, 0.093955,
  // f 1/1/1 3/3/1 4/4/1
  1.000000, 0.920505,
  0.603256, 0.093955,
  0.603256, 0.920505,
  // f 5/5/2 8/6/2 7/7/2
  0.136318, 0.850249,
  0.466938, 0.850248,
  0.466938, 0.246867,
  // f 5/5/2 7/7/2 6/8/2
  0.136318, 0.850249,
  0.466938, 0.246867,
  0.136318, 0.246867,
  // f 1/9/3 5/5/3 6/8/3
  0.000000, 0.920505,
  0.136318, 0.850249,
  0.136318, 0.246867,
  // f 1/9/3 6/8/3 2/10/3
  0.000000, 0.920505,
  0.136318, 0.246867,
  0.000000, 0.093956,
  // f 2/11/4 6/8/4 7/7/4
  0.103256, 0.0447,
  0.136318, 0.246867,
  0.466938, 0.246867,
  // f 2/11/4 7/7/4 3/12/4
  0.103256, 0.0447,
  0.466938, 0.246867,
  0.500000, 0.0447,
  // f 3/3/5 7/7/5 8/6/5
  0.603256, 0.093955,
  0.466938, 0.246867,
  0.466938, 0.850248,
  // f 3/3/5 8/6/5 4/4/5
  0.603256, 0.093955,
  0.466938, 0.850248,
  0.603256, 0.920505,
  // f 5/5/6 1/13/6 4/14/6
  0.136318, 0.850249,
  0.103257, 1,
  0.500000, 1,
  // f 5/5/6 4/14/6 8/6/6
  0.136318, 0.850249,
  0.500000, 1,
  0.466938, 0.850248,
};

