#pragma once
#include "cmmt/matrix.h"

#include <math.h>
#include <stdint.h>

enum CAMERA_TYPE
{
	PERSPECTIVE_CAMERA_TYPE = 0,
	ORTHOGRAPHIC_CAMERA_TYPE = 1,
};

struct PerspectiveCamera
{
	uint8_t type;
	float fieldOfView;
	float aspectRatio;
	float nearClipPlane;
	float farClipPlane;
};
struct OrthographicCamera
{
	uint8_t type;
	float leftFrustum;
	float rightFrustum;
	float bottomFrustum;
	float topFrustum;
	float nearClipPlane;
	float farClipPlane;
};

union Camera
{
	struct PerspectiveCamera perspective;
	struct OrthographicCamera orthographic;
};

inline static union Camera createPerspectiveCamera(
	float fieldOfView,
	float aspectRatio,
	float nearClipPlane,
	float farClipPlane)
{
	struct PerspectiveCamera _camera;
	_camera.type = PERSPECTIVE_CAMERA_TYPE;
	_camera.fieldOfView = fieldOfView;
	_camera.aspectRatio = aspectRatio;
	_camera.nearClipPlane = nearClipPlane;
	_camera.farClipPlane = farClipPlane;

	union Camera camera;
	camera.perspective = _camera;
	return camera;
}
inline static union Camera createOrthographicCamera(
	float leftFrustum,
	float rightFrustum,
	float bottomFrustum,
	float topFrustum,
	float nearClipPlane,
	float farClipPlane)
{
	struct OrthographicCamera _camera;
	_camera.type = ORTHOGRAPHIC_CAMERA_TYPE;
	_camera.leftFrustum = leftFrustum;
	_camera.rightFrustum = rightFrustum;
	_camera.bottomFrustum = bottomFrustum;
	_camera.topFrustum = topFrustum;
	_camera.nearClipPlane = nearClipPlane;
	_camera.farClipPlane = farClipPlane;

	union Camera camera;
	camera.orthographic = _camera;
	return camera;
}

inline static struct Mat4F createVkPerspectiveMat4F(
	float fieldOfView,
	float aspectRatio,
	float nearClipPlane,
	float farClipPlane)
{
	float tanHalfFov = tanf(fieldOfView / 2.0f);

	struct Mat4F matrix;
	matrix.m00 = 1.0f / (aspectRatio * tanHalfFov);
	matrix.m01 = 0.0f;
	matrix.m02 = 0.0f;
	matrix.m03 = 0.0f;

	matrix.m10 = 0.0f;
	matrix.m11 = -1.0f / tanHalfFov;
	matrix.m12 = 0.0f;
	matrix.m13 = 0.0f;

	matrix.m20 = 0.0f;
	matrix.m21 = 0.0f;
	matrix.m22 = farClipPlane / (farClipPlane - nearClipPlane);
	matrix.m23 = 0.0f;

	matrix.m30 = 0.0f;
	matrix.m31 = 0.0f;
	matrix.m32 = -(farClipPlane * nearClipPlane) / (farClipPlane - nearClipPlane);
	matrix.m33 = 0.0f;
	return matrix;
}
inline static struct Mat4F createGlPerspectiveMat4F(
	float fieldOfView,
	float aspectRatio,
	float nearClipPlane,
	float farClipPlane)
{
	float tanHalfFov = tanf(fieldOfView / 2.0f);

	struct Mat4F matrix;
	matrix.m00 = 1.0f / (aspectRatio * tanHalfFov);
	matrix.m01 = 0.0f;
	matrix.m02 = 0.0f;
	matrix.m03 = 0.0f;

	matrix.m10 = 0.0f;
	matrix.m11 = 1.0f / tanHalfFov;
	matrix.m12 = 0.0f;
	matrix.m13 = 0.0f;

	matrix.m20 = 0.0f;
	matrix.m21 = 0.0f;
	matrix.m22 = (farClipPlane + nearClipPlane) / (farClipPlane - nearClipPlane);
	matrix.m23 = 0.0f;

	matrix.m30 = 0.0f;
	matrix.m31 = 0.0f;
	matrix.m32 = -(2.0f * farClipPlane * nearClipPlane) / (farClipPlane - nearClipPlane);
	matrix.m33 = 0.0f;
	return matrix;
}

inline static struct Mat4F createVkOrthographicMat4F(
	float leftFrustum,
	float rightFrustum,
	float bottomFrustum,
	float topFrustum,
	float nearClipPlane,
	float farClipPlane)
{
	struct Mat4F matrix;
	matrix.m00 = 2.0f / (rightFrustum - leftFrustum);
	matrix.m01 = 0.0f;
	matrix.m02 = 0.0f;
	matrix.m03 = 0.0f;

	matrix.m10 = 0.0f;
	matrix.m11 = -2.0f / (topFrustum - bottomFrustum);
	matrix.m12 = 0.0f;
	matrix.m13 = 0.0f;

	matrix.m20 = 0.0f;
	matrix.m21 = 0.0f;
	matrix.m22 = 1.0f / (farClipPlane - nearClipPlane);
	matrix.m23 = 0.0f;

	matrix.m30 = -(rightFrustum + leftFrustum) / (rightFrustum - leftFrustum);
	matrix.m31 = -(topFrustum + bottomFrustum) / (topFrustum - bottomFrustum);
	matrix.m32 = -nearClipPlane / (farClipPlane - nearClipPlane);
	matrix.m33 = 1.0f;
	return matrix;
}
inline static struct Mat4F createGlOrthographicMat4F(
	float leftFrustum,
	float rightFrustum,
	float bottomFrustum,
	float topFrustum,
	float nearClipPlane,
	float farClipPlane)
{
	struct Mat4F matrix;
	matrix.m00 = 2.0f / (rightFrustum - leftFrustum);
	matrix.m01 = 0.0f;
	matrix.m02 = 0.0f;
	matrix.m03 = 0.0f;

	matrix.m10 = 0.0f;
	matrix.m11 = 2.0f / (topFrustum - bottomFrustum);
	matrix.m12 = 0.0f;
	matrix.m13 = 0.0f;

	matrix.m20 = 0.0f;
	matrix.m21 = 0.0f;
	matrix.m22 = 2.0f / (farClipPlane - nearClipPlane);
	matrix.m23 = 0.0f;

	matrix.m30 = -(rightFrustum + leftFrustum) / (rightFrustum - leftFrustum);
	matrix.m31 = -(topFrustum + bottomFrustum) / (topFrustum - bottomFrustum);
	matrix.m32 = -(farClipPlane + nearClipPlane) / (farClipPlane - nearClipPlane);
	matrix.m33 = 1.0f;
	return matrix;
}
