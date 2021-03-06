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
inline static struct Matrix4F createZeroMatrixF4()
{
	struct Matrix4F matrix;
	matrix.m00 = 0.0f;
	matrix.m01 = 0.0f;
	matrix.m02 = 0.0f;
	matrix.m03 = 0.0f;

	matrix.m10 = 0.0f;
	matrix.m11 = 0.0f;
	matrix.m12 = 0.0f;
	matrix.m13 = 0.0f;

	matrix.m20 = 0.0f;
	matrix.m21 = 0.0f;
	matrix.m22 = 0.0f;
	matrix.m23 = 0.0f;

	matrix.m30 = 0.0f;
	matrix.m31 = 0.0f;
	matrix.m32 = 0.0f;
	matrix.m33 = 0.0f;
	return matrix;
}
inline static struct Matrix4F createOneMatrix4F()
{
	struct Matrix4F matrix;
	matrix.m00 = 1.0f;
	matrix.m01 = 1.0f;
	matrix.m02 = 1.0f;
	matrix.m03 = 1.0f;

	matrix.m10 = 1.0f;
	matrix.m11 = 1.0f;
	matrix.m12 = 1.0f;
	matrix.m13 = 1.0f;

	matrix.m20 = 1.0f;
	matrix.m21 = 1.0f;
	matrix.m22 = 1.0f;
	matrix.m23 = 1.0f;

	matrix.m30 = 1.0f;
	matrix.m31 = 1.0f;
	matrix.m32 = 1.0f;
	matrix.m33 = 1.0f;
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

inline static struct Matrix4F addMatrix4F(
	struct Matrix4F a,
	struct Matrix4F b)
{
	a.m00 += b.m00; a.m01 += b.m01; a.m02 += b.m02; a.m03 += b.m03;
	a.m10 += b.m10; a.m11 += b.m11; a.m12 += b.m12; a.m13 += b.m13;
	a.m20 += b.m20; a.m21 += b.m21; a.m22 += b.m22; a.m23 += b.m23;
	a.m30 += b.m30; a.m31 += b.m31; a.m32 += b.m32; a.m33 += b.m33;
	return a;
}
inline static struct Matrix4F subMatrix4F(
	struct Matrix4F a,
	struct Matrix4F b)
{
	a.m00 -= b.m00; a.m01 -= b.m01; a.m02 -= b.m02; a.m03 -= b.m03;
	a.m10 -= b.m10; a.m11 -= b.m11; a.m12 -= b.m12; a.m13 -= b.m13;
	a.m20 -= b.m20; a.m21 -= b.m21; a.m22 -= b.m22; a.m23 -= b.m23;
	a.m30 -= b.m30; a.m31 -= b.m31; a.m32 -= b.m32; a.m33 -= b.m33;
	return a;
}
inline static struct Matrix4F mulMatrix4F(
	struct Matrix4F a,
	struct Matrix4F b)
{
	a.m00 *= b.m00; a.m01 *= b.m01; a.m02 *= b.m02; a.m03 *= b.m03;
	a.m10 *= b.m10; a.m11 *= b.m11; a.m12 *= b.m12; a.m13 *= b.m13;
	a.m20 *= b.m20; a.m21 *= b.m21; a.m22 *= b.m22; a.m23 *= b.m23;
	a.m30 *= b.m30; a.m31 *= b.m31; a.m32 *= b.m32; a.m33 *= b.m33;
	return a;
}
inline static struct Matrix4F divMatrix4F(
	struct Matrix4F a,
	struct Matrix4F b)
{
	a.m00 /= b.m00; a.m01 /= b.m01; a.m02 /= b.m02; a.m03 /= b.m03;
	a.m10 /= b.m10; a.m11 /= b.m11; a.m12 /= b.m12; a.m13 /= b.m13;
	a.m20 /= b.m20; a.m21 /= b.m21; a.m22 /= b.m22; a.m23 /= b.m23;
	a.m30 /= b.m30; a.m31 /= b.m31; a.m32 /= b.m32; a.m33 /= b.m33;
	return a;
}

inline static struct Matrix4F addValueMatrix4F(
	struct Matrix4F matrix,
	float value)
{
	matrix.m00 += value;
	matrix.m01 += value;
	matrix.m02 += value;
	matrix.m03 += value;

	matrix.m10 += value;
	matrix.m11 += value;
	matrix.m12 += value;
	matrix.m13 += value;

	matrix.m20 += value;
	matrix.m21 += value;
	matrix.m22 += value;
	matrix.m23 += value;

	matrix.m30 += value;
	matrix.m31 += value;
	matrix.m32 += value;
	matrix.m33 += value;
	return matrix;
}
inline static struct Matrix4F subValueMatrix4F(
	struct Matrix4F matrix,
	float value)
{
	matrix.m00 -= value;
	matrix.m01 -= value;
	matrix.m02 -= value;
	matrix.m03 -= value;

	matrix.m10 -= value;
	matrix.m11 -= value;
	matrix.m12 -= value;
	matrix.m13 -= value;

	matrix.m20 -= value;
	matrix.m21 -= value;
	matrix.m22 -= value;
	matrix.m23 -= value;

	matrix.m30 -= value;
	matrix.m31 -= value;
	matrix.m32 -= value;
	matrix.m33 -= value;
	return matrix;
}
inline static struct Matrix4F mulValueMatrix4F(
	struct Matrix4F matrix,
	float value)
{
	matrix.m00 *= value;
	matrix.m01 *= value;
	matrix.m02 *= value;
	matrix.m03 *= value;

	matrix.m10 *= value;
	matrix.m11 *= value;
	matrix.m12 *= value;
	matrix.m13 *= value;

	matrix.m20 *= value;
	matrix.m21 *= value;
	matrix.m22 *= value;
	matrix.m23 *= value;

	matrix.m30 *= value;
	matrix.m31 *= value;
	matrix.m32 *= value;
	matrix.m33 *= value;
	return matrix;
}
inline static struct Matrix4F divValueMatrix4F(
	struct Matrix4F matrix,
	float value)
{
	matrix.m00 /= value;
	matrix.m01 /= value;
	matrix.m02 /= value;
	matrix.m03 /= value;

	matrix.m10 /= value;
	matrix.m11 /= value;
	matrix.m12 /= value;
	matrix.m13 /= value;

	matrix.m20 /= value;
	matrix.m21 /= value;
	matrix.m22 /= value;
	matrix.m23 /= value;

	matrix.m30 /= value;
	matrix.m31 /= value;
	matrix.m32 /= value;
	matrix.m33 /= value;
	return matrix;
}

inline static struct Matrix4F negMatrix4F(
	struct Matrix4F matrix)
{
	matrix.m00 = -matrix.m00;
	matrix.m01 = -matrix.m01;
	matrix.m02 = -matrix.m02;
	matrix.m03 = -matrix.m03;

	matrix.m10 = -matrix.m10;
	matrix.m11 = -matrix.m11;
	matrix.m12 = -matrix.m12;
	matrix.m13 = -matrix.m13;

	matrix.m20 = -matrix.m20;
	matrix.m21 = -matrix.m21;
	matrix.m22 = -matrix.m22;
	matrix.m23 = -matrix.m23;

	matrix.m30 = -matrix.m30;
	matrix.m31 = -matrix.m31;
	matrix.m32 = -matrix.m32;
	matrix.m33 = -matrix.m33;
	return matrix;
}

inline static struct Matrix4F dotMatrix4F(
	struct Matrix4F a,
	struct Matrix4F b)
{
	a.m00 = a.m00 * b.m00 + a.m01 * b.m10 + a.m02 * b.m20 + a.m03 * b.m30;
	a.m01 = a.m00 * b.m01 + a.m01 * b.m11 + a.m02 * b.m21 + a.m03 * b.m31;
	a.m02 = a.m00 * b.m02 + a.m01 * b.m12 + a.m02 * b.m22 + a.m03 * b.m32;
	a.m03 = a.m00 * b.m03 + a.m01 * b.m13 + a.m02 * b.m23 + a.m03 * b.m33;
	a.m10 = a.m10 * b.m00 + a.m11 * b.m10 + a.m12 * b.m20 + a.m13 * b.m30;
	a.m11 = a.m10 * b.m01 + a.m11 * b.m11 + a.m12 * b.m21 + a.m13 * b.m31;
	a.m12 = a.m10 * b.m02 + a.m11 * b.m12 + a.m12 * b.m22 + a.m13 * b.m32;
	a.m13 = a.m10 * b.m03 + a.m11 * b.m13 + a.m12 * b.m23 + a.m13 * b.m33;
	a.m20 = a.m20 * b.m00 + a.m21 * b.m10 + a.m22 * b.m20 + a.m23 * b.m30;
	a.m21 = a.m20 * b.m01 + a.m21 * b.m11 + a.m22 * b.m21 + a.m23 * b.m31;
	a.m22 = a.m20 * b.m02 + a.m21 * b.m12 + a.m22 * b.m22 + a.m23 * b.m32;
	a.m23 = a.m20 * b.m03 + a.m21 * b.m13 + a.m22 * b.m23 + a.m23 * b.m33;
	a.m30 = a.m30 * b.m00 + a.m31 * b.m10 + a.m32 * b.m20 + a.m33 * b.m30;
	a.m31 = a.m30 * b.m01 + a.m31 * b.m11 + a.m32 * b.m21 + a.m33 * b.m31;
	a.m32 = a.m30 * b.m02 + a.m31 * b.m12 + a.m32 * b.m22 + a.m33 * b.m32;
	a.m33 = a.m30 * b.m03 + a.m31 * b.m13 + a.m32 * b.m23 + a.m33 * b.m33;
	return a;
}
inline static struct Vector4F dotVectorMatrix4F(
	struct Matrix4F matrix,
	struct Vector4F vector)
{
	vector.x =
		matrix.m00 * vector.x +
		matrix.m01 * vector.x +
		matrix.m02 * vector.x +
		matrix.m03 * vector.x;
	vector.y =
		matrix.m10 * vector.y +
		matrix.m11 * vector.y +
		matrix.m12 * vector.y +
		matrix.m13 * vector.y;
	vector.z =
		matrix.m20 * vector.z +
		matrix.m21 * vector.z +
		matrix.m22 * vector.z +
		matrix.m23 * vector.z;
	vector.w =
		matrix.m30 * vector.w +
		matrix.m31 * vector.w +
		matrix.m32 * vector.w +
		matrix.m33 * vector.w;
	return vector;
}
inline static float determinantMatrix4F(
	struct Matrix4F matrix)
{
	float f0 = matrix.m22 * matrix.m33 - matrix.m32 * matrix.m23;
	float f1 = matrix.m21 * matrix.m33 - matrix.m31 * matrix.m23;
	float f2 = matrix.m21 * matrix.m32 - matrix.m31 * matrix.m22;
	float f3 = matrix.m20 * matrix.m33 - matrix.m30 * matrix.m23;
	float f4 = matrix.m20 * matrix.m32 - matrix.m30 * matrix.m22;
	float f5 = matrix.m20 * matrix.m31 - matrix.m30 * matrix.m21;

	float c0 = matrix.m11 * f0 - matrix.m12 * f1 + matrix.m13 * f2;
	float c1 = -(matrix.m10 * f0 - matrix.m12 * f3 + matrix.m13 * f4);
	float c2 = matrix.m10 * f1 -matrix. m11 * f3 + matrix.m13 * f5;
	float c3 = -(matrix.m10 * f2 - matrix.m11 * f4 + matrix.m12 * f5);

	return matrix.m00 * c0 + matrix.m01 * c1 + matrix.m02 * c2 + matrix.m03 * c3;
}
inline static struct Matrix4F transposeMatrix4F(
	struct Matrix4F matrix)
{
	matrix.m00 = matrix.m00;
	matrix.m01 = matrix.m10;
	matrix.m02 = matrix.m20;
	matrix.m03 = matrix.m30;

	matrix.m10 = matrix.m01;
	matrix.m11 = matrix.m11;
	matrix.m12 = matrix.m21;
	matrix.m13 = matrix.m31;

	matrix.m20 = matrix.m02;
	matrix.m21 = matrix.m12;
	matrix.m22 = matrix.m22;
	matrix.m23 = matrix.m32;

	matrix.m30 = matrix.m03;
	matrix.m31 = matrix.m13;
	matrix.m32 = matrix.m23;
	matrix.m33 = matrix.m33;
	return matrix;
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
	struct Matrix4F matrix,
	struct Vector3F scale)
{
	matrix.m00 *= scale.x;
	matrix.m01 *= scale.x;
	matrix.m02 *= scale.x;
	matrix.m03 *= scale.x;

	matrix.m10 *= scale.y;
	matrix.m11 *= scale.y;
	matrix.m12 *= scale.y;
	matrix.m13 *= scale.y;

	matrix.m20 *= scale.z;
	matrix.m21 *= scale.z;
	matrix.m22 *= scale.z;
	matrix.m23 *= scale.z;
	return matrix;
}
inline static struct Vector3F getScaleMatrix4F(
	struct Matrix4F matrix)
{
	struct Vector3F scale;
	struct Vector3F vector;

	vector.x = matrix.m00;
	vector.y = matrix.m01;
	vector.x = matrix.m02;
	scale.x = lengthVector3F(vector);

	vector.x = matrix.m10;
	vector.y = matrix.m11;
	vector.x = matrix.m12;
	scale.y = lengthVector3F(vector);

	vector.x = matrix.m20;
	vector.y = matrix.m21;
	vector.x = matrix.m22;
	scale.z = lengthVector3F(vector);

	return scale;
}
inline static struct Matrix4F translateMatrix4F(
	struct Matrix4F matrix,
	struct Vector3F translation)
{
	matrix.m30 =
		matrix.m00 * translation.x +
		matrix.m10 * translation.y +
		matrix.m20 * translation.z + matrix.m30;
	matrix.m31 =
		matrix.m01 * translation.x +
		matrix.m11 * translation.y +
		matrix.m21 * translation.z + matrix.m31;
	matrix.m32 =
		matrix.m02 * translation.x +
		matrix.m12 * translation.y +
		matrix.m22 * translation.z + matrix.m32;
	matrix.m33 =
		matrix.m03 * translation.x +
		matrix.m13 * translation.y +
		matrix.m23 * translation.z + matrix.m33;
	return matrix;
}
inline static struct Vector3F getTranslationMatrix4F(
	struct Matrix4F matrix)
{
	struct Vector3F translation;
	translation.x = matrix.m30;
	translation.y = matrix.m31;
	translation.z = matrix.m32;
	return translation;
}
