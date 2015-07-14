#ifndef DRIVER_3DS_H
#define DRIVER_3DS_H

#include "gfx_device_internal.h"

#include <3ds.h>

class gfx_device_3ds : public gfx_device {
   u32 gpuCmdSize=0x40000;
	u32 *gpuCmd;
	u32 *gpuCmdRight;
   u32 *gpuDOut;
   u32 *gpuOut;
   shaderProgram_s shader;
   DVLB_s* dvlb;

	/* PICA200 extension state */
	GPU_SCISSORMODE scissorMode = GPU_SCISSOR_NORMAL;

public:
   gfx_device_3ds(gfx_state *state, int w, int h);
   virtual ~gfx_device_3ds();
   virtual void clear(u8 r, u8 g, u8 b, u8 a);
   virtual void clearDepth(GLdouble depth);
   virtual void flush(u8* fb);
   virtual void render_vertices(const mat4& mvp);
	virtual void repack_texture(gfx_texture& tex);

};

#endif
