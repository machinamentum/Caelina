#ifndef DISABLE_LISTS
#include <GL/gl.h>
#include "glImpl.h"

extern gfx_state *g_state;

static void executeList(gfx_display_list *list) {
    vec4 color = list->vColor;
    vec4 tex = list->vTex;
    vec4 norm = list->vNormal;
    if (list->useColor) glColor4f(color.x, color.y, color.z, color.w);
    if (list->useTex) glTexCoord4f(tex.x, tex.y, tex.z, tex.w);
    if (list->useNormal) glNormal3f(norm.x, norm.y, norm.z);
    for (GLuint i = 0; i < list->commands.size(); ++i) {
        gfx_command &comm = list->commands[i];
        switch (comm.type) {
            case gfx_command::PUSH_MATRIX:
                glPushMatrix();
                break;
            case gfx_command::POP_MATRIX:
                glPopMatrix();
                break;
            case gfx_command::MATRIX_MODE:
                glMatrixMode(comm.enum1);
                break;
            case gfx_command::CLEAR_COLOR:
                glClearColor(comm.floats[0], comm.floats[1], comm.floats[2], comm.floats[3]);
                break;
            case gfx_command::CLEAR:
                glClear(comm.mask1);
                break;
            case gfx_command::LOAD_IDENTITY:
                glLoadIdentity();
                break;
            case gfx_command::BEGIN:
                glBegin(comm.enum1);
                break;
            case gfx_command::END:
                g_state->endVBOData = comm.vdata;
                g_state->endVBOUnits = comm.vdata_units;
                glEnd();
                break;
            case gfx_command::BIND_TEXTURE:
                glBindTexture(comm.enum1, comm.uint1);
                break;
            case gfx_command::TEX_IMAGE_2D:
                glTexImage2D(comm.enum1, comm.int1, comm.int2, comm.size1, comm.size2,
                             comm.int3, comm.enum2, comm.enum3, comm.voidp);
                break;
            case gfx_command::ROTATE:
                glRotatef(comm.floats[0], comm.floats[1], comm.floats[2], comm.floats[3]);
                break;
            case gfx_command::SCALE:
                glScalef(comm.floats[0], comm.floats[1], comm.floats[2]);
                break;
            case gfx_command::TRANSLATE:
                glTranslatef(comm.floats[0], comm.floats[1], comm.floats[2]);
                break;
            case gfx_command::ORTHO:
                glOrtho(comm.floats[0], comm.floats[1], comm.floats[2], comm.floats[3], comm.float5, comm.float6);
                break;
            case gfx_command::FRUSTUM:
                glOrtho(comm.floats[0], comm.floats[1], comm.floats[2], comm.floats[3], comm.float5, comm.float6);
                break;
            case gfx_command::VIEWPORT:
                glViewport(comm.int1, comm.int2, comm.size1, comm.size2);
                break;
            case gfx_command::BLEND_FUNC:
                glBlendFunc(comm.enum1, comm.enum2);
                break;
            case gfx_command::ENABLE:
                glEnable(comm.enum1);
                break;
            case gfx_command::DISABLE:
                glDisable(comm.enum1);
                break;
            case gfx_command::TEX_PARAM_I:
                glTexParameteri(comm.enum1, comm.enum2, comm.int1);
                break;
            case gfx_command::SCISSOR:
                glScissor(comm.int1, comm.int2, comm.size1, comm.size2);
                break;
            case gfx_command::CALL_LIST:
                glCallList(comm.uint1);
                break;
            case gfx_command::LIGHTF:
                glLightf(comm.enum1, comm.enum2, comm.floats[0]);
                break;
            case gfx_command::LIGHTFV:
                glLightfv(comm.enum1, comm.enum2, comm.floats);
                break;
            case gfx_command::ALPHA_FUNC:
                glAlphaFunc(comm.enum1, comm.floats[0]);
                break;
            case gfx_command::COLOR_MASK:
                glColorMask(comm.uint1, comm.int1, comm.int2, comm.int3);
                break;
            case gfx_command::DEPTH_MASK:
                glDepthMask(comm.uint1);
                break;
            case gfx_command::STENCIL_MASK:
                glStencilMask(comm.uint1);
                break;
            case gfx_command::STENCIL_FUNC:
                glStencilFunc(comm.enum1, comm.int1, comm.uint1);
                break;
            case gfx_command::STENCIL_OP:
                glStencilOp(comm.enum1, comm.enum2, comm.enum3);
                break;
            case gfx_command::BLEND_COLOR:
                glBlendColor(comm.floats[0], comm.floats[1], comm.floats[2], comm.floats[3]);
                break;
            case gfx_command::CLEAR_DEPTH:
                glClearDepth(comm.floats[0]);
                break;
            case gfx_command::DEPTH_FUNC:
                glDepthFunc(comm.enum1);
                break;
            case gfx_command::NONE:
                break;
        }
    }
}

extern "C"
{

gfx_display_list *getList(GLuint name);

GLuint glGenLists( GLsizei range ) {
    CHECK_NULL(g_state, 0);
    CHECK_WITHIN_BEGIN_END(g_state, 0);

    GLuint ret = g_state->nextDisplayListName;
    for (int i = 0; i < range; ++i) {
        gfx_display_list list;
        list.name = ret + i;
        g_state->displayLists.push(list);
    }
    g_state->nextDisplayListName += range;
    return ret;
}

void glNewList( GLuint list, GLenum mode ) {
    CHECK_NULL(g_state);
    CHECK_WITHIN_BEGIN_END(g_state);
    CHECK_WITHIN_NEW_END(g_state);

#ifndef DISABLE_ERRORS
    if (list == 0) {
        setError(GL_INVALID_VALUE);
        return;
    }

    switch (mode) {
        case GL_COMPILE:
        case GL_COMPILE_AND_EXECUTE: {

        } break;

        default: {
            setError(GL_INVALID_ENUM);
            return;
        }
    }
#endif

    g_state->currentDisplayList = list;
    g_state->newDisplayListMode = mode;
    g_state->withinNewEndListBlock = GL_TRUE;
    getList(list)->commands.clear();
}

void glEndList( void ) {
    CHECK_NULL(g_state);
    CHECK_WITHIN_BEGIN_END(g_state);

#ifndef DISABLE_ERRORS
    if (!g_state->withinNewEndListBlock) {
        setError(GL_INVALID_OPERATION);
        return;
    }
#endif

    //TODO check for out of memory and set error

    g_state->currentDisplayList = 0;
    g_state->newDisplayListMode = GL_COMPILE_AND_EXECUTE;
    g_state->withinNewEndListBlock = GL_FALSE;

}
void glCallList( GLuint list ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::CALL_LIST;
        comm.uint1 = list;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);


    if (list == 0) return;
    
    if (g_state->displayListCallDepth > IMPL_MAX_LIST_CALL_DEPTH) return;
    
    gfx_display_list *clist = getList(list);
    if (clist) {
        ++g_state->displayListCallDepth;
        executeList(clist);
        --g_state->displayListCallDepth;
    }
}


}
#endif