#include "glImpl.h"

gfx_state* g_state = NULL;

extern "C" {
void* gfxCreateDevice(int width, int height, int flags) {
  gfx_device_3ds *dev = new gfx_device_3ds(new gfx_state(), width, height);
  dev->g_state->flags = flags;
  dev->g_state->device = dev;
  return dev;
}

void gfxDestroyDevice(void* device) {
  if(device) delete (gfx_device*)device;
}


void *gfxMakeCurrent(void* device) {
  void *previous = g_state;
  if (!device) {
    g_state = NULL;
    return previous;
  }
  g_state = ((gfx_device*)device)->g_state;
  return previous;
}

void gfxResize(int new_width, int new_height) {
  CHECK_NULL(g_state);

  gfx_device_3ds *state = (gfx_device_3ds*) g_state->device;
  vramFree(state->gpuOut);
  vramFree(state->gpuDOut);
  state->gpuOut = (u32*)vramAlloc(new_width * new_height * 4);
  state->gpuDOut = (u32*)vramAlloc(new_width * new_height * 4);
  state->width = new_width;
  state->height = new_height;
}

void gfxFlush(unsigned char* fb, int out_width, int out_height, int format) {
  CHECK_NULL(g_state);
  CHECK_NULL(fb);

  g_state->device->flush(fb, out_width, out_height, format);
}

} // extern "C"
