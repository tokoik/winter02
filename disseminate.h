/*
** 点を空間に散布する
*/
#ifndef DISSEMINATE_H
#define DISSEMINATE_H

#if defined(__APPLE__) || defined(MACOSX)
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

extern GLuint disseminate(int points, const GLuint *buffer);

#endif
