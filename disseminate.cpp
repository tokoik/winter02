//
// disseminate.cpp
//
/*
** 点を空間に散布する
*/
#if defined(_WIN32)
#  define _USE_MATH_DEFINES
#  define _CRT_SECURE_NO_WARNINGS
#  include <GL/glew.h>
#  include <GL/glut.h>
#elif defined(__APPLE__) || defined(MACOSX)
#  define GL_SILENCE_DEPRECATION
#  include <GLUT/glut.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>
#endif
#include <cstdlib>
#include <cmath>

#include "disseminate.h"

/* 頂点バッファオブジェクトのメモリを参照するポインタのデータ型 */
typedef GLfloat Point[3];

/*
** 点を空間に散布する
*/
GLuint disseminate(int points, const GLuint *buffer)
{
  /* 頂点バッファオブジェクトを有効にする */
  glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);

  /* 頂点バッファオブジェクトにメモリ領域を確保する */
  glBufferData(GL_ARRAY_BUFFER, sizeof (Point) * points, NULL, GL_STATIC_DRAW);

  /* 頂点バッファオブジェクトのメモリをプログラムのメモリ空間にマップする */
  Point *point = (Point *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

  /* 頂点の位置 */
  for (int i = 0; i < points; ++i) {
    float r = sqrtf((float)rand() / ((float)RAND_MAX + 1.0f));
    float t = 6.283185f * (float)rand() / ((float)RAND_MAX + 1.0f);
    (*point)[0] = r * cosf(t);
    (*point)[1] = r * sinf(t);
    (*point)[2] = (float)rand() / ((float)RAND_MAX + 1.0f);
    ++point;
  }

  /* 頂点バッファオブジェクトのメモリをプログラムのメモリ空間から切り離す */
  glUnmapBuffer(GL_ARRAY_BUFFER);

  /* 頂点バッファオブジェクトを解放する */
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  return points;
}
