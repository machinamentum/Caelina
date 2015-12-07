#ifndef DRIVER_3DS_H
#define DRIVER_3DS_H

#include "gfx_device_internal.h"
#include "glImpl.h"

#include <3ds.h>
#include <GL/glext.h>
#include <GL/ctr.h>

/* PICA200 extension state */
struct gfx_device_3ds_ext {

    GPU_SCISSORMODE scissorMode = GPU_SCISSOR_NORMAL;
};

struct gfx_device_3ds : public gfx_device {
    u32 *gpuDOut;
    u32 *gpuOut;
    gfx_device_3ds_ext ext_state;

    gfx_device_3ds(gfx_state *state, int w, int h);
    ~gfx_device_3ds();
    void clear(float r, float g, float b, float a);
    void clearDepth(GLfloat depth);
    void flush(u8* fb, int w, int h, int f);
    void render_vertices(const mat4& projection, const mat4& modelview);
    void render_vertices_vbo(const mat4& projection, const mat4& modelview, u8 *data, GLuint units);
    void render_vertices_array(GLenum mode, GLint first, GLsizei count, const mat4& projection, const mat4& modelview);
    void repack_texture(gfx_texture& tex);
    void free_texture(gfx_texture& tex);
    u8 *cache_vertex_list(GLuint *size);
    void setup_state(const mat4& projection, const mat4& modelview);
};

#endif
