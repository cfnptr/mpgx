#pragma once
#include "matrix.h"
#include <math.h>

struct PerspectiveCamera
{
	float fieldOfView;
	float aspectRatio;
	float nearClipPlane;
	float farClipPlane;
};
struct OrthographicCamera
{
	float leftFrustum;
	float rightFrustum;
	float bottomFrustum;
	float topFrustum;
	float nearClipPlane;
	float farClipPlane;
};

struct PerspectiveCamera createPerspectiveCamera(
	float fieldOfView,
	float aspectRatio,
	float nearClipPlane,
	float farClipPlane)
{
	struct PerspectiveCamera camera;
	camera.fieldOfView = fieldOfView;
	camera.aspectRatio = aspectRatio;
	camera.nearClipPlane = nearClipPlane;
	camera.farClipPlane = farClipPlane;
	return camera;
}
struct OrthographicCamera createOrthographicCamera(
	float leftFrustum,
	float rightFrustum,
	float bottomFrustum,
	float topFrustum,
	float nearClipPlane,
	float farClipPlane)
{
	struct OrthographicCamera camera;
	camera.leftFrustum = leftFrustum;
	camera.rightFrustum = rightFrustum;
	camera.bottomFrustum = bottomFrustum;
	camera.topFrustum = topFrustum;
	camera.nearClipPlane = nearClipPlane;
	camera.farClipPlane = farClipPlane;
	return camera;
}

inline static struct Matrix4F createVkPerspectiveMatrix4F(
	float fieldOfView,
	float aspectRatio,
	float nearClipPlane,
	float farClipPlane)
{
	float tanHalfFov = tanf(fieldOfView / 2.0f);

	struct Matrix4F m;
	m.m00 = 1.0f / (aspectRatio * tanHalfFov);
	m.m01 = 0.0f;
	m.m02 = 0.0f;
	m.m03 = 0.0f;

	m.m10 = 0.0f;
	m.m11 = -1.0f / tanHalfFov;
	m.m12 = 0.0f;
	m.m13 = 0.0f;

	m.m20 = 0.0f;
	m.m21 = 0.0f;
	m.m22 = farClipPlane / (farClipPlane - nearClipPlane);
	m.m23 = 0.0f;

	m.m30 = 0.0f;
	m.m31 = 0.0f;
	m.m32 = -(farClipPlane * nearClipPlane) / (farClipPlane - nearClipPlane);
	m.m33 = 0.0f;
	return m;
}
inline static struct Matrix4F createGlPerspectiveMatrix(
	float fieldOfView,
	float aspectRatio,
	float nearClipPlane,
	float farClipPlane)
{
	float tanHalfFov = tanf(fieldOfView / 2.0f);

	struct Matrix4F m;
	m.m00 = 1.0f / (aspectRatio * tanHalfFov);
	m.m01 = 0.0f;
	m.m02 = 0.0f;
	m.m03 = 0.0f;

	m.m10 = 0.0f;
	m.m11 = 1.0f / tanHalfFov;
	m.m12 = 0.0f;
	m.m13 = 0.0f;

	m.m20 = 0.0f;
	m.m21 = 0.0f;
	m.m22 = (farClipPlane + nearClipPlane) / (farClipPlane - nearClipPlane);
	m.m23 = 0.0f;

	m.m30 = 0.0f;
	m.m31 = 0.0f;
	m.m32 = -(2.0f * farClipPlane * nearClipPlane) / (farClipPlane - nearClipPlane);
	m.m33 = 0.0f;
	return m;
}

inline static struct Matrix4F createVkOrthographicMatrix(
	float leftFrustum,
	float rightFrustum,
	float bottomFrustum,
	float topFrustum,
	float nearClipPlane,
	float farClipPlane)
{
	struct Matrix4F m;
	m.m00 = 2.0f / (rightFrustum - leftFrustum);
	m.m01 = 0.0f;
	m.m02 = 0.0f;
	m.m03 = 0.0f;

	m.m10 = 0.0f;
	m.m11 = -2.0f / (topFrustum - bottomFrustum);
	m.m12 = 0.0f;
	m.m13 = 0.0f;

	m.m20 = 0.0f;
	m.m21 = 0.0f;
	m.m22 = 1.0f / (farClipPlane - nearClipPlane);
	m.m23 = 0.0f;

	m.m30 = -(rightFrustum + leftFrustum) / (rightFrustum - leftFrustum);
	m.m31 = -(topFrustum + bottomFrustum) / (topFrustum - bottomFrustum);
	m.m32 = -nearClipPlane / (farClipPlane - nearClipPlane);
	m.m33 = 1.0f;
	return m;
}
inline static struct Matrix4F createGlOrthographicMatrix(
	float leftFrustum,
	float rightFrustum,
	float bottomFrustum,
	float topFrustum,
	float nearClipPlane,
	float farClipPlane)
{
	struct Matrix4F m;
	m.m00 = 2.0f / (rightFrustum - leftFrustum);
	m.m01 = 0.0f;
	m.m02 = 0.0f;
	m.m03 = 0.0f;

	m.m10 = 0.0f;
	m.m11 = 2.0f / (topFrustum - bottomFrustum);
	m.m12 = 0.0f;
	m.m13 = 0.0f;

	m.m20 = 0.0f;
	m.m21 = 0.0f;
	m.m22 = 2.0f / (farClipPlane - nearClipPlane);
	m.m23 = 0.0f;

	m.m30 = -(rightFrustum + leftFrustum) / (rightFrustum - leftFrustum);
	m.m31 = -(topFrustum + bottomFrustum) / (topFrustum - bottomFrustum);
	m.m32 = -(farClipPlane + nearClipPlane) / (farClipPlane - nearClipPlane);
	m.m33 = 1.0f;
	return m;
}
