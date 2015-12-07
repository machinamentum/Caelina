/****************************************
 *       3DS NeHe Lesson 04            *
 *       Author: machinamentum         *
 ****************************************/

#include <3ds.h>
#include <cmath>
#include <GL/gl.h>
#include <gfx_device.h>

GLfloat     rtri;                       // Angle For The Triangle ( NEW )
GLfloat     rquad;                      // Angle For The Quad     ( NEW )

void DrawGLScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glTranslatef(-1.5f,0.0f,-6.0f);
    glRotatef(rtri,0.0f,1.0f,0.0f);             // Rotate The Triangle On The Y axis ( NEW )
    glBegin(GL_TRIANGLES);                      // Drawing Using Triangles
        glColor3f(1.0f,0.0f,0.0f);          // Set The Color To Red
        glVertex3f( 0.0f, 1.0f, 0.0f);              // Top
        glColor3f(0.0f,1.0f,0.0f);          // Set The Color To Green
        glVertex3f(-1.0f,-1.0f, 0.0f);              // Bottom Left
        glColor3f(0.0f,0.0f,1.0f);          // Set The Color To Blue
        glVertex3f( 1.0f,-1.0f, 0.0f);              // Bottom Right
    glEnd();                            // Finished Drawing The Triangle

    glLoadIdentity();                   // Reset The Current Modelview Matrix
    glTranslatef(1.5f,0.0f,-6.0f);              // Move Right 1.5 Units And Into The Screen 6.0
    glRotatef(rquad,1.0f,0.0f,0.0f);            // Rotate The Quad On The X axis ( NEW )
    glColor3f(0.5f,0.5f,1.0f);              // Set The Color To Blue One Time Only
    glBegin(GL_QUADS);                      // Draw A Quad
        glVertex3f(-1.0f, 1.0f, 0.0f);              // Top Left
        glVertex3f( 1.0f, 1.0f, 0.0f);              // Top Right
        glVertex3f( 1.0f,-1.0f, 0.0f);              // Bottom Right
        glVertex3f(-1.0f,-1.0f, 0.0f);              // Bottom Left
    glEnd();                            // Done Drawing The Quad

    rtri+=0.2f;                     // Increase The Rotation Variable For The Triangle ( NEW )
    rquad-=0.15f;                       // Decrease The Rotation Variable For The Quad     ( NEW )
}

int main()
{
    gfxInitDefault();
    hidInit();

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

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

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
    return 0;
}
