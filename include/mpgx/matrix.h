#pragma once
#include "mpgx/vector.h"

struct Matrix4F
{
	float m00, m01, m02, m03;
	float m10, m11, m12, m13;
	float m20, m21, m22, m23;
	float m30, m31, m32, m33;
};

inline static struct Matrix4F createMatrixF4(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	struct Matrix4F matrix;
	matrix.m00 = m00;
	matrix.m01 = m01;
	matrix.m02 = m02;
	matrix.m03 = m03;

	matrix.m10 = m10;
	matrix.m11 = m11;
	matrix.m12 = m12;
	matrix.m13 = m13;

	matrix.m20 = m20;
	matrix.m21 = m21;
	matrix.m22 = m22;
	matrix.m23 = m23;

	matrix.m30 = m30;
	matrix.m31 = m31;
	matrix.m32 = m32;
	matrix.m33 = m33;
	return matrix;
}
inline static struct Matrix4F createValueMatrixF4(
	float value)
{
	struct Matrix4F matrix;
	matrix.m00 = value;
	matrix.m01 = value;
	matrix.m02 = value;
	matrix.m03 = value;

	matrix.m10 = value;
	matrix.m11 = value;
	matrix.m12 = value;
	matrix.m13 = value;

	matrix.m20 = value;
	matrix.m21 = value;
	matrix.m22 = value;
	matrix.m23 = value;

	matrix.m30 = value;
	matrix.m31 = value;
	matrix.m32 = value;
	matrix.m33 = value;
	return matrix;
}
inline static struct Matrix4F createVectorMatrixF4(
	struct Vector4F column0,
	struct Vector4F column1,
	struct Vector4F column2,
	struct Vector4F column3)
{
	struct Matrix4F matrix;
	matrix.m00 = column0.x;
	matrix.m01 = column0.y;
	matrix.m02 = column0.z;
	matrix.m03 = column0.w;

	matrix.m10 = column1.x;
	matrix.m11 = column1.y;
	matrix.m12 = column1.z;
	matrix.m13 = column1.w;

	matrix.m20 = column2.x;
	matrix.m21 = column2.y;
	matrix.m22 = column2.z;
	matrix.m23 = column2.w;

	matrix.m30 = column3.x;
	matrix.m31 = column3.y;
	matrix.m32 = column3.z;
	matrix.m33 = column3.w;
	return matrix;
}
inline static struct Matrix4F createIdentityMatrix4F()
{
	struct Matrix4F matrix;
	matrix.m00 = 1.0f;
	matrix.m01 = 0.0f;
	matrix.m02 = 0.0f;
	matrix.m03 = 0.0f;

	matrix.m10 = 0.0f;
	matrix.m11 = 1.0f;
	matrix.m12 = 0.0f;
	matrix.m13 = 0.0f;

	matrix.m20 = 0.0f;
	matrix.m21 = 0.0f;
	matrix.m22 = 1.0f;
	matrix.m23 = 0.0f;

	matrix.m30 = 0.0f;
	matrix.m31 = 0.0f;
	matrix.m32 = 0.0f;
	matrix.m33 = 1.0f;
	return matrix;
}

inline static struct Vector4F getColumn0Matrix4F(
	struct Matrix4F matrix)
{
	return createVector4F(
		matrix.m00,
		matrix.m01,
		matrix.m02,
		matrix.m03);
}
inline static struct Vector4F getColumn1Matrix4F(
	struct Matrix4F matrix)
{
	return createVector4F(
		matrix.m10,
		matrix.m11,
		matrix.m12,
		matrix.m13);
}
inline static struct Vector4F getColumn2Matrix4F(
	struct Matrix4F matrix)
{
	return createVector4F(
		matrix.m20,
		matrix.m21,
		matrix.m22,
		matrix.m23);
}
inline static struct Vector4F getColumn3Matrix4F(
	struct Matrix4F matrix)
{
	return createVector4F(
		matrix.m30,
		matrix.m31,
		matrix.m32,
		matrix.m33);
}

inline static struct Vector4F getRow0Matrix4F(
	struct Matrix4F matrix)
{
	return createVector4F(
		matrix.m00,
		matrix.m10,
		matrix.m20,
		matrix.m30);
}
inline static struct Vector4F getRow1Matrix4F(
	struct Matrix4F matrix)
{
	return createVector4F(
		matrix.m01,
		matrix.m11,
		matrix.m21,
		matrix.m31);
}
inline static struct Vector4F getRow2Matrix4F(
	struct Matrix4F matrix)
{
	return createVector4F(
		matrix.m02,
		matrix.m12,
		matrix.m22,
		matrix.m32);
}
inline static struct Vector4F getRow3Matrix4F(
	struct Matrix4F matrix)
{
	return createVector4F(
		matrix.m03,
		matrix.m13,
		matrix.m23,
		matrix.m33);
}

inline static struct Matrix4F addMatrix4F(
	struct Matrix4F a,
	struct Matrix4F b)
{
	return createVectorMatrixF4(
		addVector4F(getColumn0Matrix4F(a), getColumn0Matrix4F(b)),
		addVector4F(getColumn1Matrix4F(a), getColumn1Matrix4F(b)),
		addVector4F(getColumn2Matrix4F(a), getColumn2Matrix4F(b)),
		addVector4F(getColumn3Matrix4F(a), getColumn3Matrix4F(b)));
}
inline static struct Matrix4F subMatrix4F(
	struct Matrix4F a,
	struct Matrix4F b)
{
	return createVectorMatrixF4(
		subVector4F(getColumn0Matrix4F(a), getColumn0Matrix4F(b)),
		subVector4F(getColumn1Matrix4F(a), getColumn1Matrix4F(b)),
		subVector4F(getColumn2Matrix4F(a), getColumn2Matrix4F(b)),
		subVector4F(getColumn3Matrix4F(a), getColumn3Matrix4F(b)));
}
inline static struct Matrix4F mulMatrix4F(
	struct Matrix4F a,
	struct Matrix4F b)
{
	return createVectorMatrixF4(
		mulVector4F(getColumn0Matrix4F(a), getColumn0Matrix4F(b)),
		mulVector4F(getColumn1Matrix4F(a), getColumn1Matrix4F(b)),
		mulVector4F(getColumn2Matrix4F(a), getColumn2Matrix4F(b)),
		mulVector4F(getColumn3Matrix4F(a), getColumn3Matrix4F(b)));
}
inline static struct Matrix4F divMatrix4F(
	struct Matrix4F a,
	struct Matrix4F b)
{
	return createVectorMatrixF4(
		divVector4F(getColumn0Matrix4F(a), getColumn0Matrix4F(b)),
		divVector4F(getColumn1Matrix4F(a), getColumn1Matrix4F(b)),
		divVector4F(getColumn2Matrix4F(a), getColumn2Matrix4F(b)),
		divVector4F(getColumn3Matrix4F(a), getColumn3Matrix4F(b)));
}

inline static float determinantMatrix4F(
	struct Matrix4F m)
{
	float f0 = m.m22 * m.m33 - m.m32 * m.m23;
	float f1 = m.m21 * m.m33 - m.m31 * m.m23;
	float f2 = m.m21 * m.m32 - m.m31 * m.m22;
	float f3 = m.m20 * m.m33 - m.m30 * m.m23;
	float f4 = m.m20 * m.m32 - m.m30 * m.m22;
	float f5 = m.m20 * m.m31 - m.m30 * m.m21;

	float c0 = m.m11 * f0 - m.m12 * f1 + m.m13 * f2;
	float c1 = -(m.m10 * f0 - m.m12 * f3 + m.m13 * f4);
	float c2 = m.m10 * f1 -m. m11 * f3 + m.m13 * f5;
	float c3 = -(m.m10 * f2 - m.m11 * f4 + m.m12 * f5);

	return m.m00 * c0 + m.m01 * c1 + m.m02 * c2 + m.m03 * c3;
}
inline static struct Matrix4F transposeMatrix4F(
	struct Matrix4F m)
{
	m.m00 = m.m00;
	m.m01 = m.m10;
	m.m02 = m.m20;
	m.m03 = m.m30;

	m.m10 = m.m01;
	m.m11 = m.m11;
	m.m12 = m.m21;
	m.m13 = m.m31;

	m.m20 = m.m02;
	m.m21 = m.m12;
	m.m22 = m.m22;
	m.m23 = m.m32;

	m.m30 = m.m03;
	m.m31 = m.m13;
	m.m32 = m.m23;
	m.m33 = m.m33;
	return m;
}
inline static struct Matrix4F inverseMatrix4F(
	struct Matrix4F matrix)
{
	float s[6];
	float c[6];

	s[0] = matrix.m00 * matrix.m11 - matrix.m10 * matrix.m01;
	s[1] = matrix.m00 * matrix.m12 - matrix.m10 * matrix.m02;
	s[2] = matrix.m00 * matrix.m13 - matrix.m10 * matrix.m03;
	s[3] = matrix.m01 * matrix.m12 - matrix.m11 * matrix.m02;
	s[4] = matrix.m01 * matrix.m13 - matrix.m11 * matrix.m03;
	s[5] = matrix.m02 * matrix.m13 - matrix.m12 * matrix.m03;

	c[0] = matrix.m20 * matrix.m31 - matrix.m30 * matrix.m21;
	c[1] = matrix.m20 * matrix.m32 - matrix.m30 * matrix.m22;
	c[2] = matrix.m20 * matrix.m33 - matrix.m30 * matrix.m23;
	c[3] = matrix.m21 * matrix.m32 - matrix.m31 * matrix.m22;
	c[4] = matrix.m21 * matrix.m33 - matrix.m31 * matrix.m23;
	c[5] = matrix.m22 * matrix.m33 - matrix.m32 * matrix.m23;

	float id = 1.0f /
		(s[0] * c[5] - s[1] * c[4] +
		s[2] * c[3] + s[3] * c[2] -
		s[4] * c[1] + s[5] * c[0]);

	matrix.m00 = (matrix.m11 * c[5] - matrix.m12 * c[4] + matrix.m13 * c[3]) * id;
	matrix.m01 = (-matrix.m01 * c[5] + matrix.m02 * c[4] - matrix.m03 * c[3]) * id;
	matrix.m02 = (matrix.m31 * s[5] - matrix.m32 * s[4] + matrix.m33 * s[3]) * id;
	matrix.m03 = (-matrix.m21 * s[5] + matrix.m22 * s[4] - matrix.m23 * s[3]) * id;

	matrix.m10 = (-matrix.m10 * c[5] + matrix.m12 * c[2] - matrix.m13 * c[1]) * id;
	matrix.m11 = (matrix.m00 * c[5] - matrix.m02 * c[2] + matrix.m03 * c[1]) * id;
	matrix.m12 = (-matrix.m30 * s[5] + matrix.m32 * s[2] - matrix.m33 * s[1]) * id;
	matrix.m13 = (matrix.m20 * s[5] - matrix.m22 * s[2] + matrix.m23 * s[1]) * id;

	matrix.m20 = (matrix.m10 * c[4] - matrix.m11 * c[2] + matrix.m13 * c[0]) * id;
	matrix.m21 = (-matrix.m00 * c[4] + matrix.m01 * c[2] - matrix.m03 * c[0]) * id;
	matrix.m22 = (matrix.m30 * s[4] - matrix.m31 * s[2] + matrix.m33 * s[0]) * id;
	matrix.m23 = (-matrix.m20 * s[4] + matrix.m21 * s[2] - matrix.m23 * s[0]) * id;

	matrix.m30 = (-matrix.m10 * c[3] + matrix.m11 * c[1] - matrix.m12 * c[0]) * id;
	matrix.m31 = (matrix.m00 * c[3] - matrix.m01 * c[1] + matrix.m02 * c[0]) * id;
	matrix.m32 = (-matrix.m30 * s[3] + matrix.m31 * s[1] - matrix.m32 * s[0]) * id;
	matrix.m33 = (matrix.m20 * s[3] - matrix.m21 * s[1] + matrix.m22 * s[0]) * id;
	return matrix;
}
inline static struct Matrix4F scaleMatrix4F(
	struct Matrix4F matrix)
{

	matrix.setColumn0(getColumn0() * vector.x);
	matrix.setColumn1(getColumn1() * vector.y);
	matrix.setColumn2(getColumn2() * vector.z);
	matrix.setColumn3(getColumn3());
	return matrix;
}
inline static getTranslated(const Vector3<T>& vector) const noexcept
{
	auto result = Matrix4<T>(*this);
	result.setColumn3(
	getColumn0() * vector.x +
	getColumn1() * vector.y +
	getColumn2() * vector.z +
		getColumn3());
	return result;
}

template<class T = float>
static Matrix4<T> vkCreatePerspectiveMatrix(
	float fieldOfView,
	float aspectRatio,
	float nearClipPlane,
	float farClipPlane) noexcept
{
	auto tanHalfFov =
		tan(fieldOfView / static_cast<T>(2));

return Matrix4<T>(
static_cast<T>(1) / (aspectRatio * tanHalfFov),
static_cast<T>(0),
static_cast<T>(0),
static_cast<T>(0),

static_cast<T>(0),
static_cast<T>(-1) / tanHalfFov,
static_cast<T>(0),
static_cast<T>(0),

static_cast<T>(0),
static_cast<T>(0),
farClipPlane / (farClipPlane - nearClipPlane),
static_cast<T>(1),

static_cast<T>(0),
static_cast<T>(0),
-(farClipPlane * nearClipPlane) / (farClipPlane - nearClipPlane),
static_cast<T>(0));
}
template<class T = float>
static Matrix4<T> glCreatePerspectiveMatrix(
	float fieldOfView,
	float aspectRatio,
	float nearClipPlane,
	float farClipPlane) noexcept
{
auto tanHalfFov =
tan(fieldOfView / static_cast<T>(2));

return Matrix4<T>(
static_cast<T>(1) / (aspectRatio * tanHalfFov),
static_cast<T>(0),
static_cast<T>(0),
static_cast<T>(0),

static_cast<T>(0),
static_cast<T>(1) / tanHalfFov,
static_cast<T>(0),
static_cast<T>(0),

static_cast<T>(0),
static_cast<T>(0),
(farClipPlane + nearClipPlane) / (farClipPlane - nearClipPlane),
static_cast<T>(1),

static_cast<T>(0),
static_cast<T>(0),
-(static_cast<T>(2) * farClipPlane * nearClipPlane) / (farClipPlane - nearClipPlane),
static_cast<T>(0));
}
template<class T = float>
static Matrix4<T> vkCreateOrthographicMatrix(
	float left,
	float right,
	float bottom,
	float top,
	float nearClipPlane,
	float farClipPlane) noexcept
{
return Matrix4<T>(
static_cast<T>(2) / (right - left),
static_cast<T>(0),
static_cast<T>(0),
static_cast<T>(0),

static_cast<T>(0),
static_cast<T>(-2) / (top - bottom),
static_cast<T>(0),
static_cast<T>(0),

static_cast<T>(0),
static_cast<T>(0),
static_cast<T>(1) / (farClipPlane - nearClipPlane),
static_cast<T>(0),

-(right + left) / (right - left),
-(top + bottom) / (top - bottom),
-nearClipPlane / (farClipPlane - nearClipPlane),
static_cast<T>(1));
}
template<class T = float>
static Matrix4<T> glCreateOrthographicMatrix(
	float left,
	float right,
	float bottom,
	float top,
	float nearClipPlane,
	float farClipPlane) noexcept
{
return Matrix4<T>(
static_cast<T>(2) / (right - left),
static_cast<T>(0),
static_cast<T>(0),
static_cast<T>(0),

static_cast<T>(0),
static_cast<T>(2) / (top - bottom),
static_cast<T>(0),
static_cast<T>(0),

static_cast<T>(0),
static_cast<T>(0),
static_cast<T>(2) / (farClipPlane - nearClipPlane),
static_cast<T>(0),

-(right + left) / (right - left),
-(top + bottom) / (top - bottom),
-(farClipPlane + nearClipPlane) / (farClipPlane - nearClipPlane),
static_cast<T>(1));
}
