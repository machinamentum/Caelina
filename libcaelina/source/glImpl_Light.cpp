#include "glImpl.h"

extern gfx_state *g_state;

extern "C"
{

#ifndef DISABLE_LISTS
gfx_display_list *getList(GLuint name);
#endif

void glLightf( GLenum light, GLenum pname, GLfloat param ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::LIGHTF;
        comm.enum1 = light;
        comm.enum2 = pname;
        comm.floats[0] = param;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);
#endif

    CHECK_WITHIN_BEGIN_END(g_state);

#ifndef DISABLE_ERRORS
    switch (light) {
        case GL_LIGHT0:
        case GL_LIGHT1:
        case GL_LIGHT2:
        case GL_LIGHT3:
        case GL_LIGHT4:
        case GL_LIGHT5:
        case GL_LIGHT6:
        case GL_LIGHT7: {

        } break;

        default: {
            setError(GL_INVALID_ENUM);
        } return;
    }
#endif

    GLuint light_index = light - GL_LIGHT0;

    switch (pname) {
        case GL_SPOT_EXPONENT: {
#ifndef DISABLE_ERRORS
            if (param < 0.0 || param > 128.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
#endif
            g_state->lights[light_index].spotlightExpo = param;
        } break;

        case GL_SPOT_CUTOFF: {
#ifndef DISABLE_ERRORS
            if ((param < 0.0 || param > 90.0) && param != 180.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
#endif
            g_state->lights[light_index].spotlightCutoff = param;
        } break;

        case GL_CONSTANT_ATTENUATION: {
#ifndef DISABLE_ERRORS
            if (param < 0.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
#endif
            g_state->lights[light_index].constantAttenuation = param;
        } break;

        case GL_LINEAR_ATTENUATION: {
#ifndef DISABLE_ERRORS
            if (param < 0.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
#endif
            g_state->lights[light_index].linearAttenuation = param;
        } break;

        case GL_QUADRATIC_ATTENUATION: {
#ifndef DISABLE_ERRORS
            if (param < 0.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
#endif
            g_state->lights[light_index].quadraticAttenuation = param;
        } break;

#ifndef DISABLE_ERRORS
        default: {
            setError(GL_INVALID_ENUM);
        } return;
#endif
    }

}

#ifndef DISABLE_LISTS

static int get_light_params_size( GLenum pname ) {
    switch (pname) {
        case GL_SPOT_EXPONENT:
        case GL_SPOT_CUTOFF:
        case GL_CONSTANT_ATTENUATION:
        case GL_LINEAR_ATTENUATION:
        case GL_QUADRATIC_ATTENUATION: return 1;

        case GL_AMBIENT:
        case GL_DIFFUSE:
        case GL_SPECULAR:
        case GL_POSITION: return 4;

        case GL_SPOT_DIRECTION: return 3;

        default: return -1;
    }
}

#endif

void glLightfv( GLenum light, GLenum pname, const GLfloat *params ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::LIGHTFV;
        comm.enum1 = light;
        comm.enum2 = pname;
        for (int i = 0; i < get_light_params_size(pname); ++i) {
            comm.floats[i] = params[i];
        }

        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);
#endif

    CHECK_WITHIN_BEGIN_END(g_state);

#ifndef DISABLE_ERRORS
    switch (light) {
        case GL_LIGHT0:
        case GL_LIGHT1:
        case GL_LIGHT2:
        case GL_LIGHT3:
        case GL_LIGHT4:
        case GL_LIGHT5:
        case GL_LIGHT6:
        case GL_LIGHT7: {

        } break;

        default: {
            setError(GL_INVALID_ENUM);
        } return;
    }
#endif

    GLuint light_index = light - GL_LIGHT0;

    switch (pname) {
        case GL_SPOT_EXPONENT: {
#ifndef DISABLE_ERRORS
            if (params[0] < 0.0 || params[0] > 128.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
#endif
            g_state->lights[light_index].spotlightExpo = params[0];
        } break;

        case GL_SPOT_CUTOFF: {
#ifndef DISABLE_ERRORS
            if ((params[0] < 0.0 || params[0] > 90.0) && params[0] != 180.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
#endif
            g_state->lights[light_index].spotlightCutoff = params[0];
        } break;

        case GL_CONSTANT_ATTENUATION: {
#ifndef DISABLE_ERRORS
            if (params[0] < 0.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
#endif
            g_state->lights[light_index].constantAttenuation = params[0];
        } break;

        case GL_LINEAR_ATTENUATION: {
#ifndef DISABLE_ERRORS
            if (params[0] < 0.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
#endif
            g_state->lights[light_index].linearAttenuation = params[0];
        } break;

        case GL_QUADRATIC_ATTENUATION: {
#ifndef DISABLE_ERRORS
            if (params[0] < 0.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
#endif
            g_state->lights[light_index].quadraticAttenuation = params[0];
        } break;

        case GL_AMBIENT: {
            g_state->lights[light_index].ambient = vec4(params[0], params[1], params[2], params[3]);
        } break;

        case GL_DIFFUSE: {
            g_state->lights[light_index].diffuse = vec4(params[0], params[1], params[2], params[3]);
        } break;

        case GL_SPECULAR: {
            g_state->lights[light_index].specular = vec4(params[0], params[1], params[2], params[3]);
        } break;

        case GL_POSITION: {
            g_state->lights[light_index].position = g_state->modelviewMatrixStack[g_state->currentModelviewMatrix] * vec4(params[0], params[1], params[2], params[3]);
        } break;

        case GL_SPOT_DIRECTION: {
            mat4 upper3x3 = mat4(g_state->modelviewMatrixStack[g_state->currentModelviewMatrix]);
            upper3x3[0 + 3] = 0.0;
            upper3x3[4 + 3] = 0.0;
            upper3x3[8 + 3] = 0.0;
            g_state->lights[0].spotlightDirection = upper3x3 * vec4(params[0], params[1], params[2], 0.0);
        }
#ifndef DISABLE_ERRORS
        default: {
            setError(GL_INVALID_ENUM);
        } return;
#endif
    }
}

}