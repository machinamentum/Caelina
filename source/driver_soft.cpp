
#include "driver_soft.h"

#ifdef BUILD_DEVICE_SOFT

gfx_device_soft::grad::grad(const vertex& minY, const vertex& midY, const vertex& maxY) {
	color[0] = minY.color;
	color[1] = midY.color;
	color[2] = maxY.color;

	odz[0] = 1.0f / minY.position.w;
	odz[1] = 1.0f / midY.position.w;
	odz[2] = 1.0f / maxY.position.w;

	texture[0] = minY.textureCoord * odz[0];
	texture[1] = midY.textureCoord * odz[1];
	texture[2] = maxY.textureCoord * odz[2];



	float inv_dX = 1.0 /
		(((midY.position.x - maxY.position.x) *
		 (minY.position.y - maxY.position.y)) -
		((minY.position.x - maxY.position.x) *
		 (midY.position.y - maxY.position.y)));
	float inv_dY = -inv_dX;

	vec4 dCy = (color[1] - color[2]) * (minY.position.x - maxY.position.x)
		- (color[0] - color[2]) * (midY.position.x - maxY.position.x);

	vec4 dCx = (color[1] - color[2]) * (minY.position.y - maxY.position.y)
		- (color[0] - color[2]) * (midY.position.y - maxY.position.y);

	vec4 dTy = (texture[1] - texture[2]) * (minY.position.x - maxY.position.x)
		- (texture[0] - texture[2]) * (midY.position.x - maxY.position.x);

	vec4 dTx = (texture[1] - texture[2]) * (minY.position.y - maxY.position.y)
		- (texture[0] - texture[2]) * (midY.position.y - maxY.position.y);

	float dZx = (odz[1] - odz[2]) * (minY.position.y - maxY.position.y)
		- (odz[0] - odz[2]) * (midY.position.y - maxY.position.y);

	float dZy = (odz[1] - odz[2]) * (minY.position.x - maxY.position.x)
		- (odz[0] - odz[2]) * (midY.position.x - maxY.position.x);

	color_xStep = dCx * inv_dX;
	color_yStep = dCy * inv_dY;

	texture_xStep = dTx * inv_dX;
	texture_yStep = dTy * inv_dY;

	odz_xStep = dZx * inv_dX;
	odz_yStep = dZy * inv_dY;


}



gfx_device_soft::edge::edge(const grad& grads, const vertex& start, const vertex& end, int startIndex) {
	yStart = (s32)ceil(start.position.y);
	yEnd = (s32)ceil(end.position.y);

	float yDist = end.position.y - start.position.y;
	float xDist = end.position.x - start.position.x;

	float yPrestep = yStart - start.position.y;

	xStep = xDist / yDist;
	x = start.position.x + yPrestep * xStep;

	float xPrestep = x - start.position.x;

	color = grads.color[startIndex] + grads.color_yStep * yPrestep + grads.color_xStep * xPrestep;
	colorStep = grads.color_yStep + grads.color_xStep * xStep;

	texCoord = grads.texture[startIndex] + grads.texture_yStep * yPrestep + grads.texture_xStep * xPrestep;
	texCoordStep = grads.texture_yStep + grads.texture_xStep * xStep;

	odz = grads.odz[startIndex] + grads.odz_yStep * yPrestep + grads.odz_xStep * xPrestep;
	odzStep = grads.odz_yStep + grads.odz_xStep * xStep;
}

void gfx_device_soft::edge::step() {
	x += xStep;
	color = color + colorStep;
	texCoord = texCoord + texCoordStep;
	odz = odz + odzStep;
}

gfx_device_soft::gfx_device_soft(int w, int h) {
   width = w;
   height = h;
	backbuffer = new u8[w * h * 4];
	depthBuffer = new GLdouble[w * h];
	screenSpaceMatrix = mat4::screenSpace((float)w / 2.0f, (float)h / 2.0f);
}

gfx_device_soft::~gfx_device_soft() {
		if(backbuffer) delete backbuffer;
		if(depthBuffer) delete depthBuffer;
	}

void gfx_device_soft::clear(u8 r, u8 g, u8 b, u8 a) {
		for(int  i = 0; i < width * height; i++) {
			backbuffer[i * 4] = b;
			backbuffer[i * 4 + 1] = g;
			backbuffer[i * 4 + 2] = r;
			backbuffer[i * 4 + 3] = a;
		}
	}

void gfx_device_soft::clearDepth(GLdouble d) {
	for(int  i = 0; i < width * height; i++) {
		depthBuffer[i] = d;
	}
}

void gfx_device_soft::drawPixel(int x, int y, u8 r, u8 g, u8 b, u8 a) {
	if(x < 0 || y < 0 || x >= width || y >= height) return;
	int index = (x + y * width) * 4;
	backbuffer[index] = b;
	backbuffer[index + 1] = g;
	backbuffer[index + 2] = r;
	backbuffer[index + 3] = a;
}

vec4 gfx_device_soft::getPixel(int x, int y) {
	if(x < 0 || y < 0 || x >= width || y >= height) return vec4();
	int index = (x + y * width) * 4;

	vec4 result;
	result.z = backbuffer[index];
	result.y = backbuffer[index + 1];
	result.x = backbuffer[index + 2];
	result.w = backbuffer[index + 3];

	return result;
}

void gfx_device_soft::fillTriangle(const mat4& mvp, const vertex& v1, const vertex& v2, const vertex& v3) {
	auto w_divide = [](const vec4& v) {
		return vec4(v.x / v.w, v.y / v.w, v.z / v.w, v.w);
	};

	vertex minY = vertex(w_divide(screenSpaceMatrix * mvp * v1.position), v1.color, v1.textureCoord, v1.normal);
	vertex midY = vertex(w_divide(screenSpaceMatrix * mvp * v2.position), v2.color, v2.textureCoord, v2.normal);
	vertex maxY = vertex(w_divide(screenSpaceMatrix * mvp * v3.position), v3.color, v3.textureCoord, v3.normal);



	if(maxY.position.y < midY.position.y) {
		vertex temp = vertex(maxY);
		maxY = vertex(midY);
		midY = vertex(temp);
	}

	if(midY.position.y < minY.position.y) {
		vertex temp = vertex(midY);
		midY = vertex(minY);
		minY = vertex(temp);
	}

	if(maxY.position.y < midY.position.y) {
		vertex temp = vertex(maxY);
		maxY = vertex(midY);
		midY = vertex(temp);
	}

	auto triangleArea = [](vec4& a, vec4& b, vec4& c) {
		float x1 = b.x - a.x;
		float y1 = b.y - a.y;

		float x2 = c.x - a.x;
		float y2 = c.y - a.y;

		return (x1 * y2 - x2 * y1);
	};

	float area = triangleArea(minY.position, maxY.position, midY.position);
	//int handedness = area >= 0 ? 1 : 0;

	scanTriangle(minY, midY, maxY, area >= 0);
	//fillShape((s32)ceil(minY.y), (s32)ceil(maxY.y));
}

void gfx_device_soft::scanTriangle(vertex& minY, vertex& midY, vertex& maxY, bool handedness) {
	grad grads = grad(minY, midY, maxY);
	edge top_bottom = edge(grads, minY, maxY, 0);
	edge top_middle = edge(grads, minY, midY, 0);
	edge middle_bottom = edge(grads, midY, maxY, 1);

	auto scanEdges = [&grads, &handedness, this](edge& l, edge& r) {
		edge* left = &l;
		edge* right = &r;
		if(handedness) {
			edge* temp = left;
			left = right;
			right = temp;
		}

		int yStart = r.yStart;
		int yEnd = r.yEnd;

		gfx_texture* text = NULL;
		for(unsigned int i = 0; i < this->g_state->textures.size(); i++) {
			if(this->g_state->textures[i].tname == this->g_state->currentBoundTexture) {
				text = &this->g_state->textures[i];
				break;
			}
		}

		for (int j = yStart; j < yEnd; j++) {
			s32 xMin = (s32)ceil(left->x);
			s32 xMax = (s32)ceil(right->x);
			float xPrestep = xMin - left->x;
			float xDist = right->x - left->x;
			vec4 texCoordStep = (right->texCoord - left->texCoord) * (1.0f / xDist);
			float odzStep = (right->odz - left->odz) / xDist;
			vec4 color = left->color + grads.color_xStep * xPrestep;
			vec4 texCoord = left->texCoord + grads.texture_xStep * xPrestep;
			float odz = left->odz + odzStep * xPrestep;

			for(s32 i = xMin; i < xMax; i++) {
				u8 r = 128, g = 128, b = 128, a = 255;
				r = (u8)(color.x * 255.0f + 0.5f);
				g = (u8)(color.y * 255.0f + 0.5f);
				b = (u8)(color.z * 255.0f + 0.5f);
				a = (u8)(color.w * 255.0f + 0.5f);

				// if(odz < 0.0f) continue;// Hack to cull pixels that are behind the camera.
				if(this->g_state->enableDepthTest) {
					if(odz < depthBuffer[i + j * width]) {
						depthBuffer[i + j * width] = odz;
					} else {
						continue;
					}
				}

				if(this->g_state->enableTexture2D) {
					if(text) {
						float z = 1.0f / odz;
						int srcX = (GLsizei)(clampf(texCoord.x, 0.0f, 1.0f) * z * (text->width - 1) + 0.0f);
						int srcY = (GLsizei)(clampf(texCoord.y, 0.0f, 1.0f) * z * (text->height - 1) + 0.0f);

						int index = (srcX + srcY * text->width) * 4;
						r = text->colorBuffer[index];
						g = text->colorBuffer[index + 1];
						b = text->colorBuffer[index + 2];
						a = text->colorBuffer[index + 3];
					} else {
						r = 0;
						g = 255;
						b = 0;
						a = 255;
					}
				}

				if(this->g_state->enableBlend) {
					vec4 srcAlpha;
					vec4 dstAlpha;
					switch(this->g_state->blendSrcFactor) {
						case (GL_SRC_ALPHA): {
							float As = (float)a / 255.0f;
							srcAlpha = vec4(As, As, As, As);
						} break;

						case (GL_ONE_MINUS_SRC_ALPHA): {
							float As = 1.0f - ((float)a / 255.0f);
							srcAlpha = vec4(As, As, As, As);
						}
					}

					switch(this->g_state->blendDstFactor) {
						case (GL_SRC_ALPHA): {
							float As = (float)a / 255.0f;
							dstAlpha = vec4(As, As, As, As);
						} break;

						case (GL_ONE_MINUS_SRC_ALPHA): {
							float As = 1.0f - ((float)a / 255.0f);
							dstAlpha = vec4(As, As, As, As);
						}
					}
					vec4 srcColor = vec4(r, g, b, a);
					vec4 dstColor = getPixel(i, j);
					vec4 finalColor = vec4(min(255.0f, srcColor.x * srcAlpha.x + dstColor.x * dstAlpha.x),
										min(255.0f, srcColor.y * srcAlpha.y + dstColor.y * dstAlpha.y),
										min(255.0f, srcColor.z * srcAlpha.z + dstColor.z * dstAlpha.z),
										min(255.0f, srcColor.w * srcAlpha.w + dstColor.w * dstAlpha.w));
					r = (u8)finalColor.x;
					g = (u8)finalColor.y;
					b = (u8)finalColor.z;
					a = (u8)finalColor.w;

				}
				drawPixel(i, j, r, g, b, a);
				color = color + grads.color_xStep;
				texCoord = texCoord + texCoordStep;
				odz = odz + odzStep;
			}
			left->step();
			right->step();
		}
	};
	scanEdges(top_bottom, top_middle);
	scanEdges(top_bottom, middle_bottom);




}


void gfx_device_soft::flush(u8* fb) {
	for(int i = 0; i < width * height; i++) {
		fb[i * 3] = backbuffer[i * 4];
		fb[i * 3 + 1] = backbuffer[i * 4 + 1];
		fb[i * 3 + 2] = backbuffer[i * 4 + 2];
	}
}

void gfx_device_soft::render_vertices(const mat4& mvp) {
   for(unsigned int i = 0; i < g_state->vertexBuffer.size(); i += 3) {
   	vertex v1 = g_state->vertexBuffer[i];
   	vertex v2 = g_state->vertexBuffer[i+1];
   	vertex v3 = g_state->vertexBuffer[i+2];
   	fillTriangle(mvp, v1, v2, v3);
   }
}

#endif
