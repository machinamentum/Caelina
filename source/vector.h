
#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>


struct vec4 {
	float x, y, z, w;

	vec4(float mx = 0.0f, float my = 0.0f, float mz = 0.0f, float mw = 1.0f) {
		x = mx;
		y = my;
		z = mz;
		w = mw;
	}

	vec4(const vec4& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
	}

	float length() {
		return std::sqrt(x * x + y * y + z * z + w * w);
	}

	vec4 operator+(const vec4& v) const {
		return vec4(x + v.x, y + v.y, z + v.z, w + v.w);
	}

	vec4 operator-(const vec4& v) const {
		return vec4(x - v.x, y - v.y, z - v.z, w - v.w);
	}

	vec4 operator-() const {
		return vec4(-x, -y, -z, -w);
	}

	vec4 operator*(float s) const {
		return vec4(x * s, y * s, z * s, w * s);
	}


	/**
	 * @return the dot product of the two vectors
	 */
	float operator*(const vec4& v) const {
		return x * v.x + y * v.y + z * v.z + w * v.w;
	}

	vec4 cross(const vec4& b) {
		vec4 c;
		c.x = y * b.z - z * b.y;
		c.y = z * b.x - x * b.z;
		c.z = x * b.y - y * b.x;
		return c;
	}

	vec4 lerp(const vec4& v, float factor) const {
		return (v - (*this)) * factor + (*this);
	}


};


#include <ostream>

inline std::ostream& operator<<(std::ostream& os, const vec4& v)
{
	os << '[';
	os << v.x << ' ';
	os << v.y << ' ';
	os << v.z << ' ';
	os << v.w;
	os << ']' << std::endl;
    return os;
}






#endif
