#include <cstdio>
#include <cstdlib>
#if defined(WIN32)
//#  pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#  pragma comment(lib, "glew32.lib")
#  include "glew.h"
#  include "glut.h"
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>
#endif

/*
** 変換行列
*/
#include "Matrix.h"
static Matrix projectionMatrix;
static Matrix viewMatrix;
static GLint modelViewProjectionMatrixLocation;
static GLint modelViewMatrixLocation;
static GLint projectionMatrixLocation;

/*
** ボーン
*/
#include "Bone.h"
#define BONES 2
static Bone bone[BONES];
static GLint numberOfBonesLocation;
static GLint boneBottomLocation;
static GLint boneTopLocation;
static GLint blendMatrixLocation;

/*
** 点
*/
#include "disseminate.h"
#define POINTS 5000
static GLuint buffer[1];
static GLsizei points;

/*
** シェーダ
*/
#include "shadersource.h"
static const char bVertSource[] = "bone.vert";
static const char bFragSource[] = "bone.frag";
static GLuint bProgram;
static const char pVertSource[] = "simple.vert";
static const char pFragSource[] = "simple.frag";
static GLuint pProgram;

/*
** アニメーション用データ
*/
static int cx, cy;
static int lButton = GLUT_UP;
static int rButton = GLUT_UP;
static float angle[2] = { 0.0f, 0.0f };

/*
** ボーンの描画
*/
static void drawBone(const Bone *b, float *bottom, float *top, float *blend)
{
  /* ボーンの図形データ */
  static const GLfloat boneVertex[][4] = {
    {  0.0f,  0.0f,  0.0f,  1.0f },
    {  0.1f,  0.0f,  0.1f,  1.0f },
    {  0.0f,  0.1f,  0.1f,  1.0f },
    { -0.1f,  0.0f,  0.1f,  1.0f },
    {  0.0f, -0.1f,  0.1f,  1.0f },
    {  0.0f,  0.0f,  1.0f,  1.0f },
  };
  static const GLuint boneEdge[] = {
    0, 1, 5, 3, 0, 2, 5, 4, 1, 2, 3, 4,
  };

  /* ボーンの長さに合わせてスケーリングする変換行列 */
  Matrix scale;
  scale.loadScale(b->getLength(), b->getLength(), b->getLength());
  
  /* ボーンを初期位置に配置する変換行列とアニメーション後の変換行列 */
  Matrix initial, animated;
  initial.loadIdentity();
  animated.loadIdentity();

  /* 現在の視野変換行列をかけておく */
  do {
    Matrix temp;
    temp.loadTranslate(b->getPosition());
    temp.rotate(b->getRotation());
    initial = temp * initial;
    temp.multiply(b->getAnimation());
    animated = temp * animated;
  }
  while ((b = b->getParent()) != 0);
  
  /* ボーンの初期位置における根元と先端の位置を求める */
  initial = viewMatrix * initial;
  animated = viewMatrix * animated;

  /* ボーンの初期位置における根元と先端の位置を求める */
  initial.projection(bottom, boneVertex[0]);
  (initial * scale).projection(top, boneVertex[5]);

  /* バーテックスブレンディング用の変換行列 */
  memcpy(blend, (animated * initial.invert()).get(), sizeof blend[0] * 16);

  /* ボーンを描画する */
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, boneVertex);
  glUniformMatrix4fv(modelViewProjectionMatrixLocation, 1, GL_FALSE, (projectionMatrix * animated * scale).get());
  glDrawElements(GL_LINE_LOOP, sizeof boneEdge / sizeof boneEdge[0], GL_UNSIGNED_INT, boneEdge);
  glDisableVertexAttribArray(0);
}

/*
** 画面表示
*/
static void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  
  /* バーテックスブレンディング用データ */
  GLfloat bottom[BONES][4], top[BONES][4], blend[BONES][16];

  /*
  ** ボーンのアニメーション
  */
  glUseProgram(bProgram);

  for (int i = 0; i < BONES; ++i) {
    Matrix animationMatrix;
    animationMatrix.loadRotate(0.0f, 1.0f, 0.0f, angle[i]);
    bone[i].setAnimation(animationMatrix.get());
    drawBone(&bone[i], bottom[i], top[i], blend[i]);
  }

  /*
  ** 点を描く
  */
  glUseProgram(pProgram);
  
  /* 点のモデリング変換／視野変換／投影変換 */
  Matrix modelViewMatrix = viewMatrix;
  modelViewMatrix.translate(0.0f, 0.0f, -1.5f);
  modelViewMatrix.scale(0.3f, 0.3f, 3.0f);
  glUniformMatrix4fv(modelViewMatrixLocation, 1, GL_FALSE, modelViewMatrix.get());
  glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, projectionMatrix.get());

  /* バーテックスブレンディング用の uniform 変数の設定 */
  glUniform1i(numberOfBonesLocation, BONES);
  glUniform4fv(boneBottomLocation, BONES, bottom[0]);
  glUniform4fv(boneTopLocation, BONES, top[0]);
  glUniformMatrix4fv(blendMatrixLocation, BONES, GL_FALSE, blend[0]);

  /* attribute 変数 position に頂点情報を対応付けて図形を描画する */
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glDrawArrays(GL_POINTS, 0, points);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDisableVertexAttribArray(0);

  glDisable(GL_DEPTH_TEST);
  glutSwapBuffers();
}

/*
** ウィンドウサイズ
*/
static void resize(int w, int h)
{
  /* ウィンドウの中心を求める */
  cx = w / 2;
  cy = h / 2;
  
  /* ウィンドウ全体をビューポートにする */
  glViewport(0, 0, w, h);
  
  /* 透視投影変換行列を求める */
  projectionMatrix.loadIdentity();
  projectionMatrix.camera(30.0f, (GLfloat)w / (GLfloat)h, 5.0f, 9.0f);
}

/*
** アニメーション
*/
void idle(void)
{
  glutPostRedisplay();
}

/*
** マウスのクリック
*/
static void mouse(int button, int state, int x, int y)
{
  switch (button) {
    case GLUT_LEFT_BUTTON:
      lButton = state;
      break;
    case GLUT_RIGHT_BUTTON:
      rButton = state;
      break;
    default:
      break;
  }
  
  if (lButton == GLUT_UP && rButton == GLUT_UP)
    glutIdleFunc(0);
  else
    glutIdleFunc(idle);
}

/*
** マウスのドラッグ
*/
static void motion(int x, int y)
{
  if (lButton == GLUT_DOWN) angle[0] = 180.0f * (float)(x - cx) / (float)cx;
  if (rButton == GLUT_DOWN) angle[1] = 180.0f * (float)(x - cx) / (float)cx;
}

/*
** キーボード
*/
static void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
    /* q か Q か ESC をタイプしたら終了 */
    case '\033':
    case 'q':
    case 'Q':
      exit(0);
    default:
      break;
  }
}

/*
** シェーダプログラムの読み込み
*/
static GLuint loadShader(const char *vertSource, const char *fragSource)
{
  /* シェーダオブジェクトの作成 */
  GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  
  /* シェーダのソースプログラムの読み込み */
  if (readShaderSource(vertShader, vertSource)) return 0;
  if (readShaderSource(fragShader, fragSource)) return 0;
  
  /* シェーダプログラムのコンパイル／リンク結果を得る変数 */
  GLint status;
  
  /* バーテックスシェーダのソースプログラムのコンパイル */
  glCompileShader(vertShader);
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &status);
  printShaderInfoLog(vertShader);
  if (status == GL_FALSE) {
    fprintf(stderr, "Compile error in %s.\n", vertSource);
    return 0;
  }
  
  /* フラグメントシェーダのソースプログラムのコンパイル */
  glCompileShader(fragShader);
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);
  printShaderInfoLog(fragShader);
  if (status == GL_FALSE) {
    fprintf(stderr, "Compile error in %s.\n", fragSource);
    return 0;
  }
  
  /* プログラムオブジェクトの作成 */
  GLuint gl2Program = glCreateProgram();
  
  /* シェーダオブジェクトのシェーダプログラムへの登録 */
  glAttachShader(gl2Program, vertShader);
  glAttachShader(gl2Program, fragShader);
  
  /* シェーダオブジェクトの削除 */
  glDeleteShader(vertShader);
  glDeleteShader(fragShader);
  
  /* attribute 変数 position の index に 0 を指定する */
  glBindAttribLocation(gl2Program, 0, "position");
  
  /* シェーダプログラムのリンク */
  glLinkProgram(gl2Program);
  glGetProgramiv(gl2Program, GL_LINK_STATUS, &status);
  printProgramInfoLog(gl2Program);
  if (status == GL_FALSE) {
    fprintf(stderr, "Link error.\n");
    return 0;
  }
  
  return gl2Program;
}

/*
** 初期化
*/
static void init(void)
{
#if defined(WIN32)
  /* GLEW の初期化 */
  GLenum err = glewInit();
  if (err != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    exit(1);
  }
#endif
  /* シェーダプログラムの読み込み */
  if ((bProgram = loadShader(bVertSource, bFragSource)) == 0) exit(1);
  if ((pProgram = loadShader(pVertSource, pFragSource)) == 0) exit(1);
  
  /* uniform 変数 modelViewProjectionMatrix の場所を得る */
  modelViewProjectionMatrixLocation = glGetUniformLocation(bProgram, "modelViewProjectionMatrix");
  modelViewMatrixLocation = glGetUniformLocation(pProgram, "modelViewMatrix");
  projectionMatrixLocation = glGetUniformLocation(pProgram, "projectionMatrix");

  /* バーテックスブレンディング用の uniform 変数の場所を得る */
  numberOfBonesLocation = glGetUniformLocation(pProgram, "numberOfBones");
  boneBottomLocation = glGetUniformLocation(pProgram, "boneBottom");
  boneTopLocation = glGetUniformLocation(pProgram, "boneTop");
  blendMatrixLocation = glGetUniformLocation(pProgram, "blendMatrix");
  
  /* 視野変換行列を求める */
  viewMatrix.loadIdentity();
  viewMatrix.lookat(0.0f, -7.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
  
  /* 一つ目のボーンの設定 */
  bone[0].setPosition(0.0f, 0.0f, -1.5f);
  bone[0].setRotation(0.0f, 0.0f, 1.0f, 0.0f);
  bone[0].setLength(1.5f);
  bone[0].setParent(0);         // 根元のボーン

  /* 二つ目のボーンの設定 */
  bone[1].setPosition(0.0f, 0.0f, 1.5f);
  bone[1].setRotation(0.0f, 0.0f, 1.0f, 0.0f);
  bone[1].setLength(1.5f);
  bone[1].setParent(&bone[0]);  // bone[0] を親とする

  /* 頂点バッファオブジェクトを１つ作る */
  glGenBuffers(1, buffer);
  
  /* 図形をバッファオブジェクトに登録する */
  points = disseminate(POINTS, buffer);
  
  /* 背景色 */
  glClearColor(0.0, 0.1, 0.3, 1.0);
}

/*
** メインプログラム
*/
int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  init();
  glutMainLoop();
  
  return 0;
}
