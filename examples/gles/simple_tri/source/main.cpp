/****************************************
 *       3DS NeHe Lesson 01            *
 *       Author: machinamentum         *
 ****************************************/

#include <3ds.h>
#include <cmath>
#include <GLES/gl.h>
#include <gfx_device.h>

static GLfloat *vertices = nullptr;

float trans = 0.5f;

void DrawGLScene() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glTranslatef(-1.5f,0.0f,-6.0f);
    glColor4f(1, 1, 0, trans);

    glNormal3f(1, 1, 1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableClientState(GL_VERTEX_ARRAY);

}

int main()
{
    gfxInitDefault();
    hidInit();
    vertices = (GLfloat *)linearAlloc(4 * 32 * 32);
    vertices[0] = 1.0;
    vertices[1] = 0.0;
    vertices[2] = 0.0;

    vertices[3] = 0.0;
    vertices[4] = 1.0;
    vertices[5] = 0.0;

    vertices[6] = -1.0;
    vertices[7] = 0.0;
    vertices[8] = 0.0;

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
    glFrustumf(l, r, b, t, near, far);
    //3DS' framebuffers are sideways
    glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (aptMainLoop())
    {
        hidScanInput();

        if (keysDown() & KEY_START)
            break;

      if (keysDown() & KEY_UP) {
        trans += 0.1f;
      }

      if (keysDown() & KEY_DOWN) trans -= 0.1f;

        DrawGLScene();

        gfxFlush(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 240, 400, GX_TRANSFER_FMT_RGB8);
        gfxFlushBuffers();
        gfxSwapBuffersGpu();
        gspWaitForVBlank();
    }
    
    // Exit services
    gfxExit();
    hidExit();
    return 0;
}
