#include <3ds.h>
#include <3ds/gpu/gx.h>
#include "glImpl.h"
#include <cstring>
#include "default_3ds_vsh_shbin.h"
#include "clear_shader_vsh_shbin.h"
#include "vertex_lighting_3ds_vsh_shbin.h"

static
void SetAttributeBuffers(u8 totalAttributes, u32* baseAddress,
                         u64 attributeFormats, u16 attributeMask,
                         u64 attributePermutation, u8 numBuffers,
                         u32 bufferOffsets, u64 bufferPermutations, u8 bufferNumAttributes) {
    GPU_SetAttributeBuffers(totalAttributes, baseAddress, attributeFormats, attributeMask, attributePermutation, numBuffers,
                            &bufferOffsets, &bufferPermutations, &bufferNumAttributes);
}

static //stolen from smea who stole it from staplebutt :P
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

static u32 *gpuCmd = nullptr;
static u32 gpuCmdSize = 0;
static shaderProgram_s shader;
static shaderProgram_s clear_shader;
static shaderProgram_s vertex_lighting_shader;
static DVLB_s* dvlb_default = nullptr;
static DVLB_s* dvlb_lighting = nullptr;
static DVLB_s* dvlb_clear = nullptr;
static VBO *clearQuadVBO = nullptr;

gfx_device_3ds::gfx_device_3ds(gfx_state *state, int w, int h) : gfx_device(state, w, h) {
    if (!gpuCmd) {
      gpuCmdSize = 0x40000;
      gpuCmd = (u32*)linearAlloc(gpuCmdSize*4);
      GPU_Init(NULL);
      GPU_Reset(NULL, gpuCmd, gpuCmdSize);
      sbuffer<vertex> clearQuad;
      clearQuad.push(vertex(vec4(-1, -1)));
      clearQuad.push(vertex(vec4(1, -1)));
      clearQuad.push(vertex(vec4(1, 1)));
      clearQuad.push(vertex(vec4(-1, -1)));
      clearQuad.push(vertex(vec4(1, 1)));
      clearQuad.push(vertex(vec4(-1, 1)));
      clearQuadVBO = new VBO(clearQuad.size());
      clearQuadVBO->set_data(clearQuad);
    }

    if (!dvlb_default) {
      dvlb_default = DVLB_ParseFile((u32*)default_3ds_vsh_shbin, default_3ds_vsh_shbin_size);
      shaderProgramInit(&shader);
      shaderProgramSetVsh(&shader, &dvlb_default->DVLE[0]);

      dvlb_lighting = DVLB_ParseFile((u32*)vertex_lighting_3ds_vsh_shbin, vertex_lighting_3ds_vsh_shbin_size);
      shaderProgramInit(&vertex_lighting_shader);
      shaderProgramSetVsh(&vertex_lighting_shader, &dvlb_lighting->DVLE[0]);

      dvlb_clear = DVLB_ParseFile((u32*)clear_shader_vsh_shbin, clear_shader_vsh_shbin_size);
      shaderProgramInit(&clear_shader);
      shaderProgramSetVsh(&clear_shader, &dvlb_clear->DVLE[0]);
    }

    gpuOut=(u32*)vramAlloc(height*width*4);
    gpuDOut=(u32*)vramAlloc(height*height*4);
}

gfx_device_3ds::~gfx_device_3ds() {

}


// stolen from smea
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

static
void safeWaitForEvent(Handle event);
extern Handle gspEvents[GSPGPU_EVENT_MAX];

static Result GX_RequestDmaFlush(u32* src, u32* dst, u32 length)
{
    u32 gxCommand[0x8];
    gxCommand[0]=0x00; //CommandID
    gxCommand[1]=(u32)src; //source address
    gxCommand[2]=(u32)dst; //destination address
    gxCommand[3]=length; //size
    gxCommand[4]=gxCommand[5]=gxCommand[6]=gxCommand[7]=0x2;

    return gspSubmitGxCommand(gxCmdBuf, gxCommand);
}

void gfx_device_3ds::repack_texture(gfx_texture &tex) {
    u32 size = 4*tex.width*tex.height;
    size = ((size - (size >> (2*(0+1)))) * 4) / 3;
    u32 *dst = (u32 *)linearMemAlign(size, 0x80);
    tileImage32((u32*)tex.unpackedColorBuffer, dst, tex.width, tex.height);

    if (size > vramSpaceFree()) {
        tex.colorBuffer = (GLubyte*)dst;
        tex.extdata = 0;
    } else {
        if (!tex.colorBuffer) tex.colorBuffer = (GLubyte*)vramMemAlign(size, 0x80);
        GX_RequestDmaFlush(dst, (u32*)tex.colorBuffer, size);
        safeWaitForEvent(gspEvents[GSPGPU_EVENT_DMA]);
        linearFree(dst);
        tex.extdata = 1;
    }
}

void gfx_device_3ds::free_texture(gfx_texture &tex) {
    if (tex.extdata) {
        vramFree(tex.colorBuffer);
        linearFree(tex.unpackedColorBuffer);
    } else {
        linearFree(tex.unpackedColorBuffer);
        linearFree(tex.colorBuffer);
    }
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
        case GL_SRC_ALPHA_SATURATE: return GPU_SRC_ALPHA_SATURATE;
#if !defined(SPEC_GLES) || defined(SPEC_GLES2)
      case GL_CONSTANT_COLOR: return GPU_CONSTANT_COLOR;
      case GL_ONE_MINUS_CONSTANT_COLOR: return GPU_ONE_MINUS_CONSTANT_COLOR;
      case GL_CONSTANT_ALPHA: return GPU_CONSTANT_ALPHA;
      case GL_ONE_MINUS_CONSTANT_ALPHA: return GPU_ONE_MINUS_CONSTANT_ALPHA;
#endif
    }

    return GPU_ONE;
}

static GPU_Primitive_t gl_primitive(GLenum mode) {
    switch(mode) {
#ifndef SPEC_GLES
        case GL_QUADS:
#endif
        case GL_TRIANGLES: return GPU_TRIANGLES;
        case GL_TRIANGLE_STRIP: return GPU_TRIANGLE_STRIP;
        case GL_TRIANGLE_FAN: return GPU_TRIANGLE_FAN;
        default: return GPU_GEOMETRY_PRIM;
    }

    return GPU_GEOMETRY_PRIM;
}

static GPU_TESTFUNC gl_writefunc(GLenum func) {
    switch (func) {
        case GL_NEVER: return GPU_NEVER;
        case GL_LESS: return GPU_LESS;
        case GL_EQUAL: return GPU_EQUAL;
        case GL_LEQUAL: return GPU_LEQUAL;
        case GL_GREATER: return GPU_GREATER;
        case GL_NOTEQUAL: return GPU_NOTEQUAL;
        case GL_GEQUAL: return GPU_GEQUAL;
        case GL_ALWAYS: return GPU_ALWAYS;
    }

    return GPU_NEVER;
}

static GPU_STENCILOP gl_stencilop(GLenum func) {
    switch (func) {
        case GL_KEEP: return GPU_STENCIL_KEEP;
        case GL_ZERO: return GPU_STENCIL_ZERO;
        case GL_REPLACE: return GPU_STENCIL_REPLACE;
        case GL_INCR: return GPU_STENCIL_INCR;
#if !defined(SPEC_GLES) || defined(SPEC_GLES2)
        case GL_INCR_WRAP: return GPU_STENCIL_INCR_WRAP;
#endif
        case GL_DECR: return GPU_STENCIL_DECR;
#if !defined(SPEC_GLES) || defined(SPEC_GLES2)
        case GL_DECR_WRAP: return GPU_STENCIL_DECR_WRAP;
#endif
        case GL_INVERT: return GPU_STENCIL_INVERT;
    }

    return GPU_STENCIL_KEEP;
}

static GPU_TESTFUNC gl_depthfunc(GLenum func) {
  switch (func) {
    case GL_NEVER: return GPU_NEVER;
    case GL_LESS: return GPU_GREATER;
    case GL_EQUAL: return GPU_EQUAL;
    case GL_LEQUAL: return GPU_GEQUAL;
    case GL_GREATER: return GPU_LESS;
    case GL_NOTEQUAL: return GPU_NOTEQUAL;
    case GL_GEQUAL: return GPU_LEQUAL;
    case GL_ALWAYS: return GPU_ALWAYS;
  }

  return GPU_NEVER;
}

u8 *gfx_device_3ds::cache_vertex_list(GLuint *size) {
    VBO vbo = VBO(g_state->vertexBuffer.size());
    vbo.set_data(g_state->vertexBuffer);
    *size = vbo.currentSize;
    return vbo.data;
}

void gfx_device_3ds::setup_state(const mat4& projection, const mat4& modelview) {

    if (!g_state->enableLighting) {
        shaderProgramUse(&shader);
    } else {
        shaderProgramUse(&vertex_lighting_shader);
    }

    float mu_proj[4*4];
    float mu_model[4*4];
    mat4 pica = mat4();
    pica[0xA] = 0.5;
    pica[0xB] = -0.5;

    pica = pica * g_state->viewportMatrix * projection;
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            mu_proj[i*4 + j] = pica[i*4 + (3-j)];
        }
    }
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            mu_model[i*4 + j] = modelview.at(i*4 + (3-j));
        }
    }

    if (g_state->enableLighting) {
        float mu_normal[4*4];
        mat4 normal_mtx = mat4(modelview);
        normal_mtx[0 + 3] = 0.0;
        normal_mtx[4 + 3] = 0.0;
        normal_mtx[8 + 3] = 0.0;
        // normal_mtx = normal_mtx.inverse().transpose();
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                mu_normal[i*4 + j] = normal_mtx[i*4 + (3-j)];
            }
        }
        shaderInstanceSetBool(vertex_lighting_shader.vertexShader, 0, g_state->enableLight[0]);
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "projection"), (u32*)mu_proj, 4);
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "modelview"), (u32*)mu_model, 4);
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "normal_mtx"), (u32*)mu_normal, 4);

        gfx_light *light = &g_state->lights[0];
        vec4 vtemp = light->ambient;
        vtemp = {vtemp.w, vtemp.z, vtemp.y, vtemp.x};
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "light0_ambient"), (u32*)&vtemp[0], 1);
        vtemp = light->diffuse;
        vtemp = {vtemp.w, vtemp.z, vtemp.y, vtemp.x};
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "light0_diffuse"), (u32*)&vtemp[0], 1);
        vtemp = light->specular;
        vtemp = {vtemp.w, vtemp.z, vtemp.y, vtemp.x};
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "light0_specular"), (u32*)&vtemp[0], 1);
        vtemp = light->position;
        vtemp = {vtemp.w, vtemp.z, vtemp.y, vtemp.x};
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "light0_position"), (u32*)&vtemp[0], 1);
        vtemp = light->spotlightDirection;
        vtemp = {vtemp.w, vtemp.z, vtemp.y, vtemp.x};
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "light0_spotdir"), (u32*)&vtemp[0], 1);
        vtemp = {0.0f, light->spotlightExpo, cosf(light->spotlightCutoff), light->spotlightCutoff};
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "light0_spot_cutoff"), (u32*)&vtemp[0], 1);
        vtemp = {0.0f, light->quadraticAttenuation, cosf(light->linearAttenuation), light->constantAttenuation};
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "light0_attenuation"), (u32*)&vtemp[0], 1);

        //material
        gfx_material *mat = &g_state->material;
        vtemp = mat->ambientColor;
        vtemp = {vtemp.w, vtemp.z, vtemp.y, vtemp.x};
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "material_ambient"), (u32*)&vtemp[0], 1);
        vtemp = mat->diffuseColor;
        vtemp = {vtemp.w, vtemp.z, vtemp.y, vtemp.x};
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "material_diffuse"), (u32*)&vtemp[0], 1);
        vtemp = mat->specularColor;
        vtemp = {vtemp.w, vtemp.z, vtemp.y, vtemp.x};
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "material_specular"), (u32*)&vtemp[0], 1);
        vtemp = mat->emissiveColor;
        vtemp = {vtemp.w, vtemp.z, vtemp.y, vtemp.x};
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "material_emissive"), (u32*)&vtemp[0], 1);
        vtemp = {0.0f, 0.0f, 0.0f, mat->specularExpo};
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "material_shininess"), (u32*)&vtemp[0], 1);

        vtemp = g_state->lightModelAmbient;
        vtemp = {vtemp.w, vtemp.z, vtemp.y, vtemp.x};
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(vertex_lighting_shader.vertexShader, "light_model_ambient"), (u32*)&vtemp[0], 1);

    } else {
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(shader.vertexShader, "projection"), (u32*)mu_proj, 4);
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(shader.vertexShader, "modelview"), (u32*)mu_model, 4);
    }


    GPU_SetViewport((u32 *)osConvertVirtToPhys(gpuDOut),
                    (u32 *)osConvertVirtToPhys(gpuOut),
                    0, 0, width, height);
    {
        GLint x = g_state->scissorBox.x;
        GLint y = g_state->scissorBox.y;
        GLint w = g_state->scissorBox.z;
        GLint h = g_state->scissorBox.w;
        GPU_SetScissorTest((g_state->enableScissorTest ? ext_state.scissorMode : GPU_SCISSOR_DISABLE), x, y, x + w, y + h);
    }

    GPU_DepthMap(-1.0f, 0.0f);
    GPU_SetFaceCulling(GPU_CULL_NONE);
    u8 stencil_ref = g_state->stencilRef;
    GPU_SetStencilTest(g_state->enableStencilTest, gl_writefunc(g_state->stencilFunc), stencil_ref, g_state->stencilFuncMask, g_state->stencilMask);
    GPU_SetStencilOp(gl_stencilop(g_state->stencilOpSFail), gl_stencilop(g_state->stencilOpZFail), gl_stencilop(g_state->stencilOpZPass));
    GPU_WRITEMASK write_mask = (GPU_WRITEMASK)((g_state->colorMaskRed << 0) | (g_state->colorMaskGreen << 1) | (g_state->colorMaskBlue << 2) | (g_state->colorMaskAlpha << 3) | (g_state->depthMask << 4));
    GPU_SetDepthTestAndWriteMask(g_state->enableDepthTest, gl_depthfunc(g_state->depthFunc), write_mask);
    GPUCMD_AddMaskedWrite(GPUREG_EARLYDEPTH_TEST1, 0x1, 0);
    GPUCMD_AddWrite(GPUREG_EARLYDEPTH_TEST2, 0);

    if (g_state->enableBlend) {
        GPU_SetAlphaBlending(
                             GPU_BLEND_ADD,
                             GPU_BLEND_ADD,
                             gl_blendfactor(g_state->blendSrcFactor), gl_blendfactor(g_state->blendDstFactor),
                             gl_blendfactor(g_state->blendSrcFactor), gl_blendfactor(g_state->blendDstFactor)
                             );
        u32 blendColor = g_state->blendColor;
        GPU_SetBlendingColor(blendColor & 0xFF, (blendColor >> 8) & 0xFF, (blendColor >> 16) & 0xFF, (blendColor >> 24) & 0xFF);
    }
    else
    {
        GPU_SetAlphaBlending(
                             GPU_BLEND_ADD,
                             GPU_BLEND_ADD,
                             GPU_ONE, GPU_ZERO,
                             GPU_ONE, GPU_ZERO
                             );
        GPU_SetBlendingColor(0, 0, 0, 0);
    }

    u8 alpha_ref = (u8)(g_state->alphaTestRef * 255.0f);
    GPU_SetAlphaTest(g_state->enableAlphaTest, gl_writefunc(g_state->alphaTestFunc), alpha_ref);

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
    } else {
        GPU_SetTextureEnable(GPU_TEXUNIT0);

        extern gfx_texture *getTexture(GLuint name);
        gfx_texture* text = getTexture(g_state->currentBoundTexture);
        if (text) {

            if (text->format == GL_ALPHA) {
                GPU_SetTexEnv(0,
                              GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
                              GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_TEXTURE0),
                              GPU_TEVOPERANDS(0,0,0),
                              GPU_TEVOPERANDS(0,0,0),
                              GPU_REPLACE, GPU_MODULATE,
                              0xFFFFFFFF);
            } else {
                GPU_SetTexEnv(0,
                              GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_TEXTURE0),
                              GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_TEXTURE0),
                              GPU_TEVOPERANDS(0,0,0),
                              GPU_TEVOPERANDS(0,0,0),
                              GPU_MODULATE, GPU_MODULATE,
                              0xFFFFFFFF);
            }

            GPU_SetTexture(
                           GPU_TEXUNIT0,
                           (u32*)osConvertVirtToPhys(text->colorBuffer),
                           text->width,
                           text->height,
                           GPU_TEXTURE_MIN_FILTER(text->min_filter) |
                           GPU_TEXTURE_MAG_FILTER(text->mag_filter) |
                           GPU_TEXTURE_WRAP_S(text->wrap_s) |
                           GPU_TEXTURE_WRAP_T(text->wrap_t),
                           GPU_RGBA8
                           );
        }
    }

    GPU_SetDummyTexEnv(1);
    GPU_SetDummyTexEnv(2);
    GPU_SetDummyTexEnv(3);
    GPU_SetDummyTexEnv(4);
    GPU_SetDummyTexEnv(5);
}

static
void safeWaitForEvent(Handle event) {
    Result res = svcWaitSynchronization(event, 1000*1000*100);
    if(!res)svcClearEvent(event);
}

void gfx_device_3ds::render_vertices_vbo(const mat4& projection, const mat4& modelview, u8 *data, GLuint units) {
    GPUCMD_SetBufferOffset(0);
    GPUCMD_AddMaskedWrite(GPUREG_ATTRIBBUFFERS_FORMAT_HIGH, 0b111111111111 << 16, 0);
    setup_state(projection, modelview);
    SetAttributeBuffers(
                        4,
                        (u32*)osConvertVirtToPhys(data),
                        GPU_ATTRIBFMT(0, 3, GPU_FLOAT) | GPU_ATTRIBFMT(1, 4, GPU_FLOAT) |
                        GPU_ATTRIBFMT(2, 4, GPU_FLOAT) | GPU_ATTRIBFMT(3, 4, GPU_FLOAT),
                        0xFF8,
                        0x3210,
                        1,
                        {0x0},
                        {0x3210},
                        {4}
                        );
    
    GPU_DrawArray(gl_primitive(g_state->vertexDrawMode), 0, units);
    GPU_FinishDrawing();
    GPUCMD_Finalize();
    GPUCMD_FlushAndRun();
    safeWaitForEvent(gspEvents[GSPGPU_EVENT_P3D]);
}

void gfx_device_3ds::render_vertices(const mat4& projection, const mat4& modelview) {
    GPUCMD_AddWrite(GPUREG_ATTRIBBUFFERS_FORMAT_LOW, GPU_ATTRIBFMT(0, 3, GPU_FLOAT)
                    | GPU_ATTRIBFMT(1, 4, GPU_FLOAT) | GPU_ATTRIBFMT(2, 4, GPU_FLOAT)
                    | GPU_ATTRIBFMT(3, 4, GPU_FLOAT));
    GPUCMD_AddWrite(GPUREG_ATTRIBBUFFERS_FORMAT_HIGH, ((4 - 1) << 28) | (0xFFF << 16));
    setup_state(projection, modelview);

    GPUCMD_AddMaskedWrite(GPUREG_PRIMITIVE_CONFIG, 2, gl_primitive(g_state->vertexDrawMode) << 8);
    GPUCMD_AddWrite(GPUREG_RESTART_PRIMITIVE, 1);
    GPUCMD_AddWrite(GPUREG_INDEXBUFFER_CONFIG, 0x80000000);
    GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG2, 3, 0x001);
    GPUCMD_AddMaskedWrite(GPUREG_START_DRAW_FUNC0, 1, 0);
    GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_INDEX, 0xF);

    for (u32 i = 0; i < g_state->vertexBuffer.size(); ++i)
    {
        vertex &v = g_state->vertexBuffer[i];
        vec4 &cc = v.position;
        u32 cr = f32tof24(cc.x);
        u32 cg = f32tof24(cc.y);
        u32 cb = f32tof24(cc.z);
        u32 ca = f32tof24(cc.w);
        GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA0, ((cb >> 16) & 0xFF) | (ca << 8));
        GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA1, ((cg >> 8) & 0xFFFF) | (((cb) & 0xFFFF) << 16));
        GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA2, cr | (((cg) & 0xFF) << 24));

        cc = v.textureCoord;
        cr = f32tof24(cc.x);
        cg = f32tof24(cc.y);
        cb = f32tof24(cc.z);
        ca = f32tof24(cc.w);
        GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA0, ((cb >> 16) & 0xFF) | (ca << 8));
        GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA1, ((cg >> 8) & 0xFFFF) | (((cb) & 0xFFFF) << 16));
        GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA2, cr | (((cg) & 0xFF) << 24));

        cc = v.color;
        cr = f32tof24(cc.x);
        cg = f32tof24(cc.y);
        cb = f32tof24(cc.z);
        ca = f32tof24(cc.w);
        GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA0, ((cb >> 16) & 0xFF) | (ca << 8));
        GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA1, ((cg >> 8) & 0xFFFF) | (((cb) & 0xFFFF) << 16));
        GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA2, cr | (((cg) & 0xFF) << 24));

        cc = v.normal;
        cr = f32tof24(cc.x);
        cg = f32tof24(cc.y);
        cb = f32tof24(cc.z);
        ca = f32tof24(cc.w);
        GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA0, ((cb >> 16) & 0xFF) | (ca << 8));
        GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA1, ((cg >> 8) & 0xFFFF) | (((cb) & 0xFFFF) << 16));
        GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA2, cr | (((cg) & 0xFF) << 24));
    }

    GPUCMD_AddMaskedWrite(GPUREG_START_DRAW_FUNC0, 1, 1);
    GPUCMD_AddMaskedWrite(GPUREG_GEOSTAGE_CONFIG2, 1, 0);
    GPUCMD_AddWrite(GPUREG_VTX_FUNC, 1);
}

void gfx_device_3ds::render_vertices_array(GLenum mode, GLint first, GLsizei count, const mat4& projection, const mat4& modelview) {
  GPUCMD_SetBufferOffset(0);
  setup_state(projection, modelview);
  // pos, tex, color, normal

  SetAttributeBuffers(
                      4,
                      (u32*)osConvertVirtToPhys(g_state->vertexPtr),
                      GPU_ATTRIBFMT(0, 3, GPU_FLOAT) | GPU_ATTRIBFMT(1, 4, GPU_FLOAT) |
                      GPU_ATTRIBFMT(2, 4, GPU_FLOAT) | GPU_ATTRIBFMT(3, 4, GPU_FLOAT),
                      0xFFE,
                      0x3210,
                      1,
                      {0x0},
                      {0x0},
                      {1}
                      );

  // make tex, color, and normal immediate values
  // TODO texcoordpointer
  GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_INDEX, 1);
  GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA0, 0);
  GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA1, 0);
  GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA2, 0);
  // TODO colorpointer
  GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_INDEX, 2);
  {
    vec4 cc = g_state->currentVertexColor;
    u32 cr = f32tof24(cc.x);
    u32 cg = f32tof24(cc.y);
    u32 cb = f32tof24(cc.z);
    u32 ca = f32tof24(cc.w);
    GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA0, ((cb >> 16) & 0xFF) | (ca << 8));
    GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA1, ((cg >> 8) & 0xFFFF) | (((cb) & 0xFFFF) << 16));
    GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA2, cr | (((cg) & 0xFF) << 24));
  }
  // TODO normalpointer
  GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_INDEX, 3);
  {
    vec4 cc = g_state->currentVertexNormal;
    u32 cr = f32tof24(cc.x);
    u32 cg = f32tof24(cc.y);
    u32 cb = f32tof24(cc.z);
    u32 ca = f32tof24(cc.w);
    GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA0, ((cb >> 16) & 0xFF) | (ca << 8));
    GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA1, ((cg >> 8) & 0xFFFF) | (((cb) & 0xFFFF) << 16));
    GPUCMD_AddWrite(GPUREG_FIXEDATTRIB_DATA2, cr | (((cg) & 0xFF) << 24));
  }

  GPU_DrawArray(gl_primitive(mode), first, count);
  GPU_FinishDrawing();
  GPUCMD_Finalize();
  GPUCMD_FlushAndRun();
  safeWaitForEvent(gspEvents[GSPGPU_EVENT_P3D]);
}

void gfx_device_3ds::clearDepth(GLfloat d) {
  shaderProgramUse(&clear_shader);

  float mu_proj[4*4];
  mat4 pica = mat4();
  pica[0xA] = 0.5;
  pica[0xB] = -0.5;

  int i, j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      mu_proj[i*4 + j] = pica[i*4 + (3-j)];
    }
  }

  float clear_color[4] = {1, 1, 1, 1};
  float clear_depth[4] = {1, 1, (float)d, 1};

  {
    GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(clear_shader.vertexShader, "projection"), (u32*)mu_proj, 4);
    GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(clear_shader.vertexShader, "clear_color"), (u32*)clear_color, 1);
    GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(clear_shader.vertexShader, "clear_depth"), (u32*)clear_depth, 1);
  }


  GPU_SetViewport((u32 *)osConvertVirtToPhys(gpuDOut),
                  (u32 *)osConvertVirtToPhys(gpuOut),
                  0, 0, width, height);
  {
    GLint x = g_state->scissorBox.x;
    GLint y = g_state->scissorBox.y;
    GLint w = g_state->scissorBox.z;
    GLint h = g_state->scissorBox.w;
    GPU_SetScissorTest((g_state->enableScissorTest ? ext_state.scissorMode : GPU_SCISSOR_DISABLE), x, y, x + w, y + h);
  }

  GPU_DepthMap(-1.0f, 0.0f);
  GPU_SetFaceCulling(GPU_CULL_NONE);
  u8 stencil_ref = g_state->stencilRef;
  GPU_SetStencilTest(false, GPU_NEVER, stencil_ref, g_state->stencilFuncMask, g_state->stencilMask);
  GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, GPU_STENCIL_KEEP);
  GPU_SetDepthTestAndWriteMask(true, GPU_ALWAYS, GPU_WRITE_DEPTH);
  GPUCMD_AddMaskedWrite(GPUREG_EARLYDEPTH_TEST1, 0x1, 0);
  GPUCMD_AddWrite(GPUREG_EARLYDEPTH_TEST2, 0);

  u8 alpha_ref = (u8)(g_state->alphaTestRef * 255.0f);
  GPU_SetAlphaTest(false, GPU_NEVER, alpha_ref);
  GPU_SetTexEnv(
                  0,
                  GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
                  GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
                  GPU_TEVOPERANDS(0, 0, 0),
                  GPU_TEVOPERANDS(0, 0, 0),
                  GPU_REPLACE, GPU_REPLACE,
                  0xFFFFFFFF
                  );

  GPU_SetDummyTexEnv(1);
  GPU_SetDummyTexEnv(2);
  GPU_SetDummyTexEnv(3);
  GPU_SetDummyTexEnv(4);
  GPU_SetDummyTexEnv(5);

  SetAttributeBuffers(
                      4,
                      (u32*)osConvertVirtToPhys(clearQuadVBO->data),
                      GPU_ATTRIBFMT(0, 3, GPU_FLOAT) | GPU_ATTRIBFMT(1, 4, GPU_FLOAT) |
                      GPU_ATTRIBFMT(2, 4, GPU_FLOAT) | GPU_ATTRIBFMT(3, 4, GPU_FLOAT),
                      0xFF8,
                      0x3210,
                      1,
                      {0x0},
                      {0x3210},
                      {4}
                      );
  GPU_DrawArray(gl_primitive(GL_TRIANGLES), 0, clearQuadVBO->numVertices);
}

#define DISPLAY_TRANSFER_FLAGS \
  (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
  GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | \
  GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

void gfx_device_3ds::flush(u8 *fb, int w, int h, int format) {
    
    GX_DisplayTransfer((u32*)gpuOut, GX_BUFFER_DIM(width, height), (u32 *)fb, GX_BUFFER_DIM(w, h), DISPLAY_TRANSFER_FLAGS | GX_TRANSFER_OUT_FORMAT(format));
    safeWaitForEvent(gspEvents[GSPGPU_EVENT_PPF]);
}

#define RGBA8(r,g,b,a) ( (((r)&0xFF)<<24) | (((g)&0xFF)<<16) | (((b)&0xFF)<<8) | (((a)&0xFF)<<0) )
void gfx_device_3ds::clear(float r, float g, float b, float a) {

  shaderProgramUse(&clear_shader);

  float mu_proj[4*4];
  mat4 pica = mat4();
  pica[0xA] = 0.5;
  pica[0xB] = -0.5;

  int i, j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      mu_proj[i*4 + j] = pica[i*4 + (3-j)];
    }
  }

  float clear_color[4] = { a, b, g, r };
  float clear_depth[4] = {1, 1, 0, 1};

  {
    GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(clear_shader.vertexShader, "projection"), (u32*)mu_proj, 4);
    GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(clear_shader.vertexShader, "clear_color"), (u32*)clear_color, 1);
    GPU_SetFloatUniform(GPU_VERTEX_SHADER, shaderInstanceGetUniformLocation(clear_shader.vertexShader, "clear_depth"), (u32*)clear_depth, 1);
  }


  GPU_SetViewport((u32 *)osConvertVirtToPhys(gpuDOut),
                  (u32 *)osConvertVirtToPhys(gpuOut),
                  0, 0, width, height);
  {
    GLint x = g_state->scissorBox.x;
    GLint y = g_state->scissorBox.y;
    GLint w = g_state->scissorBox.z;
    GLint h = g_state->scissorBox.w;
    GPU_SetScissorTest((g_state->enableScissorTest ? ext_state.scissorMode : GPU_SCISSOR_DISABLE), x, y, x + w, y + h);
  }

  GPU_DepthMap(-1.0f, 0.0f);
  GPU_SetFaceCulling(GPU_CULL_NONE);
  u8 stencil_ref = g_state->stencilRef;
  GPU_SetStencilTest(false, GPU_NEVER, stencil_ref, g_state->stencilFuncMask, g_state->stencilMask);
  GPU_SetStencilOp(GPU_STENCIL_KEEP, GPU_STENCIL_KEEP, GPU_STENCIL_KEEP);
  GPU_WRITEMASK write_mask = (GPU_WRITEMASK)((g_state->colorMaskRed << 0) | (g_state->colorMaskGreen << 1) | (g_state->colorMaskBlue << 2) | (g_state->colorMaskAlpha << 3));
  GPU_SetDepthTestAndWriteMask(false, GPU_ALWAYS, write_mask);
  GPUCMD_AddMaskedWrite(GPUREG_EARLYDEPTH_TEST1, 0x1, 0);
  GPUCMD_AddWrite(GPUREG_EARLYDEPTH_TEST2, 0);

  u8 alpha_ref = (u8)(g_state->alphaTestRef * 255.0f);
  GPU_SetAlphaTest(false, GPU_NEVER, alpha_ref);
  GPU_SetTexEnv(
                0,
                GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
                GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
                GPU_TEVOPERANDS(0, 0, 0),
                GPU_TEVOPERANDS(0, 0, 0),
                GPU_REPLACE, GPU_REPLACE,
                0xFFFFFFFF
                );

  GPU_SetDummyTexEnv(1);
  GPU_SetDummyTexEnv(2);
  GPU_SetDummyTexEnv(3);
  GPU_SetDummyTexEnv(4);
  GPU_SetDummyTexEnv(5);

  SetAttributeBuffers(
                      4,
                      (u32*)osConvertVirtToPhys(clearQuadVBO->data),
                      GPU_ATTRIBFMT(0, 3, GPU_FLOAT) | GPU_ATTRIBFMT(1, 4, GPU_FLOAT) |
                      GPU_ATTRIBFMT(2, 4, GPU_FLOAT) | GPU_ATTRIBFMT(3, 4, GPU_FLOAT),
                      0xFF8,
                      0x3210,
                      1,
                      {0x0},
                      {0x3210},
                      {4}
                      );
  GPU_DrawArray(gl_primitive(GL_TRIANGLES), 0, clearQuadVBO->numVertices);
}

void gfx_device_3ds::flush_commands() {
    GPU_FinishDrawing();
    GPUCMD_Finalize();
    GPUCMD_FlushAndRun();
    GPUCMD_SetBufferOffset(0);
}

void gfx_device_3ds::flush_wait_commands() {
    GPU_FinishDrawing();
    GPUCMD_Finalize();
    GPUCMD_FlushAndRun();
    safeWaitForEvent(gspEvents[GSPGPU_EVENT_P3D]);
    GPUCMD_SetBufferOffset(0);
}
