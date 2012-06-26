#version 120
//
// simple.vert
//
attribute vec3 position;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
 
void main()
{
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
}
