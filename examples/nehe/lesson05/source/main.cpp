/****************************************
 *       3DS NeHe Lesson 05            *
 *       Author: machinamentum         *
 ****************************************/

#include <3ds.h>
#include <cmath>
#include <GL/gl.h>
#include <gfx_device.h>

GLfloat     rtri;                       // Angle For The Triangle ( NEW )
GLfloat     rquad;                      // Angle For The Quad     ( NEW )

void DrawGLScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear The Screen And The Depth Buffer
    glLoadIdentity();                   // Reset The View
    glTranslatef(-1.5f,0.0f,-6.0f);             // Move Left And Into The Screen

    glRotatef(rtri,0.0f,1.0f,0.0f);             // Rotate The Pyramid On It's Y Axis

    glBegin(GL_TRIANGLES);                  // Start Drawing The Pyramid
        glColor3f(1.0f,0.0f,0.0f);          // Red
        glVertex3f( 0.0f, 1.0f, 0.0f);          // Top Of Triangle (Front)
        glColor3f(0.0f,1.0f,0.0f);          // Green
        glVertex3f(-1.0f,-1.0f, 1.0f);          // Left Of Triangle (Front)
        glColor3f(0.0f,0.0f,1.0f);          // Blue
        glVertex3f( 1.0f,-1.0f, 1.0f);          // Right Of Triangle (Front)

        glColor3f(1.0f,0.0f,0.0f);          // Red
        glVertex3f( 0.0f, 1.0f, 0.0f);          // Top Of Triangle (Front)
        glColor3f(0.0f,1.0f,0.0f);          // Green
        glVertex3f(-1.0f,-1.0f, 1.0f);          // Left Of Triangle (Front)
        glColor3f(0.0f,0.0f,1.0f);          // Blue
        glVertex3f( 1.0f,-1.0f, 1.0f);          // Right Of Triangle (Front)

        glColor3f(1.0f,0.0f,0.0f);          // Red
        glVertex3f( 0.0f, 1.0f, 0.0f);          // Top Of Triangle (Right)
        glColor3f(0.0f,0.0f,1.0f);          // Blue
        glVertex3f( 1.0f,-1.0f, 1.0f);          // Left Of Triangle (Right)
        glColor3f(0.0f,1.0f,0.0f);          // Green
        glVertex3f( 1.0f,-1.0f, -1.0f);         // Right Of Triangle (Right)

        glColor3f(1.0f,0.0f,0.0f);          // Red
        glVertex3f( 0.0f, 1.0f, 0.0f);          // Top Of Triangle (Back)
        glColor3f(0.0f,1.0f,0.0f);          // Green
        glVertex3f( 1.0f,-1.0f, -1.0f);         // Left Of Triangle (Back)
        glColor3f(0.0f,0.0f,1.0f);          // Blue
        glVertex3f(-1.0f,-1.0f, -1.0f);         // Right Of Triangle (Back)

        glColor3f(1.0f,0.0f,0.0f);          // Red
        glVertex3f( 0.0f, 1.0f, 0.0f);          // Top Of Triangle (Left)
        glColor3f(0.0f,0.0f,1.0f);          // Blue
        glVertex3f(-1.0f,-1.0f,-1.0f);          // Left Of Triangle (Left)
        glColor3f(0.0f,1.0f,0.0f);          // Green
        glVertex3f(-1.0f,-1.0f, 1.0f);          // Right Of Triangle (Left)
    glEnd();                        // Done Drawing The Pyramid


    glLoadIdentity();
    glTranslatef(1.5f,0.0f,-7.0f);              // Move Right And Into The Screen

    glRotatef(rquad,1.0f,1.0f,1.0f);            // Rotate The Cube On X, Y & Z

    glBegin(GL_QUADS);                  // Start Drawing The Cube
        glColor3f(0.0f,1.0f,0.0f);          // Set The Color To Green
        glVertex3f( 1.0f, 1.0f,-1.0f);          // Top Right Of The Quad (Top)
        glVertex3f(-1.0f, 1.0f,-1.0f);          // Top Left Of The Quad (Top)
        glVertex3f(-1.0f, 1.0f, 1.0f);          // Bottom Left Of The Quad (Top)
        glVertex3f( 1.0f, 1.0f, 1.0f);          // Bottom Right Of The Quad (Top)

        glColor3f(1.0f,0.5f,0.0f);          // Set The Color To Orange
        glVertex3f( 1.0f,-1.0f, 1.0f);          // Top Right Of The Quad (Bottom)
        glVertex3f(-1.0f,-1.0f, 1.0f);          // Top Left Of The Quad (Bottom)
        glVertex3f(-1.0f,-1.0f,-1.0f);          // Bottom Left Of The Quad (Bottom)
        glVertex3f( 1.0f,-1.0f,-1.0f);          // Bottom Right Of The Quad (Bottom)

        glColor3f(1.0f,0.0f,0.0f);          // Set The Color To Red
        glVertex3f( 1.0f, 1.0f, 1.0f);          // Top Right Of The Quad (Front)
        glVertex3f(-1.0f, 1.0f, 1.0f);          // Top Left Of The Quad (Front)
        glVertex3f(-1.0f,-1.0f, 1.0f);          // Bottom Left Of The Quad (Front)
        glVertex3f( 1.0f,-1.0f, 1.0f);          // Bottom Right Of The Quad (Front)

        glColor3f(1.0f,1.0f,0.0f);          // Set The Color To Yellow
        glVertex3f( 1.0f,-1.0f,-1.0f);          // Bottom Left Of The Quad (Back)
        glVertex3f(-1.0f,-1.0f,-1.0f);          // Bottom Right Of The Quad (Back)
        glVertex3f(-1.0f, 1.0f,-1.0f);          // Top Right Of The Quad (Back)
        glVertex3f( 1.0f, 1.0f,-1.0f);          // Top Left Of The Quad (Back)

        glColor3f(0.0f,0.0f,1.0f);          // Set The Color To Blue
        glVertex3f(-1.0f, 1.0f, 1.0f);          // Top Right Of The Quad (Left)
        glVertex3f(-1.0f, 1.0f,-1.0f);          // Top Left Of The Quad (Left)
        glVertex3f(-1.0f,-1.0f,-1.0f);          // Bottom Left Of The Quad (Left)
        glVertex3f(-1.0f,-1.0f, 1.0f);          // Bottom Right Of The Quad (Left)

        glColor3f(1.0f,0.0f,1.0f);          // Set The Color To Violet
        glVertex3f( 1.0f, 1.0f,-1.0f);          // Top Right Of The Quad (Right)
        glVertex3f( 1.0f, 1.0f, 1.0f);          // Top Left Of The Quad (Right)
        glVertex3f( 1.0f,-1.0f, 1.0f);          // Bottom Left Of The Quad (Right)
        glVertex3f( 1.0f,-1.0f,-1.0f);          // Bottom Right Of The Quad (Right)
    glEnd();                        // Done Drawing The Quad

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
