#include "matrix.h"
#include <GL/gl.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "glImpl.h"
#include "driver_soft.h"

#ifdef _3DS
#include "driver_3ds.h"
#endif

gfx_state* g_state = NULL;

extern "C" {

static gfx_device *gfx_get_platform_driver(gfx_state *state, int w, int h) {
#ifdef _3DS
    return new gfx_device_3ds(state, w, h);
#else
    return NULL;
#endif
}

static void *gfx_platform_texture_malloc(u32 size) {
#ifdef _3DS
    return linearAlloc(size);
#else
    return malloc(size);
#endif
}


void* gfxCreateDevice(int width, int height) {
    gfx_device* dev = NULL;
    dev = gfx_get_platform_driver(new gfx_state(), width, height);
    dev->g_state->device = dev;
    return dev;
}

void gfxDestroyDevice(void* device) {
    if(device) delete (gfx_device*)device;
}


void  gfxMakeCurrent(void* device) {
    if (!device) {
        g_state = NULL;
        return;
    }
    g_state = ((gfx_device*)device)->g_state;
}

void gfxFlush(unsigned char* fb) {
    CHECK_NULL(g_state);
    CHECK_NULL(fb);

    g_state->device->flush(fb);
}

static gfx_display_list *getList(GLuint name) {
    gfx_display_list *list = NULL;
    for(unsigned int i = 0; i < g_state->displayLists.size(); i++) {
        if(g_state->displayLists[i].name == name) {
            list = &g_state->displayLists[i];
            break;
        }
    }
    return list;
}

static gfx_texture *getTexture(GLuint name) {
    for(unsigned int i = 0; i < g_state->textures.size(); i++) {
        if(g_state->textures[i].tname == name) {
            return &g_state->textures[i];
        }
    }

    return NULL;
}

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
                g_state->endVBOCMD = comm;
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

            case gfx_command::NONE:
                break;
        }
    }
}

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

    g_state->clearColor = vec4(clampf(red, 0.0f, 1.0f),
                               clampf(green, 0.0f, 1.0f),
                               clampf(blue, 0.0f, 1.0f),
                               clampf(alpha, 0.0f, 1.0f));
}

void glClear (GLbitfield mask) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::CLEAR;
        comm.mask1 = mask;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    if(mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) {
        setError(GL_INVALID_VALUE);
        return;
    }

    if(mask & GL_COLOR_BUFFER_BIT) {
        u8 r = (u8)(g_state->clearColor.x * 255.0f);
        u8 g = (u8)(g_state->clearColor.y * 255.0f);
        u8 b = (u8)(g_state->clearColor.z * 255.0f);
        u8 a = (u8)(g_state->clearColor.w * 255.0f);
        g_state->device->clear(r, g, b, a);
    }
    if(mask & GL_DEPTH_BUFFER_BIT) {
        g_state->device->clearDepth(1.0);
    }
    if(mask & GL_STENCIL_BUFFER_BIT) {
        //TODO implement stencil operations
    }
}

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

void glBegin( GLenum mode ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::BEGIN;
        comm.enum1 = mode;
        getList(g_state->currentDisplayList)->commands.push_back(comm);

        g_state->vertexDrawMode = mode;
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);

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
            g_state->vertexDrawMode = mode;
        } break;

        default: {
            setError(GL_INVALID_ENUM);
        } break;
    }

    g_state->withinBeginEndBlock = GL_TRUE;
}

void glEnd( void ) {
    CHECK_NULL(g_state);

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

    if(!g_state->withinBeginEndBlock) {
        setError(GL_INVALID_OPERATION);
        return;
    }

    g_state->withinBeginEndBlock = GL_FALSE;

    mat4 projectionMatrix = g_state->projectionMatrixStack[g_state->currentProjectionMatrix];
    mat4 modelvieMatrix = g_state->modelviewMatrixStack[g_state->currentModelviewMatrix];

    if (g_state->displayListCallDepth == 0) {
        g_state->device->render_vertices(projectionMatrix, modelvieMatrix);
    } else {
        g_state->device->render_vertices_vbo(projectionMatrix, modelvieMatrix, g_state->endVBOCMD.vdata, g_state->endVBOCMD.vdata_units);
    }
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

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        getList(g_state->currentDisplayList)->useTex = GL_TRUE;
        getList(g_state->currentDisplayList)->vTex = vec4(s, t, r, q);
    }

    g_state->currentTextureCoord = vec4(s, t, r, q);
}

void glColor3f( GLfloat red, GLfloat green, GLfloat blue ) {
    glColor4f(red, green, blue, 1.0f);
}

void glColor4f( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        getList(g_state->currentDisplayList)->useColor = GL_TRUE;
        getList(g_state->currentDisplayList)->vColor = vec4(red, green, blue, alpha);
    }

    g_state->currentVertexColor = vec4(red, green, blue, alpha);
}

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

void glNormal3f( GLfloat nx, GLfloat ny, GLfloat nz ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        getList(g_state->currentDisplayList)->useNormal = GL_TRUE;
        getList(g_state->currentDisplayList)->vNormal = vec4(nx, ny, nz);
    }

    g_state->currentVertexNormal = vec4(nx, ny, nz, 1.0);
}

GLboolean glIsTexture( GLuint texture ) {
    CHECK_NULL(g_state, GL_FALSE);

    if(texture == 0) return GL_FALSE;

    for(unsigned int i = 0; i < g_state->textures.size(); i++) {
        if(g_state->textures[i].tname == texture && g_state->textures[i].target != 0) {
            return GL_TRUE;
        }
    }

    return GL_FALSE;

}

void glGenTextures( GLsizei n, GLuint *textures ) {
    CHECK_NULL(g_state);

    if(n < 0) {
        setError(GL_INVALID_VALUE);
        return;
    }

    for(GLsizei i = 0; i < n; ++i) {
        GLuint tname = rand() + 1;
        while(g_state->textures.contains(gfx_texture(tname))) {
            tname = rand() + 1;
        }
        textures[i] = tname;

        g_state->textures.push(gfx_texture(tname));
    }
}

void glDeleteTextures( GLsizei n, const GLuint *textures) {
    CHECK_NULL(g_state);

    if (n < 0) {
        setError(GL_INVALID_VALUE);
        return;
    }

    for (GLsizei i = 0; i < n; ++i) {
        gfx_texture *text = getTexture(textures[i]);
        if (text) {
            g_state->device->free_texture(*text);
        }
        g_state->textures.erase(text);
        if (textures[i] == g_state->currentBoundTexture) {
            g_state->currentBoundTexture = 0;
        }
    }
}

void glBindTexture( GLenum target, GLuint texture ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::BIND_TEXTURE;
        comm.enum1 = target;
        comm.uint1 = texture;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    gfx_texture *text = NULL;
    for(unsigned int i = 0; i < g_state->textures.size(); i++) {
        if(g_state->textures[i].tname == texture) {
            text = &g_state->textures[i];
            break;
        }
    }

    if(!text) {
        text = new gfx_texture(texture, target);
        g_state->textures.push(*text);
        glBindTexture(target, texture);
        return;
    }

    if(target != GL_TEXTURE_2D && target != GL_TEXTURE_CUBE_MAP) {
        setError(GL_INVALID_ENUM);
        return;
    }



    if(text->target == 0 ) {
        text->target = target;

    } else if(text->target != target) {
        setError(GL_INVALID_OPERATION);
        return;
    }

    g_state->currentBoundTexture = text->tname;
}

void glTexImage2D( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::TEX_IMAGE_2D;
        comm.enum1 = target;
        comm.int1 = level;
        comm.int2 = internalFormat;
        comm.size1 = width;
        comm.size2 = height;
        comm.int2 = border;
        comm.enum2 = format;
        comm.enum3 = type;
        comm.voidp = (GLvoid *)pixels;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    if(target != GL_TEXTURE_2D) {
        setError(GL_INVALID_ENUM);
        return;
    }

    switch(format) {
        case (GL_ALPHA):
        case (GL_RGB):
        case (GL_RGBA):
        case (GL_LUMINANCE):
        case (GL_LUMINANCE_ALPHA): {

        } break;

        default: {
            setError(GL_INVALID_VALUE);
            return;
        } break;

    }

    switch(type) {
        case (GL_UNSIGNED_BYTE):
        case (GL_UNSIGNED_SHORT_5_6_5):
        case (GL_UNSIGNED_SHORT_4_4_4_4):
        case (GL_UNSIGNED_SHORT_5_5_5_1): {

        } break;

        default: {
            setError(GL_INVALID_ENUM);
            return;
        } break;

    }

    if(level < 0 || level > log2(IMPL_MAX_TEXTURE_SIZE)) {
        setError(GL_INVALID_VALUE);
        return;
    }

    //Not really needed, duplicate functionailty of switch(format). Here for spec reasons.
    switch(internalFormat) {
        case (GL_ALPHA):
        case (GL_RGB):
        case (GL_RGBA):
        case (GL_LUMINANCE):
        case (GL_LUMINANCE_ALPHA): {

        } break;

        default: {
            setError(GL_INVALID_VALUE);
            return;
        } break;

    }

    if(width < 0 || height < 0
       || width > IMPL_MAX_TEXTURE_SIZE
       || height > IMPL_MAX_TEXTURE_SIZE
       || width &  0x00000001
       || height & 0x00000001) {
        setError(GL_INVALID_VALUE);
        return;
    }

    if(border != 0) {
        setError(GL_INVALID_VALUE);
        return;
    }

    if((GLenum)internalFormat != format) {
        setError(GL_INVALID_OPERATION);
        return;
    }

    if(type == GL_UNSIGNED_SHORT_5_6_5
       && format != GL_RGB) {
        setError(GL_INVALID_OPERATION);
        return;
    }

    if((type == GL_UNSIGNED_SHORT_4_4_4_4 || type == GL_UNSIGNED_SHORT_5_5_5_1)
       && format != GL_RGBA) {
        setError(GL_INVALID_OPERATION);
        return;
    }

    gfx_texture* text = NULL;
    for(unsigned int i = 0; i < g_state->textures.size(); i++) {
        if(g_state->textures[i].tname == g_state->currentBoundTexture) {
            text = &g_state->textures[i];
            break;
        }
    }

    if (!text) return;

    text->unpackedColorBuffer = (GLubyte*)gfx_platform_texture_malloc(sizeof(GLubyte) * 4 * width * height);
    text->width = width;
    text->height = height;
    text->format = format;

    if(pixels) {
        unsigned int accum = 0;
        //TODO implement unpacking for more texture formats
        for(GLsizei y = 0; y < height; y++) {
            for(GLsizei x = 0; x < width; x++) {
                switch(format) {
                    case (GL_RGBA): {
                        if(type == GL_UNSIGNED_BYTE) {
                            int index = (x + y * width) * 4;
                            GLubyte* bpixels = (GLubyte*)pixels;
                            text->unpackedColorBuffer[index] = bpixels[accum];
                            text->unpackedColorBuffer[index + 1] = bpixels[accum + 1];
                            text->unpackedColorBuffer[index + 2] = bpixels[accum + 2];
                            text->unpackedColorBuffer[index + 3] = bpixels[accum + 3];
                            accum += 4;
                        }
                        else if (type == GL_UNSIGNED_SHORT_5_5_5_1) {
                            int index = (x + y * width) * 4;
                            GLushort pixel = ((GLushort*)pixels)[accum];
                            float R = (float)((pixel >> 11) & 0b11111) / 31.0f;
                            float G = (float)((pixel >> 6) & 0b11111) / 31.0f;
                            float B = (float)((pixel >> 1) & 0b11111) / 31.0f;
                            text->unpackedColorBuffer[index + 0] = (GLubyte)(R *  255.0f);
                            text->unpackedColorBuffer[index + 1] = (GLubyte)(G *  255.0f);
                            text->unpackedColorBuffer[index + 2] = (GLubyte)(B *  255.0f);
                            text->unpackedColorBuffer[index + 3] = 0xFF * (pixel & 1);
                            accum ++;
                        }
                    } break;

                    case (GL_ALPHA): {
                        if(type == GL_UNSIGNED_BYTE) {
                            int index = (x + y * width) * 4;
                            GLubyte* bpixels = (GLubyte*)pixels;
                            text->unpackedColorBuffer[index] = 0x00;
                            text->unpackedColorBuffer[index + 1] = 0x00;
                            text->unpackedColorBuffer[index + 2] = 0x00;
                            text->unpackedColorBuffer[index + 3] = bpixels[accum];
                            accum++;
                        }
                    } break;

                    default: {

                        setError(GL_INVALID_VALUE);
                        return;
                    } break;

                }
            }

            while(accum % g_state->unpackAlignment != 0) {
                accum++;
            }
        }
    }
    g_state->device->repack_texture(*text);
}

void glTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels ) {
    CHECK_NULL(g_state);

    //TODO implement algorithm for updating a tiled image

    if(target != GL_TEXTURE_2D) {
        setError(GL_INVALID_ENUM);
        return;
    }

    switch (format) {
        case GL_COLOR_INDEX:
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
        case GL_ALPHA:
        case GL_RGB:
        case GL_RGBA:
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA: {

        } break;

        default: {
            setError(GL_INVALID_ENUM);
            return;
        } break;
    }

    switch (type) {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
        case GL_BITMAP:
        case GL_UNSIGNED_SHORT:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_SHORT:
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT: {

        } break;

        default: {
            setError(GL_INVALID_ENUM);
            return;
        } break;
    }

    if (type == GL_BITMAP && format != GL_COLOR_INDEX) {
        setError(GL_INVALID_ENUM);
        return;
    }

    gfx_texture* text = NULL;
    for(unsigned int i = 0; i < g_state->textures.size(); i++) {
        if(g_state->textures[i].tname == g_state->currentBoundTexture) {
            text = &g_state->textures[i];
            break;
        }
    }

    if (!text) {
        setError(GL_INVALID_OPERATION);
        return;
    }

    if(pixels) {
        unsigned int accum = 0;
        //TODO implement unpacking for more texture formats
        for(GLsizei y = yoffset; y < yoffset + height && y < text->height; ++y) {
            for(GLsizei x = xoffset; x < yoffset + width && x < text->width; ++x) {
                switch(format) {
                    case (GL_RGBA): {
                        if(type == GL_UNSIGNED_BYTE) {
                            int index = (x + y * text->width) * 4;
                            int srcindex = ((x - xoffset) + y * width) * 4;
                            GLubyte* bpixels = (GLubyte*)pixels;
                            text->unpackedColorBuffer[index] = bpixels[srcindex];
                            text->unpackedColorBuffer[index + 1] = bpixels[srcindex + 1];
                            text->unpackedColorBuffer[index + 2] = bpixels[srcindex + 2];
                            text->unpackedColorBuffer[index + 3] = bpixels[srcindex + 3];
                            accum += 4;
                        }
                        else if (type == GL_UNSIGNED_SHORT_5_5_5_1) {
                            int index = (x + y * width) * 4;
                            GLushort pixel = ((GLushort*)pixels)[accum];
                            float R = (float)((pixel >> 11) & 0b11111) / 31.0f;
                            float G = (float)((pixel >> 6) & 0b11111) / 31.0f;
                            float B = (float)((pixel >> 1) & 0b11111) / 31.0f;
                            text->unpackedColorBuffer[index + 0] = (GLubyte)(R *  255.0f);
                            text->unpackedColorBuffer[index + 1] = (GLubyte)(G *  255.0f);
                            text->unpackedColorBuffer[index + 2] = (GLubyte)(B *  255.0f);
                            text->unpackedColorBuffer[index + 3] = 0xFF * (pixel & 1);
                            accum ++;
                        }
                    } break;

                    case (GL_ALPHA): {
                        if(type == GL_UNSIGNED_BYTE) {
                            int index = (x + y * text->width) * 4;
                            int srcindex = ((x - xoffset) + y * width);
                            GLubyte* bpixels = (GLubyte*)pixels;
                            text->unpackedColorBuffer[index] = 0x00;
                            text->unpackedColorBuffer[index + 1] = 0x00;
                            text->unpackedColorBuffer[index + 2] = 0x00;
                            text->unpackedColorBuffer[index + 3] = bpixels[srcindex];
                            accum++;
                        }
                    } break;

                }
            }

            while(accum % g_state->unpackAlignment != 0) {
                accum++;
            }
        }
    }

    g_state->device->repack_texture(*text);
}

void glPixelStorei( GLenum pname, GLint param ) {
    CHECK_NULL(g_state);

    if(param != 1 && param != 2 && param != 4 && param != 8) {
        setError(GL_INVALID_VALUE);
        return;
    }

    switch(pname) {
        case (GL_PACK_ALIGNMENT): {
            g_state->packAlignment = param;
        } break;
        case (GL_UNPACK_ALIGNMENT): {
            g_state->unpackAlignment = param;
        } break;

        default: {
            setError(GL_INVALID_ENUM);
            return;
        }
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

void glViewport( GLint x, GLint y, GLsizei width, GLsizei height ) {
    CHECK_NULL(g_state);

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

    if (width < 0 || height < 0) {
        setError(GL_INVALID_VALUE);
        return;
    }

    float h = g_state->device->getHeight();
    float w = g_state->device->getWidth();
    float vw = (float)width;
    float vh = (float)height;
    g_state->viewportMatrix = mat4::viewport(((float)x / w), (float)y / h,  vw / (float)w, vh / (float)h);
}

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

void glEnable( GLenum cap ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::ENABLE;
        comm.enum1 = cap;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

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
            
        default: {
            setError(GL_INVALID_ENUM);
            return;
        }
    }
}

void glDisable( GLenum cap ) {
    CHECK_NULL(g_state);
    
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::DISABLE;
        comm.enum1 = cap;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }
    
    CHECK_COMPILE_AND_EXECUTE(g_state);
    
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

        default: {
            setError(GL_INVALID_ENUM);
            return;
        }
    }
}

void glTexParameteri( GLenum target, GLenum pname, GLint param ) {
    CHECK_NULL(g_state);
    
    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::TEX_PARAM_I;
        comm.enum1 = target;
        comm.enum2 = pname;
        comm.int1 = param;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }
    
    CHECK_COMPILE_AND_EXECUTE(g_state);
    
    switch (target) {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP: {
            
        } break;
            
        default: {
            setError(GL_INVALID_ENUM);
            return;
        }
    }
    
    switch (pname) {
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_MAG_FILTER: {
            switch (param) {
                case GL_LINEAR:
                case GL_NEAREST: {
                    
                } break;
                    
                default: {
                    setError(GL_INVALID_ENUM);
                    return;
                } break;
            }
        } break;
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T: {
            switch (param) {
                case GL_CLAMP_TO_EDGE:
//                case GL_MIRRORED_REPEAT:
                case GL_REPEAT: {
                } break;
                    
                default: {
                    setError(GL_INVALID_ENUM);
                    return;
                } break;
            }
        } break;
            
        default: {
            setError(GL_INVALID_ENUM);
            return;
        }
    }
    
    gfx_texture* text = NULL;
    for(unsigned int i = 0; i < g_state->textures.size(); i++) {
        if(g_state->textures[i].tname == g_state->currentBoundTexture) {
            text = &g_state->textures[i];
            break;
        }
    }
    
    if (!text) return;
    
    switch (pname) {
        case GL_TEXTURE_MIN_FILTER: {
            text->min_filter = param;
        } break;
        case GL_TEXTURE_MAG_FILTER: {
            text->mag_filter = param;
        } break;
        case GL_TEXTURE_WRAP_S: {
            text->wrap_s = param;
        } break;
        case GL_TEXTURE_WRAP_T: {
            text->wrap_t = param;
        } break;
            
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
    
    if (list == 0) {
        setError(GL_INVALID_VALUE);
        return;
    }
    
    switch (mode) {
        case GL_COMPILE:
        case GL_COMPILE_AND_EXECUTE: {
            g_state->currentDisplayList = list;
            g_state->newDisplayListMode = mode;
            g_state->withinNewEndListBlock = GL_TRUE;
            getList(list)->commands.clear();
        } break;
            
        default: {
            setError(GL_INVALID_ENUM);
            return;
        }
    }
}

void glEndList( void ) {
    CHECK_NULL(g_state);
    CHECK_WITHIN_BEGIN_END(g_state);
    
    if (!g_state->withinNewEndListBlock) {
        setError(GL_INVALID_OPERATION);
        return;
    }
    
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

void glLightf( GLenum light, GLenum pname, GLfloat param ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::LIGHTF;
        comm.enum1 = light;
        comm.enum2 = pname;
        comm.floats[0] = param;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);

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

    GLuint light_index = light - GL_LIGHT0;

    switch (pname) {
        case GL_SPOT_EXPONENT: {
            if (param < 0.0 || param > 128.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
            g_state->lights[light_index].spotlightExpo = param;
        } break;

        case GL_SPOT_CUTOFF: {
            if ((param < 0.0 || param > 90.0) && param != 180.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
            g_state->lights[light_index].spotlightCutoff = param;
        } break;

        case GL_CONSTANT_ATTENUATION: {
            if (param < 0.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
             g_state->lights[light_index].constantAttenuation = param;
        } break;

        case GL_LINEAR_ATTENUATION: {
            if (param < 0.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
            g_state->lights[light_index].linearAttenuation = param;
        } break;

        case GL_QUADRATIC_ATTENUATION: {
            if (param < 0.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
            g_state->lights[light_index].quadraticAttenuation = param;
        } break;

        default: {
            setError(GL_INVALID_ENUM);
        } return;
    }

}

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

void glLightfv( GLenum light, GLenum pname, const GLfloat *params ) {
    CHECK_NULL(g_state);

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

    CHECK_WITHIN_BEGIN_END(g_state);

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

    GLuint light_index = light - GL_LIGHT0;

    switch (pname) {
        case GL_SPOT_EXPONENT: {
            if (params[0] < 0.0 || params[0] > 128.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
            g_state->lights[light_index].spotlightExpo = params[0];
        } break;

        case GL_SPOT_CUTOFF: {
            if ((params[0] < 0.0 || params[0] > 90.0) && params[0] != 180.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
            g_state->lights[light_index].spotlightCutoff = params[0];
        } break;

        case GL_CONSTANT_ATTENUATION: {
            if (params[0] < 0.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
            g_state->lights[light_index].constantAttenuation = params[0];
        } break;

        case GL_LINEAR_ATTENUATION: {
            if (params[0] < 0.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
            g_state->lights[light_index].linearAttenuation = params[0];
        } break;

        case GL_QUADRATIC_ATTENUATION: {
            if (params[0] < 0.0) {
                setError(GL_INVALID_VALUE);
                return;
            }
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
            
        default: {
            setError(GL_INVALID_ENUM);
        } return;
    }
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

void glDepthMask( GLboolean flag ) {
    CHECK_NULL(g_state);

    if (g_state->withinNewEndListBlock && g_state->displayListCallDepth == 0) {
        gfx_command comm;
        comm.type = gfx_command::DEPTH_MASK;
        comm.uint1 = flag;
        getList(g_state->currentDisplayList)->commands.push_back(comm);
    }

    CHECK_COMPILE_AND_EXECUTE(g_state);

    CHECK_WITHIN_BEGIN_END(g_state);

    g_state->depthMask = flag != 0;
}

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

}
