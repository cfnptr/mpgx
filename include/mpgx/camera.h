#pragma once
#include "cmmt/matrix.h"

#include <math.h>
#include <stdint.h>

typedef enum CAMERA_TYPE
{
	PERSPECTIVE_CAMERA_TYPE,
	ORTHOGRAPHIC_CAMERA_TYPE,
	CAMERA_TYPE_COUNT,
} CAMERA_TYPE;

typedef struct PerspectiveCamera
{
	uint8_t type;
	float fieldOfView;
	float aspectRatio;
	float nearClipPlane;
	float farClipPlane;
} PerspectiveCamera;
typedef struct OrthographicCamera
{
	uint8_t type;
	float leftFrustum;
	float rightFrustum;
	float bottomFrustum;
	float topFrustum;
	float nearClipPlane;
	float farClipPlane;
} OrthographicCamera;

typedef union Camera
{
	PerspectiveCamera perspective;
	OrthographicCamera orthographic;
} Camera;

inline static Camera createPerspectiveCamera(
	float fieldOfView,
	float aspectRatio,
	float nearClipPlane,
	float farClipPlane)
{
	Camera camera;
	camera.perspective.type = PERSPECTIVE_CAMERA_TYPE;
	camera.perspective.fieldOfView = fieldOfView;
	camera.perspective.aspectRatio = aspectRatio;
	camera.perspective.nearClipPlane = nearClipPlane;
	camera.perspective.farClipPlane = farClipPlane;
	return camera;
}
inline static Camera createOrthographicCamera(
	float leftFrustum,
	float rightFrustum,
	float bottomFrustum,
	float topFrustum,
	float nearClipPlane,
	float farClipPlane)
{
	Camera camera;
	camera.orthographic.type = ORTHOGRAPHIC_CAMERA_TYPE;
	camera.orthographic.leftFrustum = leftFrustum;
	camera.orthographic.rightFrustum = rightFrustum;
	camera.orthographic.bottomFrustum = bottomFrustum;
	camera.orthographic.topFrustum = topFrustum;
	camera.orthographic.nearClipPlane = nearClipPlane;
	camera.orthographic.farClipPlane = farClipPlane;
	return camera;
}

inline static Matrix4F createVkPerspectiveMat4F(
	float fieldOfView,
	float aspectRatio,
	float nearClipPlane,
	float farClipPlane)
{
	float tanHalfFov = tanf(fieldOfView / 2.0f);

	Matrix4F matrix;
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
	matrix.m23 = 1.0f;

	matrix.m30 = 0.0f;
	matrix.m31 = 0.0f;
	matrix.m32 = -(farClipPlane * nearClipPlane) / (farClipPlane - nearClipPlane);
	matrix.m33 = 0.0f;
	return matrix;
}
inline static Matrix4F createGlPerspectiveMat4F(
	float fieldOfView,
	float aspectRatio,
	float nearClipPlane,
	float farClipPlane)
{
	float tanHalfFov = tanf(fieldOfView / 2.0f);

	Matrix4F matrix;
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
	matrix.m23 = 1.0f;

	matrix.m30 = 0.0f;
	matrix.m31 = 0.0f;
	matrix.m32 = -(2.0f * farClipPlane * nearClipPlane) / (farClipPlane - nearClipPlane);
	matrix.m33 = 0.0f;
	return matrix;
}

inline static Matrix4F createVkOrthographicMat4F(
	float leftFrustum,
	float rightFrustum,
	float bottomFrustum,
	float topFrustum,
	float nearClipPlane,
	float farClipPlane)
{
	Matrix4F matrix;
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
inline static Matrix4F createGlOrthographicMat4F(
	float leftFrustum,
	float rightFrustum,
	float bottomFrustum,
	float topFrustum,
	float nearClipPlane,
	float farClipPlane)
{
	Matrix4F matrix;
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
