
#include <GL/gl.h>
#include "glImpl.h"

extern gfx_state *g_state;

extern "C"
{

gfx_display_list *getList(GLuint name);


void glBlendFunc( GLenum sfactor, GLenum dfactor ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::BLEND_FUNC;
        comm.enum1 = sfactor;
        comm.enum2 = dfactor;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END();

    switch (sfactor) {
        case (GL_ZERO):
        case (GL_ONE):
        case (GL_SRC_COLOR):
        case (GL_ONE_MINUS_SRC_COLOR):
        case (GL_DST_COLOR):
        case (GL_ONE_MINUS_DST_COLOR):
        case (GL_SRC_ALPHA):
        case (GL_ONE_MINUS_SRC_ALPHA):
        case (GL_DST_ALPHA):
        case (GL_ONE_MINUS_DST_ALPHA):
        case (GL_CONSTANT_COLOR):
        case (GL_ONE_MINUS_CONSTANT_COLOR):
        case (GL_CONSTANT_ALPHA):
        case (GL_ONE_MINUS_CONSTANT_ALPHA):
        case (GL_SRC_ALPHA_SATURATE): {
            g_state->blendSrcFactor = sfactor;
        } break;

        default: {
            setError(GL_INVALID_ENUM);
        } return;
    }

    switch (dfactor) {
        case (GL_ZERO):
        case (GL_ONE):
        case (GL_SRC_COLOR):
        case (GL_ONE_MINUS_SRC_COLOR):
        case (GL_DST_COLOR):
        case (GL_ONE_MINUS_DST_COLOR):
        case (GL_SRC_ALPHA):
        case (GL_ONE_MINUS_SRC_ALPHA):
        case (GL_DST_ALPHA):
        case (GL_ONE_MINUS_DST_ALPHA):
        case (GL_CONSTANT_COLOR):
        case (GL_ONE_MINUS_CONSTANT_COLOR):
        case (GL_CONSTANT_ALPHA):
        case (GL_ONE_MINUS_CONSTANT_ALPHA):
        {
            g_state->blendDstFactor = dfactor;
        } break;
            
        default: {
            setError(GL_INVALID_ENUM);
        } return;
    }
}


void glScissor( GLint x, GLint y, GLsizei width, GLsizei height) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::SCISSOR;
        comm.int1 = x;
        comm.int2 = y;
        comm.size1 = width;
        comm.size2 = height;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);


    if (width < 0 || height < 0) {
        setError(GL_INVALID_VALUE);
        return;
    }
    
    g_state->scissorBox = {x, y, width, height};
}

void glBlendColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::BLEND_COLOR;
        comm.floats[0] = red;
        comm.floats[1] = green;
        comm.floats[2] = blue;
        comm.floats[3] = alpha;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);

    u32 Value = 0;
    Value |= ((GLuint)(clampf(red, 0.0f, 1.0f) * 255.0f) & 0xFF);
    Value |= ((GLuint)(clampf(green, 0.0f, 1.0f) * 255.0f) & 0xFF) << 8;
    Value |= ((GLuint)(clampf(blue, 0.0f, 1.0f) * 255.0f) & 0xFF) << 16;
    Value |= ((GLuint)(clampf(alpha, 0.0f, 1.0f) * 255.0f) & 0xFF) << 24;
    g_state->blendColor = Value;
}

void glAlphaFunc( GLenum func, GLclampf ref ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::ALPHA_FUNC;
        comm.enum1 = func;
        comm.floats[0] = ref;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);

    switch (func) {
        case GL_NEVER:
        case GL_LESS:
        case GL_EQUAL:
        case GL_LEQUAL:
        case GL_GREATER:
        case GL_NOTEQUAL:
        case GL_GEQUAL:
        case GL_ALWAYS: {
            g_state->alphaTestFunc = func;
            g_state->alphaTestRef = clampf(ref, 0.0, 1.0);
        } break;

        default: {
            setError(GL_INVALID_ENUM);
            return;
        }
    }
}

void glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::COLOR_MASK;
        comm.uint1 = red;
        comm.int1 = green;
        comm.int2 = blue;
        comm.int3 = alpha;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }
    
    CHECK_COMPILE_AND_EXECUTE(g_state);
    
    g_state->colorMaskRed = red != 0;
    g_state->colorMaskGreen = green != 0;
    g_state->colorMaskBlue = blue != 0;
    g_state->colorMaskAlpha = alpha != 0;
}

    
}