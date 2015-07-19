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
		buffer = (T *)linearAlloc(reserve * sizeof(T));
		buffer_size = reserve;
		current_index = 0;
		reserve_block_size = reserve;
	}

	~sbuffer() {
		if(buffer) linearFree(buffer);
	}

	void push(const T& data) {
		if(current_index + 1 >= buffer_size) {
			resize(buffer_size + reserve_block_size);
		}

		buffer[current_index] = data;
		// memcpy(&buffer[current_index], &data, sizeof(T));
		current_index++;
	}

	void resize(unsigned int n_size) {
		T* n_buf = (T *)linearAlloc(n_size * sizeof(T));
		for(unsigned int i = 0; i < buffer_size; i++) {
			n_buf[i] = buffer[i];
		}

		linearFree(buffer);
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
	GLenum format;
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

struct gfx_command {
	enum CMD_TYPE {
		PUSH_MATRIX = 0,
		POP_MATRIX = 1,
		MATRIX_MODE = 2,
		CLEAR_COLOR = 3,
		CLEAR = 4,
		LOAD_IDENTITY = 5,
		BEGIN = 6,
		END = 7,
		BIND_TEXTURE = 8,
		TEX_IMAGE_2D,
		ROTATE,
		SCALE,
		TRANSLATE,
		ORTHO,
		FRUSTUM,
		VIEWPORT,
		BLEND_FUNC,
		ENABLE,
		DISABLE,
		TEX_PARAM_I,
		SCISSOR,
		CALL_LIST,
		NONE
	};

	CMD_TYPE type = NONE;
	GLuint vdata_size = 0;
	GLuint vdata_units = 0;
	u8 *vdata = NULL;

	GLenum enum1 = 0;
	GLenum enum2 = 0;
	GLenum enum3 = 0;
	union {
		GLclampf clamp1;
		GLfloat float1;
	};
	union {
		GLclampf clamp2;
		GLfloat float2;
	};
	union {
		GLclampf clamp3;
		GLfloat float3;
	};
	union {
		GLclampf clamp4;
		GLfloat float4;
	};
	GLfloat float5;
	GLfloat float6;
	GLbitfield mask1;
	GLuint uint1;
	GLint int1;
	GLint int2;
	GLint int3;
	GLsizei size1;
	GLsizei size2;
	GLvoid *voidp = NULL;

};

#include <vector>

struct gfx_display_list {
	GLuint name;
	GLboolean useColor = GL_FALSE;
	GLboolean useTex = GL_FALSE;
	GLboolean useNormal = GL_FALSE;
	vec4 vColor;
	vec4 vTex;
	vec4 vNormal;
	std::vector<gfx_command> commands;
};

struct gfx_light {
	vec4 ambient = { 0.0, 0.0, 0.0, 1.0 };
	vec4 diffuse = { 0.0, 0.0, 0.0, 1.0 }; // set in gfx_device()
	vec4 specular = { 0.0, 0.0, 0.0, 1.0 }; // set int gfx_device();
	vec4 position = { 0.0, 0.0, 1.0, 0.0 };
	vec4 spotlightDirection = { 0.0, 0.0, -1.0, 0.0 };
	float spotlightExpo = 0.0; //[0.0, 128.0]
	float spotlightCutoff = 180.0; // [0.0, 90.0], 180.0
	float constantAttenuation = 1.0; // [0.0, inf]
	float linearAttentuation = 0.0; // [0.0, inf]
	float quadraticAttenuation = 0.0; // [0.0, inf]
};

struct gfx_material {
	vec4 ambientColor = { 0.2, 0.2, 0.2, 1.0 };
	vec4 diffuseColor = { 0.8, 0.8, 0.8, 1.0 };
	vec4 specularColor = { 0.0, 0.0, 0.0, 1.0 };
	vec4 emissiveColor = { 0.0, 0.0, 0.0, 1.0 };
	float specularExpo = 0.0; // [0.0, 128.0]
	float ambientColorIndex = 0.0;
	float diffuseColorIndex = 1.0;
	float specularColorIndex = 1.0;
};

struct gfx_state {
	gfx_device* device;

	vec4 clearColor;

	mat4 modelviewMatrixStack[IMPL_MAX_MODELVIEW_STACK_DEPTH];
	mat4 projectionMatrixStack[IMPL_MAX_PROJECTION_STACK_DEPTH];
	mat4 textureMatrixStack[IMPL_MAX_TEXTURE_STACK_DEPTH];
	mat4 viewportMatrix;

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
	GLboolean enableLighting = GL_FALSE;
	GLboolean enableLight[IMPL_MAX_LIGHTS];

	gfx_vec4i scissorBox;

	gfx_light lights[IMPL_MAX_LIGHTS];
	vec4 lightModelAmbient = { 0.2, 0.2, 0.2, 1.0};
	GLboolean lightModelLocalEye = GL_FALSE;
	GLboolean lightModelTwoSided = GL_FALSE; //TODO
	gfx_material material;

	sbuffer<gfx_display_list> displayLists;
	GLuint nextDisplayListName = 1;
	GLboolean withinNewEndListBlock = GL_FALSE;
	GLenum newDisplayListMode = GL_COMPILE_AND_EXECUTE;
	GLuint currentDisplayList = 0; //for compiling only
	GLuint displayListCallDepth = 0;
	gfx_command endVBOCMD;

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
		g_state->lights[0].diffuse = { 1.0, 1.0, 1.0, 1.0 };
		g_state->lights[0].specular = { 1.0, 1.0, 1.0, 1.0 };

		for (int i = 0; i < IMPL_MAX_LIGHTS; ++i) {
			g_state->enableLight[i] = GL_FALSE;
		}
	}

	virtual ~gfx_device() {};
	virtual void clear(u8 r, u8 g, u8 b, u8 a) = 0;
	virtual void clearDepth(GLdouble depth) = 0;
	virtual void flush(u8 *fb) = 0;
	virtual void render_vertices(const mat4& projection, const mat4& modelview) = 0;
	virtual void render_vertices_vbo(const mat4& projection, const mat4& modelview, u8 *data, GLuint units) = 0;
	virtual void repack_texture(gfx_texture &tex) {}
	virtual u8 *cache_vertex_list(GLuint *size) = 0;

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
