#ifndef __ctr_h_
#define __ctr_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif

#define GL_SCISSOR_INVERT_DMP                   0x0C13
#define GL_SCISSOR_NORMAL_DMP                   0x0C12

GLAPI void APIENTRY glScissorMode( GLenum mode );

#ifndef GL_DMP_scissor_mode
#define GL_DMP_scissor_mode
#endif


#ifdef __cplusplus
}
#endif

#endif
