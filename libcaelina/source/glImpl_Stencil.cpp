
#include <GL/gl.h>
#include <GL/glext.h>
#include "glImpl.h"

extern gfx_state *g_state;

extern "C"
{

gfx_display_list *getList(GLuint name);

void glStencilFunc( GLenum func, GLint ref, GLuint mask ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::STENCIL_FUNC;
        comm.enum1 = func;
        comm.int1 = ref;
        comm.uint1 = mask;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    switch (func) {
        case GL_NEVER:
        case GL_LESS:
        case GL_EQUAL:
        case GL_LEQUAL:
        case GL_GREATER:
        case GL_NOTEQUAL:
        case GL_GEQUAL:
        case GL_ALWAYS: {
            g_state->stencilFunc = func;
            g_state->stencilRef = clampi(ref, 0, (2 << 8) - 1); // TODO get number of actual stencil bits
            g_state->stencilFuncMask = mask;
        } break;

        default: {
            setError(GL_INVALID_ENUM);
            return;
        }
    }

}

void glStencilMask( GLuint mask ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::STENCIL_MASK;
        comm.uint1 = mask;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    g_state->stencilMask = mask;

}

void glStencilOp( GLenum fail, GLenum zfail, GLenum zpass ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::STENCIL_OP;
        comm.enum1 = fail;
        comm.enum2 = zfail;
        comm.enum3 = zpass;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);

    switch (fail) {
        case GL_KEEP:
        case GL_ZERO:
        case GL_REPLACE:
        case GL_INCR:
        case GL_INCR_WRAP:
        case GL_DECR:
        case GL_DECR_WRAP:
        case GL_INVERT: {

        } break;

        default: {
            setError(GL_INVALID_ENUM);
        }
    }

    switch (zfail) {
        case GL_KEEP:
        case GL_ZERO:
        case GL_REPLACE:
        case GL_INCR:
        case GL_INCR_WRAP:
        case GL_DECR:
        case GL_DECR_WRAP:
        case GL_INVERT: {

        } break;

        default: {
            setError(GL_INVALID_ENUM);
        }
    }
    
    switch (zpass) {
        case GL_KEEP:
        case GL_ZERO:
        case GL_REPLACE:
        case GL_INCR:
        case GL_INCR_WRAP:
        case GL_DECR:
        case GL_DECR_WRAP:
        case GL_INVERT: {
            g_state->stencilOpSFail = fail;
            g_state->stencilOpZFail = zfail;
            g_state->stencilOpZPass = zpass;
        } break;
            
        default: {
            setError(GL_INVALID_ENUM);
        }
    }
}

}