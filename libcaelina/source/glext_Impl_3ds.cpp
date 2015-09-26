#include <GL/gl.h>
#include <GL/ctr.h>
#include "glImpl.h"
#include "driver_3ds.h"

extern gfx_state *g_state;

static GPU_SCISSORMODE glext_scissor_mode(GLenum mode) {
    switch (mode) {
        case GL_SCISSOR_NORMAL_DMP: return GPU_SCISSOR_NORMAL;
        case GL_SCISSOR_INVERT_DMP: return GPU_SCISSOR_INVERT;
    }

    return GPU_SCISSOR_DISABLE;
}

GLAPI void APIENTRY glScissorMode( GLenum mode ) {
    CHECK_NULL(g_state);

    switch (mode) {
        case GL_SCISSOR_NORMAL_DMP:
        case GL_SCISSOR_INVERT_DMP: {
            g_state->device->ext_state.scissorMode = glext_scissor_mode(mode);
        } break;

        default: {
            setError(GL_INVALID_ENUM);
            return;
        } break;
    }
}
