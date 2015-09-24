
#include <GL/gl.h>
#include "glImpl.h"

extern gfx_state *g_state;

extern "C"
{

gfx_display_list *getList(GLuint name);

void glLoadIdentity (void) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::LOAD_IDENTITY;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);

    switch(g_state->matrixMode) {
        case (GL_MODELVIEW): {
            g_state->modelviewMatrixStack[g_state->currentModelviewMatrix] = mat4();
        } break;
        case (GL_PROJECTION): {
            g_state->projectionMatrixStack[g_state->currentProjectionMatrix] = mat4();
        } break;
        case (GL_TEXTURE): {
            g_state->textureMatrixStack[g_state->currentTextureMatrix] = mat4();
        } break;
    }
}

void glPopMatrix (void) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::POP_MATRIX;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);

    switch(g_state->matrixMode) {
        case (GL_MODELVIEW): {
            if(g_state->currentModelviewMatrix - 1 < 0) {
                setError(GL_STACK_UNDERFLOW);
                return;
            }

            g_state->currentModelviewMatrix--;
        } break;

        case (GL_PROJECTION): {
            if(g_state->currentProjectionMatrix - 1 < 0) {
                setError(GL_STACK_UNDERFLOW);
                return;
            }

            g_state->currentProjectionMatrix--;
        } break;

        case (GL_TEXTURE): {
            if(g_state->currentTextureMatrix - 1 < 0) {
                setError(GL_STACK_UNDERFLOW);
                return;
            }

            g_state->currentTextureMatrix--;
        } break;

    }
}

void glPushMatrix (void) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::PUSH_MATRIX;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);



    switch(g_state->matrixMode) {
        case (GL_MODELVIEW): {
            if(g_state->currentModelviewMatrix + 1 >= IMPL_MAX_MODELVIEW_STACK_DEPTH) {
                setError(GL_STACK_OVERFLOW);
                return;
            }

            g_state->modelviewMatrixStack[g_state->currentModelviewMatrix + 1] = mat4(g_state->modelviewMatrixStack[g_state->currentModelviewMatrix]);
            g_state->currentModelviewMatrix++;
        } break;

        case (GL_PROJECTION): {
            if(g_state->currentProjectionMatrix + 1 >= IMPL_MAX_PROJECTION_STACK_DEPTH) {
                setError(GL_STACK_OVERFLOW);
                return;
            }

            g_state->projectionMatrixStack[g_state->currentProjectionMatrix + 1] = mat4(g_state->projectionMatrixStack[g_state->currentProjectionMatrix]);
            g_state->currentProjectionMatrix++;
        } break;

        case (GL_TEXTURE): {
            if(g_state->currentTextureMatrix + 1 >= IMPL_MAX_TEXTURE_STACK_DEPTH) {
                setError(GL_STACK_OVERFLOW);
                return;
            }

            g_state->textureMatrixStack[g_state->currentTextureMatrix + 1] = mat4(g_state->textureMatrixStack[g_state->currentTextureMatrix]);
            g_state->currentTextureMatrix++;
        } break;

    }
}

void glMatrixMode (GLenum mode) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::MATRIX_MODE;
        comm.enum1 = mode;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }
    
    CHECK_WITHIN_BEGIN_END(g_state);
    
    CHECK_COMPILE_AND_EXECUTE(g_state);
    
    switch(mode) {
        case (GL_MODELVIEW):
        case (GL_PROJECTION):
        case (GL_TEXTURE):
        {
            g_state->matrixMode = mode;
        } break;
            
        default: {
            setError(GL_INVALID_ENUM);
        } break;
    }
    
}

void glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::ROTATE;
        comm.floats[0] = angle;
        comm.floats[1] = x;
        comm.floats[2] = y;
        comm.floats[3] = z;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);

    mat4 rotation = mat4::rotate(angle, x, y, z);

    switch(g_state->matrixMode) {
        case (GL_MODELVIEW): {
            g_state->modelviewMatrixStack[g_state->currentModelviewMatrix] = g_state->modelviewMatrixStack[g_state->currentModelviewMatrix] * rotation;
        } break;
        case (GL_PROJECTION): {
            g_state->projectionMatrixStack[g_state->currentProjectionMatrix] = g_state->projectionMatrixStack[g_state->currentProjectionMatrix] * rotation;
        } break;
        case (GL_TEXTURE): {
            g_state->textureMatrixStack[g_state->currentTextureMatrix] = g_state->textureMatrixStack[g_state->currentTextureMatrix] * rotation;
        } break;
    }
}

void glTranslatef (GLfloat x, GLfloat y, GLfloat z) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::TRANSLATE;
        comm.floats[0] = x;
        comm.floats[1] = y;
        comm.floats[2] = z;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);

    mat4 translation = mat4::translate(x, y, z);

    switch(g_state->matrixMode) {
        case (GL_MODELVIEW): {
            g_state->modelviewMatrixStack[g_state->currentModelviewMatrix] = g_state->modelviewMatrixStack[g_state->currentModelviewMatrix] * translation;
        } break;
        case (GL_PROJECTION): {
            g_state->projectionMatrixStack[g_state->currentProjectionMatrix] = g_state->projectionMatrixStack[g_state->currentProjectionMatrix] * translation;
        } break;
        case (GL_TEXTURE): {
            g_state->textureMatrixStack[g_state->currentTextureMatrix] = g_state->textureMatrixStack[g_state->currentTextureMatrix] * translation;
        } break;
    }
}

void glScalef( GLfloat x, GLfloat y, GLfloat z ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::SCALE;
        comm.floats[0] = x;
        comm.floats[1] = y;
        comm.floats[2] = z;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);

    mat4 scale = mat4::scale(x, y, z);

    switch(g_state->matrixMode) {
        case (GL_MODELVIEW): {
            g_state->modelviewMatrixStack[g_state->currentModelviewMatrix] = g_state->modelviewMatrixStack[g_state->currentModelviewMatrix] * scale;
        } break;
        case (GL_PROJECTION): {
            g_state->projectionMatrixStack[g_state->currentProjectionMatrix] = g_state->projectionMatrixStack[g_state->currentProjectionMatrix] * scale;
        } break;
        case (GL_TEXTURE): {
            g_state->textureMatrixStack[g_state->currentTextureMatrix] = g_state->textureMatrixStack[g_state->currentTextureMatrix] * scale;
        } break;
    }
}

void glOrtho( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near_val, GLdouble far_val ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::ORTHO;
        comm.floats[0] = left;
        comm.floats[1] = right;
        comm.floats[2] = bottom;
        comm.floats[3] = top;
        comm.float5 = near_val;
        comm.float6 = far_val;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);

    if(left == right || top == bottom || near_val == far_val) {
        setError(GL_INVALID_VALUE);
        return;
    }

    mat4 ortho = mat4::ortho(left, right, bottom, top, near_val, far_val);

    switch(g_state->matrixMode) {
        case (GL_MODELVIEW): {
            g_state->modelviewMatrixStack[g_state->currentModelviewMatrix] = g_state->modelviewMatrixStack[g_state->currentModelviewMatrix] * ortho;
        } break;
        case (GL_PROJECTION): {
            g_state->projectionMatrixStack[g_state->currentProjectionMatrix] = g_state->projectionMatrixStack[g_state->currentProjectionMatrix] * ortho;
        } break;
        case (GL_TEXTURE): {
            g_state->textureMatrixStack[g_state->currentTextureMatrix] = g_state->textureMatrixStack[g_state->currentTextureMatrix] * ortho;
        } break;
    }
}

void glFrustumf (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::FRUSTUM;
        comm.floats[0] = left;
        comm.floats[1] = right;
        comm.floats[2] = bottom;
        comm.floats[3] = top;
        comm.float5 = zNear;
        comm.float6 = zFar;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);

    if(left == right || top == bottom || zNear == zFar || zNear < 0.0f || zFar < 0.0f) {
        setError(GL_INVALID_VALUE);
        return;
    }
    
    mat4 frustum = mat4::frustum(left, right, bottom, top, zNear, zFar);
    
    switch(g_state->matrixMode) {
        case (GL_MODELVIEW): {
            g_state->modelviewMatrixStack[g_state->currentModelviewMatrix] = g_state->modelviewMatrixStack[g_state->currentModelviewMatrix] * frustum;
        } break;
        case (GL_PROJECTION): {
            g_state->projectionMatrixStack[g_state->currentProjectionMatrix] = g_state->projectionMatrixStack[g_state->currentProjectionMatrix] * frustum;
        } break;
        case (GL_TEXTURE): {
            g_state->textureMatrixStack[g_state->currentTextureMatrix] = g_state->textureMatrixStack[g_state->currentTextureMatrix] * frustum;
        } break;
    }
}
}