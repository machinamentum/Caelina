#include "driver_3ds.h"
#include <3ds.h>

#include <cstring>
#include "default_3ds_vsh_shbin.h"
#include "vertex_lighting_3ds_vsh_shbin.h"

#define DISPLAY_TRANSFER_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
    GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
    GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_X))


#undef GPUCMD_AddSingleParam
static void GPUCMD_AddSingleParam(u32 header, u32 param) {
    GPUCMD_Add(header, &param, 1);
}

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

gfx_device_3ds::gfx_device_3ds(gfx_state *state, int w, int h) : gfx_device(state, w, h) {
    GPU_Init(NULL);
    gpuCmd = (u32*)linearAlloc(gpuCmdSize*4);
    gpuOut=(u32*)vramMemAlign(height*width*8, 0x100);;
    gpuDOut=(u32*)vramMemAlign(height*height*8, 0x100);;
    GPU_Reset(NULL, gpuCmd, gpuCmdSize);

    dvlb=DVLB_ParseFile((u32*)default_3ds_vsh_shbin, default_3ds_vsh_shbin_size);
    shaderProgramInit(&shader);
    shaderProgramSetVsh(&shader, &dvlb->DVLE[0]);
    shaderProgramUse(&shader);

    dvlb=DVLB_ParseFile((u32*)vertex_lighting_3ds_vsh_shbin, vertex_lighting_3ds_vsh_shbin_size);
    shaderProgramInit(&vertex_lighting_shader);
    shaderProgramSetVsh(&vertex_lighting_shader, &dvlb->DVLE[0]);
    shaderProgramUse(&vertex_lighting_shader);

    GPUCMD_Finalize();
    GPUCMD_FlushAndRun(NULL);
    gspWaitForP3D();

}

gfx_device_3ds::~gfx_device_3ds() {

}

void gfx_device_3ds::clearDepth(GLdouble d) {

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
extern Handle gspEvents[GSPEVENT_MAX];

void gfx_device_3ds::repack_texture(gfx_texture &tex) {
    u32 size = 4*tex.width*tex.height;
    size = ((size - (size >> (2*(0+1)))) * 4) / 3;
    u32 *dst = (u32 *)linearMemAlign(size, 0x80);
    tileImage32((u32*)tex.colorBuffer, dst, tex.width, tex.height);
    linearFree(tex.colorBuffer);
    if (vramSpaceFree() < size) {
        tex.colorBuffer = (GLubyte*)dst;
    } else {
        tex.colorBuffer = (GLubyte*)vramMemAlign(size, 0x80);
        GX_RequestDma(NULL, dst, (u32*)tex.colorBuffer, size);
        safeWaitForEvent(gspEvents[GSPEVENT_DMA]);
        linearFree(dst);
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
        case GL_CONSTANT_COLOR: return GPU_CONSTANT_COLOR;
        case GL_ONE_MINUS_CONSTANT_COLOR: return GPU_ONE_MINUS_CONSTANT_COLOR;
        case GL_CONSTANT_ALPHA: return GPU_CONSTANT_ALPHA;
        case GL_ONE_MINUS_CONSTANT_ALPHA: return GPU_ONE_MINUS_CONSTANT_ALPHA;
        case GL_SRC_ALPHA_SATURATE: return GPU_SRC_ALPHA_SATURATE;
    }

    return GPU_ONE;
}

static GPU_Primitive_t gl_primitive(GLenum mode) {
    switch(mode) {
        case GL_QUADS:
        case GL_TRIANGLES: return GPU_TRIANGLES;
        case GL_TRIANGLE_STRIP: return GPU_TRIANGLE_STRIP;
        default: return GPU_UNKPRIM;
    }

    return GPU_UNKPRIM;
}

static GPU_TEXTURE_WRAP_PARAM gl_tex_wrap(GLenum wrap) {
    switch (wrap) {
        case GL_CLAMP_TO_EDGE: return GPU_CLAMP_TO_EDGE;
//        case GL_MIRRORED_REPEAT: return GPU_MIRRORED_REPEAT;
        case GL_REPEAT: return GPU_REPEAT;
    }

    return GPU_REPEAT;
}

static GPU_TEXTURE_FILTER_PARAM gl_tex_filter(GLenum filt) {
    switch (filt) {
        case GL_LINEAR: return GPU_LINEAR;
        case GL_NEAREST: return GPU_NEAREST;
    }

    return GPU_LINEAR;
}

static GPU_SCISSORMODE glext_scissor_mode(GLenum mode) {
    switch (mode) {
        case GL_SCISSOR_NORMAL_DMP: return GPU_SCISSOR_NORMAL;
        case GL_SCISSOR_INVERT_DMP: return GPU_SCISSOR_INVERT;
    }

    return GPU_SCISSOR_DISABLE;
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


    GPU_SetViewport((u32 *)osConvertVirtToPhys((u32)gpuDOut),
                    (u32 *)osConvertVirtToPhys((u32)gpuOut),
                    0, 0, width, height);
    {
        GLint x = g_state->scissorBox.x;
        GLint y = g_state->scissorBox.y;
        GLint w = g_state->scissorBox.z;
        GLint h = g_state->scissorBox.w;
        GPU_SetScissorTest((g_state->enableScissorTest ? glext_scissor_mode(ext_state.scissorMode) : GPU_SCISSOR_DISABLE), x, y, x + w, y + h);
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

        gfx_texture* text = NULL;
        for(unsigned int i = 0; i < g_state->textures.size(); i++) {
            if(g_state->textures[i].tname == g_state->currentBoundTexture) {
                text = &g_state->textures[i];
                break;
            }
        }
        if (text) {

            if (text->format == GL_ALPHA) {
                GPU_SetTexEnv(0,
                              GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
                              GPU_TEVSOURCES(GPU_TEXTURE0, GPU_TEXTURE0, GPU_TEXTURE0),
                              GPU_TEVOPERANDS(0,0,0),
                              GPU_TEVOPERANDS(0,0,0),
                              GPU_REPLACE, GPU_MODULATE,
                              0xFFFFFFFF);
            } else {
                GPU_SetTexEnv(0,
                              GPU_TEVSOURCES(GPU_TEXTURE0, GPU_TEXTURE0, GPU_TEXTURE0),
                              GPU_TEVSOURCES(GPU_TEXTURE0, GPU_TEXTURE0, GPU_TEXTURE0),
                              GPU_TEVOPERANDS(0,0,0),
                              GPU_TEVOPERANDS(2,2,2),
                              GPU_MODULATE, GPU_MODULATE,
                              0xFFFFFFFF);
            }

            GPU_SetTexture(
                           GPU_TEXUNIT0,
                           (u32*)osConvertVirtToPhys((u32)text->colorBuffer),
                           text->width,
                           text->height,
                           GPU_TEXTURE_MAG_FILTER(gl_tex_filter(text->mag_filter)) |
                           GPU_TEXTURE_MIN_FILTER(gl_tex_filter(text->min_filter)) |
                           GPU_TEXTURE_WRAP_S(gl_tex_wrap(text->wrap_s)) |
                           GPU_TEXTURE_WRAP_T(gl_tex_wrap(text->wrap_t)),
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
    Result res = svcWaitSynchronization(event, 30*1000*1000);
    if(!res)svcClearEvent(event);
}

void gfx_device_3ds::render_vertices_vbo(const mat4& projection, const mat4& modelview, u8 *data, GLuint units) {
    GPUCMD_SetBufferOffset(0);
    setup_state(projection, modelview);
    SetAttributeBuffers(
                        4,
                        (u32*)osConvertVirtToPhys((u32)data),
                        GPU_ATTRIBFMT(0, 3, GPU_FLOAT) | GPU_ATTRIBFMT(1, 4, GPU_FLOAT) |
                        GPU_ATTRIBFMT(2, 4, GPU_FLOAT) | GPU_ATTRIBFMT(3, 4, GPU_FLOAT),
                        0xFF8,
                        0x3210,
                        1,
                        {0x0},
                        {0x3210},
                        {4}
                        );
    
    GPU_DrawArray(gl_primitive(g_state->vertexDrawMode), units);
    GPU_FinishDrawing();
    GPUCMD_Finalize();
    GPUCMD_FlushAndRun(NULL);
    safeWaitForEvent(gspEvents[GSPEVENT_P3D]);
}

void gfx_device_3ds::render_vertices(const mat4& projection, const mat4& modelview) {
    GPUCMD_SetBufferOffset(0);
    
    setup_state(projection, modelview);

    temp_vbo.set_data(g_state->vertexBuffer);

    SetAttributeBuffers(
                        4,
                        (u32*)osConvertVirtToPhys((u32)temp_vbo.data),
                        GPU_ATTRIBFMT(0, 3, GPU_FLOAT) | GPU_ATTRIBFMT(1, 4, GPU_FLOAT) |
                        GPU_ATTRIBFMT(2, 4, GPU_FLOAT) | GPU_ATTRIBFMT(3, 4, GPU_FLOAT),
                        0xFF8,
                        0x3210,
                        1,
                        {0x0},
                        {0x3210},
                        {4}
                        );
    GPU_DrawArray(gl_primitive(g_state->vertexDrawMode), temp_vbo.numVertices);
    GPU_FinishDrawing();
    GPUCMD_Finalize();
    GPUCMD_FlushAndRun(NULL);
    safeWaitForEvent(gspEvents[GSPEVENT_P3D]);
    
}

void gfx_device_3ds::flush(u8 *fb) {
    
    GX_SetDisplayTransfer(NULL, (u32*)gpuOut, GX_BUFFER_DIM(width, height),
                          (u32*)fb,
                          GX_BUFFER_DIM(width, height), 0x1000);
}

#define RGBA8(r,g,b,a) ((((r)&0xFF)<<24) | (((g)&0xFF)<<16) | (((b)&0xFF)<<8) | (((a)&0xFF)<<0))
void gfx_device_3ds::clear(u8 r, u8 g, u8 b, u8 a) {
    
    GX_SetMemoryFill(NULL, (u32*)gpuOut, RGBA8(r,g,b,a), (u32*)&gpuOut[0x2EE00], 0x201, (u32*)gpuDOut, 0x00000000, (u32*)&gpuDOut[0x2EE00], 0x201);
    safeWaitForEvent(gspEvents[GSPEVENT_PSC0]);
}
