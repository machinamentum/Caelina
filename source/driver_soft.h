#ifndef DRIVER_SOFT_H
#define DRIVER_SOFT_H

#include "gfx_device_internal.h"

#ifdef BUILD_DEVICE_SOFT

class gfx_device_soft : public gfx_device {
    u8* backbuffer;
    GLdouble* depthBuffer;
    mat4 screenSpaceMatrix;

    struct grad {
        vec4 color[3];
        vec4 color_xStep;
        vec4 color_yStep;
        vec4 texture[3];
        vec4 texture_xStep;
        vec4 texture_yStep;
        float odz[3];
        float odz_xStep;
        float odz_yStep;

        grad(const vertex& minY, const vertex& midY, const vertex& maxY);
    };

    struct edge {
        float x;
        float xStep;
        s32 yStart;
        s32 yEnd;
        vec4 color;
        vec4 colorStep;
        vec4 texCoord;
        vec4 texCoordStep;
        float odz;
        float odzStep;

        edge(const grad& grads, const vertex& start, const vertex& end, int startIndex);
        void step();
    };

    void drawPixel(int x, int y, u8 r, u8 g, u8 b, u8 a);
    vec4 getPixel(int x, int y);
    void fillTriangle(const mat4& mvp, const vertex& v1, const vertex& v2, const vertex& v3);
    void scanTriangle(vertex& minY, vertex& midY, vertex& maxY, bool handedness);

public:
    gfx_device_soft(int w, int h);
    virtual ~gfx_device_soft();
    virtual void clear(u8 r, u8 g, u8 b, u8 a);
    virtual void clearDepth(GLdouble depth);
    virtual void flush(u8* fb);
    virtual void render_vertices(const mat4& mvp);
    
};

#endif

#endif
