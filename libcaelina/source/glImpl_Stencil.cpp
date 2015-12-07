#include "glImpl.h"

extern gfx_state *g_state;

extern "C"
{

#ifndef DISABLE_LISTS
gfx_display_list *getList(GLuint name);
#endif

void glStencilFunc( GLenum func, GLint ref, GLuint mask ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::STENCIL_FUNC;
        comm.enum1 = func;
        comm.int1 = ref;
        comm.uint1 = mask;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);
#endif

#ifndef DISABLE_ERRORS
    switch (func) {
        case GL_NEVER:
        case GL_LESS:
        case GL_EQUAL:
        case GL_LEQUAL:
        case GL_GREATER:
        case GL_NOTEQUAL:
        case GL_GEQUAL:
        case GL_ALWAYS: {

        } break;

        default: {
            setError(GL_INVALID_ENUM);
            return;
        }
    }
#endif

    g_state->stencilFunc = func;
    g_state->stencilRef = clampi(ref, 0, (2 << 8) - 1); // TODO get number of actual stencil bits
    g_state->stencilFuncMask = mask;
}

void glStencilMask( GLuint mask ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::STENCIL_MASK;
        comm.uint1 = mask;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);
#endif

    g_state->stencilMask = mask;

}

void glStencilOp( GLenum fail, GLenum zfail, GLenum zpass ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::STENCIL_OP;
        comm.enum1 = fail;
        comm.enum2 = zfail;
        comm.enum3 = zpass;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);
#endif

    CHECK_WITHIN_BEGIN_END(g_state);

#ifndef DISABLE_ERRORS
    switch (fail) {
        case GL_KEEP:
        case GL_ZERO:
        case GL_REPLACE:
        case GL_INCR:
#if !defined(SPEC_GLES) || defined(SPEC_GLES2)
        case GL_INCR_WRAP:
#endif
        case GL_DECR:
#if !defined(SPEC_GLES) || defined(SPEC_GLES2)
        case GL_DECR_WRAP:
#endif
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
#if !defined(SPEC_GLES) || defined(SPEC_GLES2)
        case GL_INCR_WRAP:
#endif
        case GL_DECR:
#if !defined(SPEC_GLES) || defined(SPEC_GLES2)
        case GL_DECR_WRAP:
#endif
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
#if !defined(SPEC_GLES) || defined(SPEC_GLES2)
        case GL_INCR_WRAP:
#endif
        case GL_DECR:
#if !defined(SPEC_GLES) || defined(SPEC_GLES2)
        case GL_DECR_WRAP:
#endif
        case GL_INVERT: {

        } break;
            
        default: {
            setError(GL_INVALID_ENUM);
        }
    }
#endif

    g_state->stencilOpSFail = fail;
    g_state->stencilOpZFail = zfail;
    g_state->stencilOpZPass = zpass;
}

}