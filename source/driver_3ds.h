#ifndef DRIVER_3DS_H
#define DRIVER_3DS_H

#include "gfx_device_internal.h"
#include "glImpl.h"

#include <3ds.h>
#include <GL/glext.h>

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
   virtual u8 *cache_vertex_list(GLuint *size);

};

#endif
