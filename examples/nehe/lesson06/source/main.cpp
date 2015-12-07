/****************************************
 *       3DS NeHe Lesson 06            *
 *       Author: machinamentum         *
 ****************************************/

#include <3ds.h>
#include <cmath>
#include <GL/gl.h>
#include <gfx_device.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint LoadPNG(const char *FileName) {
  int width, height, comp;
  unsigned char *data = stbi_load(FileName, &width, &height, &comp, 4); // 4 forces RGBA components / 4 bytes-per-pixel
  if (data) {
    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
    stbi_image_free(data);
    return texId;
  }

  return 0;
}

int Textures[1];
GLfloat     xrot;                               // X Rotation ( NEW )
GLfloat     yrot;                               // Y Rotation ( NEW )
GLfloat     zrot;                               // Z Rotation ( NEW )

int LoadGLTextures() {
  Textures[0] = LoadPNG("RTS_Crate.png"); // http://opengameart.org/content/2d-wooden-box
  return Textures[0] != 0;
}

void DrawGLScene() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);         // Clear Screen And Depth Buffer
  glLoadIdentity();                           // Reset The Current Matrix
  glTranslatef(0.0f,0.0f,-5.0f);                      // Move Into The Screen 5 Units

  glRotatef(xrot,1.0f,0.0f,0.0f);                     // Rotate On The X Axis
  glRotatef(yrot,0.0f,1.0f,0.0f);                     // Rotate On The Y Axis
  glRotatef(zrot,0.0f,0.0f,1.0f);                     // Rotate On The Z Axis

  glBindTexture(GL_TEXTURE_2D, Textures[0]);               // Select Our Texture

  glBegin(GL_QUADS);
  // Front Face
  glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
  glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
  glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
  glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
  // Back Face
  glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
  glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
  glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
  glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
  // Top Face
  glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
  glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
  glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
  glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
  // Bottom Face
  glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Right Of The Texture and Quad
  glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
  glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
  glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
  // Right face
  glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
  glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
  glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
  glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
  // Left Face
  glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
  glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
  glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
  glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
  glEnd();

  xrot+=0.3f;                             // X Axis Rotation
  yrot+=0.2f;                             // Y Axis Rotation
  zrot+=0.4f;                             // Z Axis Rotation
}

int InitGL() {
  if (!LoadGLTextures())                          // Jump To Texture Loading Routine ( NEW )
  {
    return false;                           // If Texture Didn't Load Return FALSE ( NEW )
  }

  glEnable(GL_TEXTURE_2D);                        // Enable Texture Mapping ( NEW )
  glClearColor(0.0f, 0.0f, 0.0f, 0.5f);                   // Black Background
  glClearDepth(1.0f);                         // Depth Buffer Setup
  glEnable(GL_DEPTH_TEST);                        // Enables Depth Testing
  glDepthFunc(GL_LEQUAL);                         // The Type Of Depth Testing To Do
  return true;                                // Initialization Went OK
}

int main()
{
  gfxInitDefault();
  hidInit();
  romfsInit();

  void* device = gfxCreateDevice(240, 400, 0);
  gfxMakeCurrent(device);

  glViewport(0, 0, 240, 400);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  float near = 0.1f;
  float far = 100.0f;
  float fov = 90.0f;
  float aspect = 240.0f / 400.0f;
  float t = tan(fov * 3.14159 / 360.0) * near;
  float b = -t;
  float l = aspect * b;
  float r = aspect * t;
  glFrustum(l, r, b, t, near, far);
  //3DS' framebuffers are sideways
  glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  InitGL();

  while (aptMainLoop())
  {
    hidScanInput();

    if (keysDown() & KEY_START)
      break;

    DrawGLScene();

    gfxFlush(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 240, 400, GX_TRANSFER_FMT_RGB8);
    gfxFlushBuffers();
    gfxSwapBuffersGpu();
    gspWaitForVBlank();
  }

  // Exit services
  gfxExit();
  hidExit();
  romfsExit();
  return 0;
}
