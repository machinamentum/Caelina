#ifndef GFX_DEVICE_INTERNAL_H
#define GFX_DEVICE_INTERNAL_H

#include <GL/gl.h>

#include "vector.h"
#include "matrix.h"
#include "glImpl.h"

#ifndef _3DS
typedef int s32;
typedef unsigned char u8;
typedef char s8;
#else
#include <3ds.h>
#endif
struct gfx_state;
struct gfx_texture;
struct gfx_device;

template <class T>
class sbuffer {
	T* buffer;
	unsigned int buffer_size;
	unsigned int current_index;
	unsigned int reserve_block_size;
public:

	sbuffer(unsigned int reserve = 128) {
		if(reserve == 0) reserve = 128;
		buffer = new T[reserve];;
		buffer_size = reserve;
		current_index = 0;
		reserve_block_size = reserve;
	}

	~sbuffer() {
		if(buffer) delete[] buffer;
	}

	void push(const T& data) {
		if(current_index + 1 >= buffer_size) {
			resize(buffer_size + reserve_block_size);
		}

		buffer[current_index] = data;
		current_index++;
	}

	void resize(unsigned int n_size) {
		T* n_buf = new T[n_size];
		for(unsigned int i = 0; i < buffer_size; i++) {
			n_buf[i] = buffer[i];
		}

		delete[] buffer;
		buffer = n_buf;
		buffer_size = n_size;
	}

	void clear() {
		current_index = 0;
	}

	unsigned int size() const {
		return current_index;
	}

	T& operator[](unsigned int index) const {
		return buffer[index];
	}

	bool contains(const T& data) const {
		for(unsigned int i = 0; i < size(); ++i) {
			if(data == buffer[i]) return true;
		}

		return false;
	}
};

struct vertex {
	vec4 position;
	vec4 color;
	vec4 textureCoord;
	vec4 normal;

	vertex(const vec4& pos = vec4(0, 0, 0, 1),
			const vec4& col = vec4(1, 1, 1, 1),
			const vec4& tex = vec4(0, 0, 0, 1),
			const vec4& norm = vec4(0, 0, 1, 0)) {
		position = vec4(pos);
		color = vec4(col);
		textureCoord = vec4(tex);
		normal = vec4(norm);
	}

	vertex(const vertex& v) {
		position = vec4(v.position);
		color = vec4(v.color);
		textureCoord = vec4(v.textureCoord);
		normal = vec4(v.normal);
	}
};


struct gfx_texture {
	GLuint tname;
	GLenum target;
	GLubyte* colorBuffer;
	GLsizei width;
	GLsizei height;
	GLenum min_filter = GL_NEAREST_MIPMAP_LINEAR;//TODO
	GLenum mag_filter = GL_LINEAR;
	GLenum wrap_s = GL_REPEAT;
	GLenum wrap_t = GL_REPEAT;
	//TODO(josh)

	gfx_texture(GLuint name = 0, GLenum tar = 0) {
		tname = name;
		target = tar;
		colorBuffer = NULL;
	}

	~gfx_texture() {
		//if(colorBuffer) delete colorBuffer;
	}


	bool operator==(const gfx_texture& t) const {
		return t.tname == tname;
	}
};

struct gfx_vec4i {
	GLint x;
	GLint y;
	GLint z;
	GLint w;
};

struct gfx_state {
	//TODO(josh) make gfx_device an interface to allow hardware devices
	gfx_device* device;

	vec4 clearColor;

	mat4 modelviewMatrixStack[IMPL_MAX_MODELVIEW_STACK_DEPTH];
	mat4 projectionMatrixStack[IMPL_MAX_PROJECTION_STACK_DEPTH];
	mat4 textureMatrixStack[IMPL_MAX_TEXTURE_STACK_DEPTH];

	s8 currentModelviewMatrix = 0;
	s8 currentProjectionMatrix = 0;
	s8 currentTextureMatrix = 0;

	GLenum matrixMode = GL_MODELVIEW;

	GLenum errorFlag;

	GLboolean withinBeginEndBlock = GL_FALSE;

	sbuffer<vertex> vertexBuffer;
	vec4 currentVertexColor = vec4(1, 1, 1, 1);
	vec4 currentTextureCoord = vec4(0, 0, 0, 1);
	vec4 currentVertexNormal = vec4(0, 0, 1, 0);
	GLenum vertexDrawMode;

	sbuffer<gfx_texture> textures;
	GLuint currentBoundTexture = 0;
	GLint packAlignment = 4;
	GLint unpackAlignment = 4;

	GLenum blendSrcFactor = GL_ONE;
	GLenum blendDstFactor = GL_ZERO;

	GLboolean enableTexture2D = GL_FALSE;
	GLboolean enableDepthTest = GL_FALSE;
	GLboolean enableBlend = GL_FALSE;
	GLboolean enableScissorTest = GL_FALSE;

	gfx_vec4i scissorBox;

	mat4 viewportMatrix;

};

class gfx_device {
protected:
	int width, height;

public:
	gfx_state* g_state = NULL;

	gfx_device(gfx_state *state, int w, int h) {
		g_state = state;
		g_state->scissorBox = {0, 0, w, h};
		width = w;
		height = h;
	}

	virtual ~gfx_device() {};
	virtual void clear(u8 r, u8 g, u8 b, u8 a) = 0;
	virtual void clearDepth(GLdouble depth) = 0;
	virtual void flush(u8 *fb) = 0;
	virtual void render_vertices(const mat4& mvp) = 0;
	virtual void repack_texture(gfx_texture &tex) {}

	int getWidth() {
		return width;
	}

	int getHeight() {
		return height;
	}
};


inline float min(float a, float b) {
	return a < b ? a : b;
}

inline float clampf(float val, float low, float hi) {
	return val < low ? low : (val > hi ? hi : val);
}

#endif
