#include "glImpl.h"

extern gfx_state *g_state;

extern "C" {

#ifndef DISABLE_LISTS
gfx_display_list *getList(GLuint name) {
    gfx_display_list *list = NULL;
    for(unsigned int i = 0; i < g_state->displayLists.size(); i++) {
        if(g_state->displayLists[i].name == name) {
            list = &g_state->displayLists[i];
            break;
        }
    }
    return list;
}
#endif

#ifndef DISABLE_ERRORS
void setError(GLenum error) {
    CHECK_NULL(g_state);

    if(g_state->errorFlag == GL_NO_ERROR) {
        g_state->errorFlag = error;
    }
}

GLenum glGetError (void) {
    CHECK_NULL(g_state, -1);

    //TODO implement multiple error flags.
    GLenum error = g_state->errorFlag;
    g_state->errorFlag = GL_NO_ERROR;
    return error;
}
#endif


void glGetIntegerv (GLenum pname, GLint *params) {
    CHECK_NULL(g_state);
    CHECK_WITHIN_BEGIN_END(g_state);

    switch(pname) {
        case (GL_MAX_MODELVIEW_STACK_DEPTH): {
            params[0] = IMPL_MAX_MODELVIEW_STACK_DEPTH;
        } break;
        case (GL_MAX_TEXTURE_STACK_DEPTH): {
            params[0] = IMPL_MAX_TEXTURE_STACK_DEPTH;
        } break;
        case (GL_MAX_PROJECTION_STACK_DEPTH): {
            params[0] = IMPL_MAX_PROJECTION_STACK_DEPTH;
        } break;
    }
}

void glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::CLEAR_COLOR;
        comm.floats[0] = red;
        comm.floats[1] = green;
        comm.floats[2] = blue;
        comm.floats[3] = alpha;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);
#endif

    g_state->clearColor = vec4(clampf(red, 0.0f, 1.0f),
                               clampf(green, 0.0f, 1.0f),
                               clampf(blue, 0.0f, 1.0f),
                               clampf(alpha, 0.0f, 1.0f));
}

#ifndef SPEC_GLES

void glClearDepth( GLclampd depth ) {
  CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
  if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
    gfx_command comm;
    comm.type = gfx_command::CLEAR_DEPTH;
    comm.floats[0] = depth;
    getList(g_state->currentDisplayList)->commands.push_back(comm);
  }

  CHECK_COMPILE_AND_EXECUTE(g_state);
#endif

  g_state->clearDepth = clampf(depth, 0.0f, 1.0f);
}

#else

void glClearDepthf (GLclampf depth) {
  CHECK_NULL(g_state);

  g_state->clearDepth = clampf(depth, 0.0f, 1.0f);
}

#endif

void glDepthFunc( GLenum func ) {
  CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
  if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
    gfx_command comm;
    comm.type = gfx_command::DEPTH_FUNC;
    comm.enum1 = func;
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
    }
      break;
  }
#endif

  g_state->depthFunc = func;
}

void glClear (GLbitfield mask) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::CLEAR;
        comm.mask1 = mask;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);
#endif

#ifndef DISABLE_ERRORS
    if(mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) {
        setError(GL_INVALID_VALUE);
        return;
    }
#endif

    if(mask & GL_COLOR_BUFFER_BIT) {
        g_state->device->clear(g_state->clearColor.x,
                               g_state->clearColor.y,
                               g_state->clearColor.z,
                               g_state->clearColor.w);
    }
    if(mask & GL_DEPTH_BUFFER_BIT) {
        g_state->device->clearDepth(g_state->clearDepth);
    }
    if(mask & GL_STENCIL_BUFFER_BIT) {
        //TODO implement stencil operations
    }
}



void glViewport( GLint x, GLint y, GLsizei width, GLsizei height ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::VIEWPORT;
        comm.int1 = x;
        comm.int2 = y;
        comm.size1 = width;
        comm.size2 = height;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);
#endif

#ifndef DISABLE_ERRORS
    if (width < 0 || height < 0) {
        setError(GL_INVALID_VALUE);
        return;
    }
#endif

    float h = g_state->device->getHeight();
    float w = g_state->device->getWidth();
    float vw = (float)width;
    float vh = (float)height;
    g_state->viewportMatrix = mat4::viewport(((float)x / w), (float)y / h,  vw / (float)w, vh / (float)h);
}


void glEnable( GLenum cap ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::ENABLE;
        comm.enum1 = cap;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);
#endif

    CHECK_WITHIN_BEGIN_END();

    switch(cap) {
        case (GL_TEXTURE_2D): {
            g_state->enableTexture2D = GL_TRUE;
        } break;

        case (GL_DEPTH_TEST): {
            g_state->enableDepthTest = GL_TRUE;
        } break;

        case (GL_BLEND): {
            g_state->enableBlend = GL_TRUE;
        } break;

        case (GL_SCISSOR_TEST): {
            g_state->enableScissorTest = GL_TRUE;
        } break;
            
        case (GL_LIGHTING): {
            g_state->enableLighting = GL_TRUE;
        } break;
            
        case (GL_LIGHT0):
        case (GL_LIGHT1):
        case (GL_LIGHT2):
        case (GL_LIGHT3):
        case (GL_LIGHT4):
        case (GL_LIGHT5):
        case (GL_LIGHT6):
        case (GL_LIGHT7): {
            g_state->enableLight[GL_LIGHT0 - cap] = GL_TRUE;
        } break;

        case (GL_ALPHA_TEST): {
            g_state->enableAlphaTest = GL_TRUE;
        } break;

        case (GL_STENCIL_TEST): {
            g_state->enableStencilTest = GL_TRUE;
        } break;

#ifndef DISABLE_ERRORS
        default: {
            setError(GL_INVALID_ENUM);
            return;
        }
#endif
    }
}

void glDisable( GLenum cap ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::DISABLE;
        comm.enum1 = cap;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }
    
    CHECK_COMPILE_AND_EXECUTE(g_state);
#endif
    
    CHECK_WITHIN_BEGIN_END();
    
    switch(cap) {
        case (GL_TEXTURE_2D): {
            g_state->enableTexture2D = GL_FALSE;
        } break;
            
        case (GL_DEPTH_TEST): {
            g_state->enableDepthTest = GL_FALSE;
        } break;
            
        case (GL_BLEND): {
            g_state->enableBlend = GL_FALSE;
        } break;
            
        case (GL_SCISSOR_TEST): {
            g_state->enableScissorTest = GL_FALSE;
        } break;
            
        case (GL_LIGHTING): {
            g_state->enableLighting = GL_FALSE;
        } break;
            
        case (GL_LIGHT0):
        case (GL_LIGHT1):
        case (GL_LIGHT2):
        case (GL_LIGHT3):
        case (GL_LIGHT4):
        case (GL_LIGHT5):
        case (GL_LIGHT6):
        case (GL_LIGHT7): {
            g_state->enableLight[GL_LIGHT0 - cap] = GL_FALSE;
        } break;

        case (GL_ALPHA_TEST): {
            g_state->enableAlphaTest = GL_FALSE;
        } break;

        case (GL_STENCIL_TEST): {
            g_state->enableStencilTest = GL_FALSE;
        } break;
#ifndef DISABLE_ERRORS
        default: {
            setError(GL_INVALID_ENUM);
            return;
        }
#endif
    }
}

void glDepthMask( GLboolean flag ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::DEPTH_MASK;
        comm.uint1 = flag;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);
#endif

    CHECK_WITHIN_BEGIN_END(g_state);

    g_state->depthMask = flag != 0;
}

void glFlush ( void ) {
    CHECK_NULL(g_state);
    CHECK_WITHIN_BEGIN_END();
    g_state->device->flush_commands();
}

void glFinish ( void ) {
    CHECK_NULL(g_state);
    CHECK_WITHIN_BEGIN_END();
    g_state->device->flush_wait_commands();
}

void glEnableClientState (GLenum array) {
  // TODO
}

void glDisableClientState (GLenum array) {
  // TODO
}

}
