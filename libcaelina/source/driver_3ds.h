#ifndef DRIVER_3DS_H
#define DRIVER_3DS_H

#include "gfx_device_internal.h"
#include "glImpl.h"

#include <3ds.h>
#include <GL/glext.h>
#include <GL/ctr.h>

struct _3ds_vec3 {
    float x, y, z;
};

struct _3ds_vertex {
    _3ds_vec3 pos;
    vec4 texCoord;
    vec4 color;
    vec4 normal;
};


struct VBO {
    u8* data;
    u32 currentSize; // in bytes
    u32 maxSize; // in bytes
    u32 numVertices;

    VBO(u32 size) {
        data = (u8 *)linearAlloc(size * sizeof(_3ds_vertex));
        currentSize = 0;
        maxSize = size * sizeof(_3ds_vertex);
        numVertices = 0;
    }

    ~VBO() {

    }

    int set_data(sbuffer<vertex>& vdat) {
        currentSize = vdat.size() * sizeof(_3ds_vertex);
        numVertices = vdat.size();
        if (currentSize > maxSize) return -1;
        _3ds_vertex *ver = (_3ds_vertex *)data;
        for (unsigned int i = 0; i < vdat.size(); ++i) {
            ver[i].color = vec4(vdat[i].color);
            ver[i].texCoord = vec4(vdat[i].textureCoord);
            ver[i].pos.x = vdat[i].position.x;
            ver[i].pos.y = vdat[i].position.y;
            ver[i].pos.z = vdat[i].position.z;
            ver[i].normal = vec4(vdat[i].normal);
        }

        return 0;
    }

};

/* PICA200 extension state */
struct gfx_device_3ds_ext {

    GLenum scissorMode = GL_SCISSOR_NORMAL_DMP;
};

class gfx_device_3ds : public gfx_device {
    u32 gpuCmdSize=0x40000;
    u32 *gpuCmd;
    u32 *gpuDOut;
    u32 *gpuOut;
    shaderProgram_s shader;
    shaderProgram_s vertex_lighting_shader;
    DVLB_s* dvlb;
    VBO temp_vbo = VBO(10000); // approx ~10,000 vertices

    void setup_state(const mat4& projection, const mat4& modelview);

public:
    gfx_device_3ds_ext ext_state;

    gfx_device_3ds(gfx_state *state, int w, int h);
    virtual ~gfx_device_3ds();
    virtual void clear(u8 r, u8 g, u8 b, u8 a);
    virtual void clearDepth(GLdouble depth);
    virtual void flush(u8* fb);
    virtual void render_vertices(const mat4& projection, const mat4& modelview);
    virtual void render_vertices_vbo(const mat4& projection, const mat4& modelview, u8 *data, GLuint units);
    virtual void repack_texture(gfx_texture& tex);
    virtual void free_texture(gfx_texture& tex);
    virtual u8 *cache_vertex_list(GLuint *size);
    
};

#endif
