
#include "driver_3ds.h"
#include <3ds.h>

#include <cstring>
#include "default_3ds_vsh_shbin.h"

#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	 GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	 GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_X))

struct _3ds_vec3 {
	float x, y, z;
};

struct _3ds_vertex {
	_3ds_vec3 pos;
	vec4 color;
};

struct VBO {
   u8* data;
	u32 currentSize; // in bytes
	u32 maxSize; // in bytes
	u32 numVertices;
	u32* commands;
	u32 commandsSize;

   VBO(u32 size) {
      data = (u8 *)linearAlloc(size * sizeof(_3ds_vertex));
      currentSize = 0;
      maxSize = size;
      numVertices = 0;
      commands = NULL;
      commandsSize = 0;
   }

   ~VBO() {
      linearFree(data);
   }

   int set_data(sbuffer<vertex>& vdat, bool color) {
		_3ds_vertex *ver = (_3ds_vertex *)data;
		for (int i = 0; i < vdat.size(); ++i) {
			if (color) {
				ver[i].color = vec4(vdat[i].color);
			} else {
				ver[i].color = vec4(vdat[i].textureCoord);
			}
			ver[i].pos.x = vdat[i].position.x;
			ver[i].pos.y = vdat[i].position.y;
			ver[i].pos.z = vdat[i].position.z;
		}
      currentSize += vdat.size() * sizeof(_3ds_vertex);
      numVertices += vdat.size();
      return 0;
   }

};

extern "C" {
	void set_attr(u32 *loc);
};

#undef GPUCMD_AddSingleParam
static void GPUCMD_AddSingleParam(u32 header, u32 param) {
   GPUCMD_Add(header, &param, 1);
}

static
void GPU_DrawArrayDirectly(GPU_Primitive_t primitive, u8* data, u32 n)
{
	//set attribute buffer address
	GPUCMD_AddSingleParam(0x000F0200, (osConvertVirtToPhys((u32)data))>>3);
	//set primitive type
	GPUCMD_AddSingleParam(0x0002025E, primitive);
	GPUCMD_AddSingleParam(0x0002025F, 0x00000001);
	//index buffer not used for drawArrays but 0x000F0227 still required
	GPUCMD_AddSingleParam(0x000F0227, 0x80000000);
	//pass number of vertices
	GPUCMD_AddSingleParam(0x000F0228, n);

	GPUCMD_AddSingleParam(0x00010253, 0x00000001);

	GPUCMD_AddSingleParam(0x00010245, 0x00000000);
	GPUCMD_AddSingleParam(0x000F022E, 0x00000001);
	GPUCMD_AddSingleParam(0x00010245, 0x00000001);
	GPUCMD_AddSingleParam(0x000F0231, 0x00000001);

	// GPUCMD_AddSingleParam(0x000F0111, 0x00000001); //breaks stuff
}

static
void SetAttributeBuffers(u8 totalAttributes, u32* baseAddress,
                     u64 attributeFormats, u16 attributeMask,
                     u64 attributePermutation, u8 numBuffers,
                     u32 bufferOffsets, u64 bufferPermutations, u8 bufferNumAttributes) {
   GPU_SetAttributeBuffers(totalAttributes, baseAddress, attributeFormats, attributeMask, attributePermutation, numBuffers,
                           &bufferOffsets, &bufferPermutations, &bufferNumAttributes);
}

static //stolen from staplebutt & smea :P
void GPU_SetDummyTexEnv(u8 num)
{
	GPU_SetTexEnv(num,
		GPU_TEVSOURCES(GPU_PREVIOUS, 0, 0),
		GPU_TEVSOURCES(GPU_PREVIOUS, 0, 0),
		GPU_TEVOPERANDS(0,0,0),
		GPU_TEVOPERANDS(0,0,0),
		GPU_REPLACE,
		GPU_REPLACE,
		0xFFFFFFFF);
}

gfx_device_3ds::gfx_device_3ds(gfx_state *state, int w, int h) : gfx_device(state, w, h) {
   GPU_Init(NULL);
   gpuCmd = (u32*)linearAlloc(gpuCmdSize*4);
   gpuCmdRight = (u32*)linearAlloc(gpuCmdSize*4);
   gpuOut=(u32*)vramMemAlign(height*240*8, 0x100);;
   gpuDOut=(u32*)vramMemAlign(height*240*8, 0x100);;
   GPU_Reset(NULL, gpuCmd, gpuCmdSize);

   dvlb=DVLB_ParseFile((u32*)default_3ds_vsh_shbin, default_3ds_vsh_shbin_size);
	shaderProgramInit(&shader);
	shaderProgramSetVsh(&shader, &dvlb->DVLE[0]);
   shaderProgramUse(&shader);

   GPUCMD_Finalize();
	GPUCMD_FlushAndRun(NULL);
	gspWaitForP3D();

}

gfx_device_3ds::~gfx_device_3ds() {

}

void gfx_device_3ds::clearDepth(GLdouble d) {

}

static u8 tileOrder[] = {0,1,8,9,2,3,10,11,16,17,24,25,18,19,26,27,4,5,12,13,6,7,14,15,20,21,28,29,22,23,30,31,32,33,40,41,34,35,42,43,48,49,56,57,50,51,58,59,36,37,44,45,38,39,46,47,52,53,60,61,54,55,62,63};

static
unsigned long htonl(unsigned long v)
{
	u8* v2=(u8*)&v;
	return (v2[0]<<24)|(v2[1]<<16)|(v2[2]<<8)|(v2[3]);
}

static void tileImage32(u32* src, u32* dst, int width, int height)
{
	if(!src || !dst)return;

	int i, j, k, l;
	l=0;
	for(j=0; j<height; j+=8)
	{
		for(i=0; i<width; i+=8)
		{
			for(k=0; k<8*8; k++)
			{
				int x=i+tileOrder[k]%8;
				int y=j+(tileOrder[k]-(x-i))/8;
				u32 v=src[x+(height-1-y)*width];
				dst[l++]=htonl(v);
			}
		}
	}
}


void gfx_device_3ds::repack_texture(gfx_texture &tex) {
	u32 size = 4*tex.width*tex.height;
	size = ((size - (size >> (2*(0+1)))) * 4) / 3;
	u32 *dst = (u32 *)linearMemAlign(size, 0x80);
	tileImage32((u32*)tex.colorBuffer, dst, tex.width, tex.height);
	linearFree(tex.colorBuffer);
	tex.colorBuffer = (u8*)dst;
}

static GPU_BLENDFACTOR gl_blendfactor(GLenum factor) {
	switch(factor) {
		case GL_ZERO: return GPU_ZERO;
		case GL_ONE:  return GPU_ONE;
		case GL_SRC_COLOR: return GPU_SRC_COLOR;
		case GL_ONE_MINUS_SRC_COLOR: return GPU_ONE_MINUS_SRC_COLOR;
		case GL_DST_COLOR: return GPU_DST_COLOR;
		case GL_ONE_MINUS_DST_COLOR: return GPU_ONE_MINUS_DST_COLOR;
		case GL_SRC_ALPHA: return GPU_SRC_ALPHA;
		case GL_ONE_MINUS_SRC_ALPHA: return GPU_ONE_MINUS_SRC_ALPHA;
		case GL_DST_ALPHA: return GPU_DST_ALPHA;
		case GL_ONE_MINUS_DST_ALPHA: return GPU_ONE_MINUS_DST_ALPHA;
		case GL_CONSTANT_COLOR: return GPU_CONSTANT_COLOR;
		case GL_ONE_MINUS_CONSTANT_COLOR: return GPU_ONE_MINUS_CONSTANT_COLOR;
		case GL_CONSTANT_ALPHA: return GPU_CONSTANT_ALPHA;
		case GL_ONE_MINUS_CONSTANT_ALPHA: return GPU_ONE_MINUS_CONSTANT_ALPHA;
		case GL_SRC_ALPHA_SATURATE: return GPU_SRC_ALPHA_SATURATE;
	}
}

static GPU_Primitive_t gl_primitive(GLenum mode) {
	switch(mode) {
		case GL_QUADS:
		case GL_TRIANGLES: return GPU_TRIANGLES;
		case GL_TRIANGLE_STRIP: return GPU_TRIANGLE_STRIP;
		default: return GPU_UNKPRIM;
	}
}

static GPU_TEXTURE_WRAP_PARAM gl_tex_wrap(GLenum wrap) {
	switch (wrap) {
		case GL_CLAMP_TO_EDGE: return GPU_CLAMP_TO_EDGE;
		// case GL_MIRRORED_REPEAT: return GPU_MIRRORED_REPEAT;
		case GL_REPEAT: return GPU_REPEAT;
	}
}

static GPU_TEXTURE_FILTER_PARAM gl_tex_filter(GLenum filt) {
	switch (filt) {
		case GL_LINEAR: return GPU_LINEAR;
		case GL_NEAREST: return GPU_NEAREST;
	}
}

void gfx_device_3ds::render_vertices(const mat4& mvp) {
	GPUCMD_SetBufferOffset(0);
	shaderProgramUse(&shader);
	// matrix_gpu_set_uniform(ortho_matrix_top, projection_desc);
	// float colors[4] = {1.0f, 0.0f, 1.0f, 1.0f};
   // GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(shader.vertexShader, "colors"), (u32*)colors, 1);
	float mu[4*4];
	mat4 pica = mat4();
	pica[0xA] = 0.5;
	pica[0xB] = -0.5;

	pica = pica * g_state->viewportMatrix * mvp;
	int i, j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			mu[i*4 + j] = pica[i*4 + (3-j)];
		}
	}

	GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(shader.vertexShader, "projection"), (u32*)mu, 4);

	GPU_SetViewport((u32 *)osConvertVirtToPhys((u32)gpuDOut),
		(u32 *)osConvertVirtToPhys((u32)gpuOut),
		0, 0, 240, height);
	{
		GLint x = g_state->scissorBox.x;
		GLint y = g_state->scissorBox.y;
		GLint w = g_state->scissorBox.z;
		GLint h = g_state->scissorBox.w;
		GPU_SetScissorTest((g_state->enableScissorTest ? scissorMode : GPU_SCISSOR_DISABLE), x, y, x + w, y + h);
	}

	GPU_DepthMap(-1.0f, 0.0f);
	GPU_SetFaceCulling(GPU_CULL_NONE);
	GPU_SetStencilTest(false, GPU_ALWAYS, 0x00, 0xFF, 0x00);
	GPU_SetStencilOp(GPU_KEEP, GPU_KEEP, GPU_KEEP);
	GPU_SetBlendingColor(0,0,0,0);
	GPU_SetDepthTestAndWriteMask(g_state->enableDepthTest, GPU_GEQUAL, GPU_WRITE_ALL);
	GPUCMD_AddMaskedWrite(GPUREG_0062, 0x1, 0);
	GPUCMD_AddWrite(GPUREG_0118, 0);

	GPU_SetAlphaBlending(
		GPU_BLEND_ADD,
		GPU_BLEND_ADD,
		gl_blendfactor(g_state->blendSrcFactor), gl_blendfactor(g_state->blendDstFactor),
		gl_blendfactor(g_state->blendSrcFactor), gl_blendfactor(g_state->blendDstFactor)
	);

	GPU_SetAlphaTest(false, GPU_ALWAYS, 0x00);
	VBO vbo = VBO(g_state->vertexBuffer.size());

	//=========================================
	if (!g_state->enableTexture2D) {
		GPU_SetTexEnv(
			0,
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
			GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_TEVOPERANDS(0, 0, 0),
			GPU_REPLACE, GPU_REPLACE,
			0xFFFFFFFF
		);
		vbo.set_data(g_state->vertexBuffer, true);
	} else {
		GPU_SetTextureEnable(GPU_TEXUNIT0);
		GPU_SetTexEnv(0,
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_TEXTURE0, GPU_TEXTURE0),
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_TEXTURE0, GPU_TEXTURE0),
		GPU_TEVOPERANDS(0,0,0),
		GPU_TEVOPERANDS(0,0,0),
		GPU_MODULATE, GPU_MODULATE,
		0xFFFFFFFF);

		gfx_texture* text = NULL;
		for(unsigned int i = 0; i < g_state->textures.size(); i++) {
			if(g_state->textures[i].tname == g_state->currentBoundTexture) {
				text = &g_state->textures[i];
				break;
			}
		}
		if (text) {

			GPU_SetTexture(
				GPU_TEXUNIT0, //texture unit
				(u32*)osConvertVirtToPhys((u32)text->colorBuffer), //data buffer
				text->width, //texture width
				text->height, //texture height
				GPU_TEXTURE_MAG_FILTER(gl_tex_filter(text->mag_filter)) |
				GPU_TEXTURE_MIN_FILTER(gl_tex_filter(text->min_filter)) |
				GPU_TEXTURE_WRAP_S(gl_tex_wrap(text->wrap_s)) |
				GPU_TEXTURE_WRAP_T(gl_tex_wrap(text->wrap_t)), //texture params
				GPU_RGBA8 //texture pixel format
			);
			vbo.set_data(g_state->vertexBuffer, false);
		}
	}

	GPU_SetDummyTexEnv(1);
	GPU_SetDummyTexEnv(2);
	GPU_SetDummyTexEnv(3);
	GPU_SetDummyTexEnv(4);
	GPU_SetDummyTexEnv(5);


	SetAttributeBuffers(
		2, // number of attributes
		(u32*)osConvertVirtToPhys((u32)vbo.data),
		GPU_ATTRIBFMT(0, 3, GPU_FLOAT) | GPU_ATTRIBFMT(1, 4, GPU_FLOAT),
		0xFFFC, //0b1100
		0x10,
		1, //number of buffers
		{0x0}, // buffer offsets (placeholders)
		{0x10}, // attribute permutations for each buffer
		{2} // number of attributes for each buffer
	);
		// set_attr((u32*)osConvertVirtToPhys((u32)vbo.data));

	// GSPGPU_FlushDataCache(NULL, (u8*)&g_state->vertexBuffer[0], g_state->vertexBuffer.size() * sizeof(vertex));
   GPU_DrawArray(gl_primitive(g_state->vertexDrawMode), vbo.numVertices);
	GPU_FinishDrawing();
	GPUCMD_Finalize();
	GPUCMD_FlushAndRun(NULL);
	gspWaitForP3D();

}

void gfx_device_3ds::flush(u8 *fb) {

   // u32 offset; GPUCMD_GetBuffer(NULL, NULL, &offset);
   // memcpy(gpuCmdRight, gpuCmd, offset*4);
   // /we wait for the left buffer to finish drawing
   // GX_SetDisplayTransfer(NULL, (u32*)gpuOut, GX_BUFFER_DIM(240*2, width), (u32 *)fb, GX_BUFFER_DIM(240*2, width), DISPLAY_TRANSFER_FLAGS);
	GX_SetDisplayTransfer(NULL, (u32*)gpuOut, GX_BUFFER_DIM(240, height),
		(u32*)fb,
		GX_BUFFER_DIM(240, height), 0x1000);
}
#define RGBA8(r,g,b,a) ((((r)&0xFF)<<24) | (((g)&0xFF)<<16) | (((b)&0xFF)<<8) | (((a)&0xFF)<<0))
void gfx_device_3ds::clear(u8 r, u8 g, u8 b, u8 a) {

   GX_SetMemoryFill(NULL, (u32*)gpuOut, RGBA8(r,g,b,a), (u32*)&gpuOut[0x2EE00], 0x201, (u32*)gpuDOut, 0x00000000, (u32*)&gpuDOut[0x2EE00], 0x201);
	gspWaitForPSC0();
}
