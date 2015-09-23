
#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>

#include "vector.h"

class mat4 {

public:
    float m[4][4];
    mat4() {
        *this = identity();
    }

    mat4(float val) {
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                m[i][j] = val;
            }
        }
    }

    mat4(const mat4& mat) {
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                m[i][j] = mat.m[i][j];
            }
        }
    }

    float& operator[](unsigned int index) {
        return m[index / 4][index % 4];
    }

    float at(unsigned int index) const {
        return m[index / 4][index % 4];;
    }

    float determinate() const {
        return
        m[0][3] * m[1][2] * m[2][1] * m[3][0] - m[0][2] * m[1][3] * m[2][1] * m[3][0] -

        m[0][3] * m[1][1] * m[2][2] * m[3][0] + m[0][1] * m[1][3] * m[2][2] * m[3][0] +

        m[0][2] * m[1][1] * m[2][3] * m[3][0] - m[0][1] * m[1][2] * m[2][3] * m[3][0] -

        m[0][3] * m[1][2] * m[2][0] * m[3][1] + m[0][2] * m[1][3] * m[2][0] * m[3][1] +

        m[0][3] * m[1][0] * m[2][2] * m[3][1] - m[0][0] * m[1][3] * m[2][2] * m[3][1] -

        m[0][2] * m[1][0] * m[2][3] * m[3][1] + m[0][0] * m[1][2] * m[2][3] * m[3][1] +

        m[0][3] * m[1][1] * m[2][0] * m[3][2] - m[0][1] * m[1][3] * m[2][0] * m[3][2] -

        m[0][3] * m[1][0] * m[2][1] * m[3][2] + m[0][0] * m[1][3] * m[2][1] * m[3][2] +

        m[0][1] * m[1][0] * m[2][3] * m[3][2] - m[0][0] * m[1][1] * m[2][3] * m[3][2] -

        m[0][2] * m[1][1] * m[2][0] * m[3][3] + m[0][1] * m[1][2] * m[2][0] * m[3][3] +

        m[0][2] * m[1][0] * m[2][1] * m[3][3] - m[0][0] * m[1][2] * m[2][1] * m[3][3] -

        m[0][1] * m[1][0] * m[2][2] * m[3][3] + m[0][0] * m[1][1] * m[2][2] * m[3][3];
    }

    //TODO(josh) better calculations
    mat4 inverse() const {
        mat4 result;
        result.m[0][0] = m[1][1]*m[2][2]*m[3][3] + m[1][2]*m[2][3]*m[3][1] + m[1][3]*m[2][1]*m[3][2] -
								m[1][1]*m[2][3]*m[3][2] - m[1][2]*m[2][1]*m[3][3] - m[1][3]*m[2][2]*m[3][1];
        result.m[0][1] = m[0][1]*m[2][3]*m[3][2] + m[0][2]*m[2][1]*m[3][3] + m[0][3]*m[2][2]*m[3][1] -
								m[0][1]*m[2][2]*m[3][3] - m[0][2]*m[2][3]*m[3][1] - m[0][3]*m[2][1]*m[3][2];
        result.m[0][2] = m[0][1]*m[1][2]*m[3][3] + m[0][2]*m[1][3]*m[3][1] + m[0][3]*m[1][1]*m[3][2] -
								m[0][1]*m[1][3]*m[3][2] - m[0][2]*m[1][1]*m[3][3] - m[0][3]*m[1][2]*m[3][1];
        result.m[0][3] = m[0][1]*m[1][3]*m[2][2] + m[0][2]*m[1][1]*m[2][3] + m[0][3]*m[1][2]*m[2][1] -
								m[0][1]*m[1][2]*m[2][3] - m[0][2]*m[1][3]*m[2][1] - m[0][3]*m[1][1]*m[2][2];


        result.m[1][0] = m[1][0]*m[2][3]*m[3][2] + m[1][2]*m[2][0]*m[3][3] + m[1][3]*m[2][2]*m[3][0] -
								m[1][0]*m[2][2]*m[3][3] - m[1][2]*m[2][3]*m[3][0] - m[1][3]*m[2][0]*m[3][2];
        result.m[1][1] = m[0][0]*m[2][2]*m[3][3] + m[0][2]*m[2][3]*m[3][0] + m[0][3]*m[2][0]*m[3][2] -
								m[0][0]*m[2][3]*m[3][2] - m[0][2]*m[2][0]*m[3][3] - m[0][3]*m[2][2]*m[3][0];
        result.m[1][2] = m[0][0]*m[1][3]*m[3][2] + m[0][2]*m[1][0]*m[3][3] + m[0][3]*m[1][2]*m[3][0] -
								m[0][0]*m[1][2]*m[3][3] - m[0][2]*m[1][3]*m[3][0] - m[0][3]*m[1][0]*m[3][2];
        result.m[1][3] = m[0][0]*m[1][2]*m[2][3] + m[0][2]*m[1][3]*m[2][0] + m[0][3]*m[1][0]*m[2][2] -
								m[0][0]*m[1][3]*m[2][2] - m[0][2]*m[1][0]*m[2][3] - m[0][3]*m[1][2]*m[2][1];

        result.m[2][0] = m[1][0]*m[2][1]*m[3][3] + m[1][1]*m[2][3]*m[3][0] + m[1][3]*m[2][0]*m[3][1] -
								m[1][0]*m[2][3]*m[3][1] - m[1][1]*m[2][0]*m[3][3] - m[1][3]*m[2][1]*m[3][0];
        result.m[2][1] = m[0][0]*m[2][3]*m[3][1] + m[0][1]*m[2][0]*m[3][3] + m[0][3]*m[2][1]*m[3][0] -
								m[0][0]*m[2][1]*m[3][3] - m[0][1]*m[2][3]*m[3][0] - m[0][3]*m[2][0]*m[3][1];
        result.m[2][2] = m[0][0]*m[1][1]*m[3][3] + m[0][1]*m[1][3]*m[3][0] + m[0][3]*m[1][0]*m[3][1] -
								m[0][0]*m[1][3]*m[3][1] - m[0][1]*m[1][0]*m[3][3] - m[0][3]*m[1][1]*m[3][0];
        result.m[2][3] = m[0][0]*m[1][3]*m[2][1] + m[0][1]*m[1][0]*m[2][3] + m[0][3]*m[1][1]*m[2][0] -
								m[0][0]*m[1][1]*m[2][3] - m[0][1]*m[1][3]*m[2][0] - m[0][3]*m[1][0]*m[2][1];

        result.m[3][0] = m[1][0]*m[2][2]*m[3][1] + m[1][1]*m[2][0]*m[3][2] + m[1][2]*m[2][1]*m[3][0] -
								m[1][0]*m[2][1]*m[3][2] - m[1][1]*m[2][2]*m[3][0] - m[1][2]*m[2][0]*m[3][1];
        result.m[3][1] = m[0][0]*m[2][1]*m[3][2] + m[0][1]*m[2][2]*m[3][0] + m[0][2]*m[2][0]*m[3][1] -
								m[0][0]*m[2][2]*m[3][1] - m[0][1]*m[2][0]*m[3][2] - m[0][2]*m[2][1]*m[3][0];
        result.m[3][2] = m[0][0]*m[1][2]*m[3][1] + m[0][1]*m[1][0]*m[3][2] + m[0][2]*m[1][1]*m[3][0] -
								m[0][0]*m[1][1]*m[3][2] - m[0][1]*m[1][2]*m[3][0] - m[0][2]*m[1][0]*m[3][1];
        result.m[3][3] = m[0][0]*m[1][1]*m[2][2] + m[0][1]*m[1][2]*m[2][0] + m[0][2]*m[1][0]*m[2][1] -
								m[0][0]*m[1][2]*m[2][1] + m[0][1]*m[1][0]*m[2][2] + m[0][2]*m[1][1]*m[2][0];

        float det = 1.0/result.determinate();
        if (det != 0.0) {
            for (int i = 0; i < 16; ++i) {
                result[i] *= det;
            }
        }
        return result;
    }

    mat4 transpose() const {
        mat4 result;
        result.m[1][0] = m[0][1];
        result.m[2][0] = m[0][2];
        result.m[3][0] = m[0][3];

        result.m[0][1] = m[1][0];
        result.m[2][1] = m[1][2];
        result.m[3][1] = m[1][3];

        result.m[1][2] = m[2][1];
        result.m[0][2] = m[2][0];
        result.m[3][2] = m[2][3];

        result.m[1][3] = m[3][1];
        result.m[2][3] = m[3][2];
        result.m[0][3] = m[3][0];
        return result;
    }

    vec4 operator*(const vec4& v) {
        vec4 result;
        result.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w;
        result.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w;
        result.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w;

        result.w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w;

        return result;
    }

    mat4 operator*(const mat4& mat) {
        mat4 result;
        result[0] = m[0][0] * mat.m[0][0] + m[0][1] * mat.m[1][0] + m[0][2] * mat.m[2][0] + m[0][3] * mat.m[3][0];
        result[1] = m[0][0] * mat.m[0][1] + m[0][1] * mat.m[1][1] + m[0][2] * mat.m[2][1] + m[0][3] * mat.m[3][1];
        result[2] = m[0][0] * mat.m[0][2] + m[0][1] * mat.m[1][2] + m[0][2] * mat.m[2][2] + m[0][3] * mat.m[3][2];
        result[3] = m[0][0] * mat.m[0][3] + m[0][1] * mat.m[1][3] + m[0][2] * mat.m[2][3] + m[0][3] * mat.m[3][3];

        result[4] = m[1][0] * mat.m[0][0] + m[1][1] * mat.m[1][0] + m[1][2] * mat.m[2][0] + m[1][3] * mat.m[3][0];
        result[5] = m[1][0] * mat.m[0][1] + m[1][1] * mat.m[1][1] + m[1][2] * mat.m[2][1] + m[1][3] * mat.m[3][1];
        result[6] = m[1][0] * mat.m[0][2] + m[1][1] * mat.m[1][2] + m[1][2] * mat.m[2][2] + m[1][3] * mat.m[3][2];
        result[7] = m[1][0] * mat.m[0][3] + m[1][1] * mat.m[1][3] + m[1][2] * mat.m[2][3] + m[1][3] * mat.m[3][3];

        result[8] = m[2][0] * mat.m[0][0] + m[2][1] * mat.m[1][0] + m[2][2] * mat.m[2][0] + m[2][3] * mat.m[3][0];
        result[9] = m[2][0] * mat.m[0][1] + m[2][1] * mat.m[1][1] + m[2][2] * mat.m[2][1] + m[2][3] * mat.m[3][1];
        result[10] = m[2][0] * mat.m[0][2] + m[2][1] * mat.m[1][2] + m[2][2] * mat.m[2][2] + m[2][3] * mat.m[3][2];
        result[11] = m[2][0] * mat.m[0][3] + m[2][1] * mat.m[1][3] + m[2][2] * mat.m[2][3] + m[2][3] * mat.m[3][3];

        result[12] = m[3][0] * mat.m[0][0] + m[3][1] * mat.m[1][0] + m[3][2] * mat.m[2][0] + m[3][3] * mat.m[3][0];
        result[13] = m[3][0] * mat.m[0][1] + m[3][1] * mat.m[1][1] + m[3][2] * mat.m[2][1] + m[3][3] * mat.m[3][1];
        result[14] = m[3][0] * mat.m[0][2] + m[3][1] * mat.m[1][2] + m[3][2] * mat.m[2][2] + m[3][3] * mat.m[3][2];
        result[15] = m[3][0] * mat.m[0][3] + m[3][1] * mat.m[1][3] + m[3][2] * mat.m[2][3] + m[3][3] * mat.m[3][3];

        return result;
    }

    static mat4 screenSpace(float halfWidth, float halfHeight) {
        mat4 mat;
        mat.m[0][0] = halfWidth;
        mat.m[1][1] =-halfHeight;
        mat.m[0][3] = halfWidth;
        mat.m[1][3] = halfHeight;
        return mat;
    }

    static mat4 viewport(float x, float y, float width, float height) {
        mat4 scale = mat4::scale(width, height, 0.5f);
        mat4 trans = mat4::translate(x + x + width, y + y + height, 0.0f);
        mat4 correction = mat4::translate(-(1.0 / width), -(1.0 / height), 1.0);
        return trans * scale * correction;
    }

    static mat4 rotate(double degrees, float tx, float ty, float tz) {
        mat4 result;
        float x = tx, y = ty, z = tz;
        float length = std::sqrt(x * x + y * y + z * z);
        if(length == 0.0f) return 0;//NOTE use length == 0.0?
        x /= length;
        y /= length;
        z /= length;

        double angle = degrees * (3.14159265359 / 180.0);
        double c = std::cos(angle);
        float omc = 1.0f - c;
        float s = sin(angle);

        result[0] = x * x * omc + c;
        result[1] = x * y * omc - z * s;
        result[2] = x * z * omc + y * s;

        result[4] = y * x * omc + z * s;
        result[5] = y * y * omc + c;
        result[6] = y * z * omc - x * s;

        result.m[2][0] = x * z * omc - y * s;
        result.m[2][1] = y * z * omc + x * s;
        result.m[2][2] = z * z * omc + c;
        return result;
    }

    static mat4 translate(float x, float y, float z) {
        mat4 mat;
        mat.m[0][3] = x;
        mat.m[1][3] = y;
        mat.m[2][3] = z;
        return mat;
    }

    static mat4 scale(float x, float y, float z) {
        mat4 mat;
        mat.m[0][0] = x;
        mat.m[1][1] = y;
        mat.m[2][2] = z;
        return mat;
    }

    static mat4 perspective(float fov, float aspect, float near, float far) {
        float t = tan(fov * 3.14159 / 360.0) * near;
        float b = -t;
        float l = aspect * b;
        float r = aspect * t;
        return frustum(l, r, b, t, near, far);
    }

    static mat4 frustum(float left, float right, float bottom, float top, float near, float far) {
        mat4 mat;
        float A = (right + left) / (right - left);
        float B = (top + bottom) / (top - bottom);
        float C = - (far + near) / (far - near);
        float D = - (2 * far * near) / (far - near);
        
        mat.m[0][0] = 2 * near / (right - left);
        mat.m[0][2] = A;
        mat.m[1][1] = 2 * near / (top - bottom);
        mat.m[1][2] = B;
        mat.m[2][2] = C;
        mat.m[2][3] = D;
        mat.m[3][2] = -1.0f;
        mat.m[3][3] = 0.0f;
        return mat;
    }
    
    static mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
        mat4 mat;
        mat.m[0][0] = 2.0 / (right - left);
        mat.m[0][3] = - (right + left) / (right - left);
        mat.m[1][1] = 2.0 / (top - bottom);
        mat.m[1][3] = - (top + bottom) / (top - bottom);
        mat.m[2][2] = - 2.0 / (far - near);
        mat.m[2][3] = - (far + near) / (far - near);
        return mat;
    }
    
    static mat4 identity() {
        mat4 mat(0.0f);
        mat.m[0][0] = 1.0f;
        mat.m[1][1] = 1.0f;
        mat.m[2][2] = 1.0f;
        mat.m[3][3] = 1.0f;
        return mat;
    }
    
};



#endif
