#ifndef GLIMPL_H
#define GLIMPL_H

#define IMPL_MAX_MODELVIEW_STACK_DEPTH    32
#define IMPL_MAX_PROJECTION_STACK_DEPTH    2
#define IMPL_MAX_TEXTURE_STACK_DEPTH       2
#define IMPL_MAX_TEXTURE_SIZE           1024
#define IMPL_MAX_LIST_CALL_DEPTH          64
#define IMPL_MAX_LIGHTS                    8

#include "gfx_device_internal.h"

#define CHECK_NULL(x, ...) \
    if(!x) return __VA_ARGS__;

#define CHECK_WITHIN_BEGIN_END(x, ...) \
    if(g_state->withinBeginEndBlock) { \
        setError(GL_INVALID_OPERATION); \
        return __VA_ARGS__; \
    }

#define CHECK_WITHIN_NEW_END(x, ...) \
    if(g_state->withinNewEndListBlock) { \
        setError(GL_INVALID_OPERATION); \
        return __VA_ARGS__; \
    }

#define CHECK_COMPILE_AND_EXECUTE(x, ...) \
    if(g_state->newDisplayListMode == GL_COMPILE) { \
        return __VA_ARGS__; \
    }

struct gfx_state;
extern gfx_state *g_state;

#ifdef __cplusplus
extern "C" {
#endif

    void setError(GLenum error);

#ifdef __cplusplus
}
#endif


#endif
