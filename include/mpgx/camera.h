#pragma once
#include "cmmt/matrix.h"
#include "cmmt/plane.h"
#include "cmmt/bounding.h"

#include <math.h>
#include <stdint.h>
#include <assert.h>

typedef enum CameraType
{
	PERSPECTIVE_CAMERA_TYPE = 0,
	ORTHOGRAPHIC_CAMERA_TYPE = 1,
	CAMERA_TYPE_COUNT = 2,
} CameraType;

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

inline static Camera perspectiveCamera(
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
inline static Camera orthographicCamera(
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

inline static Mat4F vkPerspectiveMat4F(
	float fieldOfView,
	float aspectRatio,
	float nearClipPlane,
	float farClipPlane)
{
	float tanHalfFov = tanf(fieldOfView / 2.0f);

	Mat4F matrix;
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
inline static Mat4F glPerspectiveMat4F(
	float fieldOfView,
	float aspectRatio,
	float nearClipPlane,
	float farClipPlane)
{
	float tanHalfFov = tanf(fieldOfView / 2.0f);

	Mat4F matrix;
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

inline static Mat4F vkOrthographicMat4F(
	float leftFrustum,
	float rightFrustum,
	float bottomFrustum,
	float topFrustum,
	float nearClipPlane,
	float farClipPlane)
{
	Mat4F matrix;
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
inline static Mat4F glOrthographicMat4F(
	float leftFrustum,
	float rightFrustum,
	float bottomFrustum,
	float topFrustum,
	float nearClipPlane,
	float farClipPlane)
{
	Mat4F matrix;
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

inline static Plane3F normFrustumPlane(Plane3F plane)
{
	float l = sqrtf(
		plane.normal.x * plane.normal.x +
		plane.normal.y * plane.normal.y +
		plane.normal.z * plane.normal.z);

	plane.normal.x /= l;
	plane.normal.y /= l;
	plane.normal.z /= l;
	plane.distance /= l;
	return plane;
}

inline static void glFrustumPlanes(
	Mat4F frustum,
	Plane3F* _leftPlane,
	Plane3F* _rightPlane,
	Plane3F* _bottomPlane,
	Plane3F* _topPlane,
	Plane3F* _backPlane,
	Plane3F* _frontPlane,
	bool normalize)
{
	assert(_leftPlane != NULL);
	assert(_rightPlane != NULL);
	assert(_bottomPlane != NULL);
	assert(_topPlane != NULL);
	assert(_backPlane != NULL);
	assert(_frontPlane != NULL);

	Plane3F leftPlane;
	leftPlane.normal.x = frustum.m03 + frustum.m00;
	leftPlane.normal.y = frustum.m13 + frustum.m10;
	leftPlane.normal.z = frustum.m23 + frustum.m20;
	leftPlane.distance = frustum.m33 + frustum.m30;

	Plane3F rightPlane;
	rightPlane.normal.x = frustum.m03 - frustum.m00;
	rightPlane.normal.y = frustum.m13 - frustum.m10;
	rightPlane.normal.z = frustum.m23 - frustum.m20;
	rightPlane.distance = frustum.m33 - frustum.m30;

	Plane3F bottomPlane;
	bottomPlane.normal.x = frustum.m03 + frustum.m01;
	bottomPlane.normal.y = frustum.m13 + frustum.m11;
	bottomPlane.normal.z = frustum.m23 + frustum.m21;
	bottomPlane.distance = frustum.m33 + frustum.m31;

	Plane3F topPlane;
	topPlane.normal.x = frustum.m03 - frustum.m01;
	topPlane.normal.y = frustum.m13 - frustum.m11;
	topPlane.normal.z = frustum.m23 - frustum.m21;
	topPlane.distance = frustum.m33 - frustum.m31;

	Plane3F backPlane;
	backPlane.normal.x = frustum.m03 + frustum.m02;
	backPlane.normal.y = frustum.m13 + frustum.m12;
	backPlane.normal.z = frustum.m23 + frustum.m22;
	backPlane.distance = frustum.m33 + frustum.m32;

	Plane3F frontPlane;
	frontPlane.normal.x = frustum.m03 - frustum.m02;
	frontPlane.normal.y = frustum.m13 - frustum.m12;
	frontPlane.normal.z = frustum.m23 - frustum.m22;
	frontPlane.distance = frustum.m33 - frustum.m32;

	if (normalize == true)
	{
		leftPlane = normFrustumPlane(leftPlane);
		rightPlane = normFrustumPlane(rightPlane);
		bottomPlane = normFrustumPlane(bottomPlane);
		topPlane = normFrustumPlane(topPlane);
		backPlane = normFrustumPlane(backPlane);
		frontPlane = normFrustumPlane(frontPlane);
	}

	*_leftPlane = leftPlane;
	*_rightPlane = rightPlane;
	*_bottomPlane = bottomPlane;
	*_topPlane = topPlane;
	*_backPlane = backPlane;
	*_frontPlane = frontPlane;
}
inline static void vkFrustumPlanes(
	Mat4F frustum,
	Plane3F* _leftPlane,
	Plane3F* _rightPlane,
	Plane3F* _bottomPlane,
	Plane3F* _topPlane,
	Plane3F* _backPlane,
	Plane3F* _frontPlane,
	bool normalize)
{
	assert(_leftPlane != NULL);
	assert(_rightPlane != NULL);
	assert(_bottomPlane != NULL);
	assert(_topPlane != NULL);
	assert(_backPlane != NULL);
	assert(_frontPlane != NULL);

	Plane3F leftPlane;
	leftPlane.normal.x = frustum.m03 + frustum.m00;
	leftPlane.normal.y = frustum.m13 + frustum.m10;
	leftPlane.normal.z = frustum.m23 + frustum.m20;
	leftPlane.distance = frustum.m33 + frustum.m30;

	Plane3F rightPlane;
	rightPlane.normal.x = frustum.m03 - frustum.m00;
	rightPlane.normal.y = frustum.m13 - frustum.m10;
	rightPlane.normal.z = frustum.m23 - frustum.m20;
	rightPlane.distance = frustum.m33 - frustum.m30;

	Plane3F bottomPlane;
	bottomPlane.normal.x = frustum.m03 + frustum.m01;
	bottomPlane.normal.y = frustum.m13 + frustum.m11;
	bottomPlane.normal.z = frustum.m23 + frustum.m21;
	bottomPlane.distance = frustum.m33 + frustum.m31;

	Plane3F topPlane;
	topPlane.normal.x = frustum.m03 - frustum.m01;
	topPlane.normal.y = frustum.m13 - frustum.m11;
	topPlane.normal.z = frustum.m23 - frustum.m21;
	topPlane.distance = frustum.m33 - frustum.m31;

	Plane3F backPlane;
	backPlane.normal.x = frustum.m02;
	backPlane.normal.y = frustum.m12;
	backPlane.normal.z = frustum.m22;
	backPlane.distance = frustum.m32;

	Plane3F frontPlane;
	frontPlane.normal.x = frustum.m03 - frustum.m02;
	frontPlane.normal.y = frustum.m13 - frustum.m12;
	frontPlane.normal.z = frustum.m23 - frustum.m22;
	frontPlane.distance = frustum.m33 - frustum.m32;

	if (normalize == true)
	{
		leftPlane = normFrustumPlane(leftPlane);
		rightPlane = normFrustumPlane(rightPlane);
		bottomPlane = normFrustumPlane(bottomPlane);
		topPlane = normFrustumPlane(topPlane);
		backPlane = normFrustumPlane(backPlane);
		frontPlane = normFrustumPlane(frontPlane);
	}

	*_leftPlane = leftPlane;
	*_rightPlane = rightPlane;
	*_bottomPlane = bottomPlane;
	*_topPlane = topPlane;
	*_backPlane = backPlane;
	*_frontPlane = frontPlane;
}

inline static bool isBoxInFrustumPlane(
	Plane3F plane,
	Box3F box)
{
	return
		!(distPlanePoint3F(plane, vec3F(
			box.minimum.x, box.minimum.y, box.minimum.z)) < 0.0f &&
		distPlanePoint3F(plane, vec3F(
			box.minimum.x, box.minimum.y, box.maximum.z)) < 0.0f &&
		distPlanePoint3F(plane, vec3F(
			box.minimum.x, box.maximum.y, box.minimum.z)) < 0.0f &&
		distPlanePoint3F(plane, vec3F(
			box.minimum.x, box.maximum.y, box.maximum.z)) < 0.0f &&
		distPlanePoint3F(plane, vec3F(
			box.maximum.x, box.minimum.y, box.minimum.z)) < 0.0f &&
		distPlanePoint3F(plane, vec3F(
			box.maximum.x, box.minimum.y, box.maximum.z)) < 0.0f &&
		distPlanePoint3F(plane, vec3F(
			box.maximum.x, box.maximum.y, box.minimum.z)) < 0.0f &&
		distPlanePoint3F(plane, vec3F(
			box.maximum.x, box.maximum.y, box.maximum.z)) < 0.0f);
}
inline static bool isBoxInFrustum(
	Plane3F leftPlane,
	Plane3F rightPlane,
	Plane3F bottomPlane,
	Plane3F topPlane,
	Plane3F backPlane,
	Plane3F frontPlane,
	Box3F box)
{
	return
		isBoxInFrustumPlane(leftPlane, box) &&
		isBoxInFrustumPlane(rightPlane, box) &&
		isBoxInFrustumPlane(bottomPlane, box) &&
		isBoxInFrustumPlane(topPlane, box) &&
		isBoxInFrustumPlane(backPlane, box) &&
		isBoxInFrustumPlane(frontPlane, box);
}
