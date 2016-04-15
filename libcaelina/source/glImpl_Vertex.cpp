#include "glImpl.h"

extern gfx_state *g_state;

extern "C"
{

#ifndef DISABLE_LISTS
gfx_display_list *getList(GLuint name);
#endif

#ifndef SPEC_GLES

void glBegin( GLenum mode ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::BEGIN;
        comm.enum1 = mode;
        getList(g_state->currentDisplayList)->commands.push_back(comm);

        g_state->vertexDrawMode = mode;
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);
#endif

    CHECK_WITHIN_BEGIN_END(g_state);

#ifndef DISABLE_ERRORS
    switch(mode) {
        case (GL_POINTS):
        case (GL_LINES):
        case (GL_LINE_STRIP):
        case (GL_LINE_LOOP):
        case (GL_TRIANGLES):
        case (GL_TRIANGLE_STRIP):
        case (GL_TRIANGLE_FAN):
        case (GL_QUADS):
        case (GL_QUAD_STRIP):
        case (GL_POLYGON):
        {

        } break;

        default: {
            setError(GL_INVALID_ENUM);
        } break;
    }
#endif

    g_state->vertexDrawMode = mode;
    g_state->withinBeginEndBlock = GL_TRUE;
}

void glEnd( void ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::END;
        comm.vdata_units = g_state->vertexBuffer.size();
        comm.vdata = g_state->device->cache_vertex_list(&comm.vdata_size);
        getList(g_state->currentDisplayList)->commands.push_back(comm);

        if (g_state->newDisplayListMode == GL_COMPILE) {
            g_state->vertexBuffer.clear();
            return;
        }
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);
#endif

#ifndef DISABLE_ERRORS
    if(!g_state->withinBeginEndBlock) {
        setError(GL_INVALID_OPERATION);
        return;
    }
#endif

    g_state->withinBeginEndBlock = GL_FALSE;

    mat4 projectionMatrix = g_state->projectionMatrixStack[g_state->currentProjectionMatrix];
    mat4 modelvieMatrix = g_state->modelviewMatrixStack[g_state->currentModelviewMatrix];

#ifndef DISABLE_LISTS
    if (g_state->displayListCallDepth == 0) {
#endif

        g_state->device->render_vertices(projectionMatrix, modelvieMatrix);

#ifndef DISABLE_LISTS
    } else {
        g_state->device->render_vertices_vbo(projectionMatrix, modelvieMatrix, g_state->endVBOData, g_state->endVBOUnits);
    }
#endif
    g_state->vertexBuffer.clear();
}

void glTexCoord1f( GLfloat s ) {
    glTexCoord4f(s, 0.0f, 0.0f, 1.0f);
}

void glTexCoord2f( GLfloat s, GLfloat t ) {
    glTexCoord4f(s, t, 0.0f, 1.0f);
}

void glTexCoord3f( GLfloat s, GLfloat t, GLfloat r ) {
    glTexCoord4f(s, t, r, 1.0f);
}

void glTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        getList(g_state->currentDisplayList)->useTex = GL_TRUE;
        getList(g_state->currentDisplayList)->vTex = vec4(s, t, r, q);
    }
#endif

    g_state->currentTextureCoord = vec4(s, t, r, q);
}

void glColor4ub( GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha ) {
    GLfloat r = (GLfloat)red / 255.0f;
    GLfloat g = (GLfloat)green / 255.0f;
    GLfloat b = (GLfloat)blue / 255.0f;
    GLfloat a = (GLfloat)alpha / 255.0f;
    glColor4f(r, g, b, a);
}

void glColor3ub( GLubyte red, GLubyte green, GLubyte blue ) {
    GLfloat r = (GLfloat)red / 255.0f;
    GLfloat g = (GLfloat)green / 255.0f;
    GLfloat b = (GLfloat)blue / 255.0f;
    glColor4f(r, g, b, 1.0f);
}

void glColor3f( GLfloat red, GLfloat green, GLfloat blue ) {
    glColor4f(red, green, blue, 1.0f);
}

#endif // SPEC_GLES

void glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        getList(g_state->currentDisplayList)->useColor = GL_TRUE;
        getList(g_state->currentDisplayList)->vColor = vec4(red, green, blue, alpha);
    }
#endif

    g_state->currentVertexColor = vec4(red, green, blue, alpha);
}

#ifndef SPEC_GLES

void glVertex2i( GLint x, GLint y )
{
    glVertex2f((GLfloat)x, (GLfloat)y);
}

void glVertex2f( GLfloat x, GLfloat y ) {
    glVertex4f(x, y, 0.0f, 1.0f);
}

void glVertex3f( GLfloat x, GLfloat y, GLfloat z ) {
    glVertex4f(x ,y, z, 1.0f);
}

void glVertex4f( GLfloat x, GLfloat y, GLfloat z, GLfloat w ) {
    CHECK_NULL(g_state);

    vertex result = vertex(vec4(x, y, z, w),
                           vec4(g_state->currentVertexColor),
                           vec4(g_state->currentTextureCoord),
                           vec4(g_state->currentVertexNormal));
    if (g_state->vertexDrawMode == GL_QUADS && g_state->vertexBuffer.size() % 6 == 3) {
        int index = g_state->vertexBuffer.size() - 1;
        vertex s2 = vertex(g_state->vertexBuffer[index]);
        vertex s0 = vertex(g_state->vertexBuffer[index - 2]);
        g_state->vertexBuffer.push(s0);
        g_state->vertexBuffer.push(s2);
    }
    g_state->vertexBuffer.push(result);
}

#endif // SPEC_GLES

void glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz ) {
    CHECK_NULL(g_state);

#ifndef DISABLE_LISTS
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        getList(g_state->currentDisplayList)->useNormal = GL_TRUE;
        getList(g_state->currentDisplayList)->vNormal = vec4(nx, ny, nz);
    }
#endif
    
    g_state->currentVertexNormal = vec4(nx, ny, nz, 1.0);
}

void glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) {
  CHECK_NULL(g_state);

#ifndef DISABLE_ERRORS
  if (size < 2 || size > 4) {
    setError(GL_INVALID_VALUE);
  }

  switch (type) {
    case GL_SHORT:
#if !defined(SPEC_GLES) || defined(SPEC_GLES2)
    case GL_INT:
#endif
    case GL_FLOAT:
#ifndef SPEC_GLES
    case GL_DOUBLE:
#endif
    {

    } break;


    default:
      setError(GL_INVALID_ENUM);
  }

  if (stride < 0)
    setError(GL_INVALID_VALUE);
#endif

  g_state->vertexPtrSize = size;
  g_state->vertexPtrType = type;
  g_state->vertexPtrStride = stride;
  g_state->vertexPtr = pointer;
}

void glDrawArrays (GLenum mode, GLint first, GLsizei count) {
  CHECK_NULL(g_state);

#ifndef DISABLE_ERRORS
  switch(mode) {
    case (GL_POINTS):
    case (GL_LINES):
    case (GL_LINE_STRIP):
    case (GL_LINE_LOOP):
    case (GL_TRIANGLES):
    case (GL_TRIANGLE_STRIP):
    case (GL_TRIANGLE_FAN):
    {

    } break;

    default: {
      setError(GL_INVALID_ENUM);
    } break;
  }

  if (count < 0)
    setError(GL_INVALID_VALUE);
#endif

  mat4 projectionMatrix = g_state->projectionMatrixStack[g_state->currentProjectionMatrix];
  mat4 modelvieMatrix = g_state->modelviewMatrixStack[g_state->currentModelviewMatrix];
  g_state->device->render_vertices_array(mode, first, count, projectionMatrix, modelvieMatrix);

}

} // extern "C"
