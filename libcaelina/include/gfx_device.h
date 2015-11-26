
#ifndef GFX_DEVICE
#define GFX_DEVICE


#ifdef __cplusplus
extern "C" {
#endif

#define CAELINA_SHARED_TEXTURES       (1 << 0)
#define CAELINA_SHARED_DISPLAY_LISTS  (1 << 1)

void *gfxCreateDevice(int width, int height, int flags);
void  gfxDestroyDevice(void* device);
void *gfxMakeCurrent(void* device);
void  gfxResize(int new_width, int new_height);
void  gfxFlush(unsigned char* fb, int out_width, int out_height, int format);

#ifdef __cplusplus
}
#endif


#endif
