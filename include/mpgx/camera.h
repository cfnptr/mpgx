#pragma once
#include "cmmt/matrix.h"
#include "cmmt/plane.h"
#include "cmmt/bounding.h"

#include <math.h>
#include <stdint.h>
#include <assert.h>

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
	Plane3F planes[6],
	bool normalize)
{
	assert(planes != NULL);

	Plane3F plane;

	plane.normal.x = frustum.m03 + frustum.m00;
	plane.normal.y = frustum.m13 + frustum.m10;
	plane.normal.z = frustum.m23 + frustum.m20;
	plane.distance = frustum.m33 + frustum.m30;
	planes[0] = plane;

	plane.normal.x = frustum.m03 - frustum.m00;
	plane.normal.y = frustum.m13 - frustum.m10;
	plane.normal.z = frustum.m23 - frustum.m20;
	plane.distance = frustum.m33 - frustum.m30;
	planes[1] = plane;

	plane.normal.x = frustum.m03 + frustum.m01;
	plane.normal.y = frustum.m13 + frustum.m11;
	plane.normal.z = frustum.m23 + frustum.m21;
	plane.distance = frustum.m33 + frustum.m31;
	planes[2] = plane;

	plane.normal.x = frustum.m03 - frustum.m01;
	plane.normal.y = frustum.m13 - frustum.m11;
	plane.normal.z = frustum.m23 - frustum.m21;
	plane.distance = frustum.m33 - frustum.m31;
	planes[3] = plane;

	plane.normal.x = frustum.m03 + frustum.m02;
	plane.normal.y = frustum.m13 + frustum.m12;
	plane.normal.z = frustum.m23 + frustum.m22;
	plane.distance = frustum.m33 + frustum.m32;
	planes[4] = plane;

	plane.normal.x = frustum.m03 - frustum.m02;
	plane.normal.y = frustum.m13 - frustum.m12;
	plane.normal.z = frustum.m23 - frustum.m22;
	plane.distance = frustum.m33 - frustum.m32;
	planes[5] = plane;

	if (normalize == true)
	{
		planes[0] = normFrustumPlane(planes[0]);
		planes[1] = normFrustumPlane(planes[1]);
		planes[2] = normFrustumPlane(planes[2]);
		planes[3] = normFrustumPlane(planes[3]);
		planes[4] = normFrustumPlane(planes[4]);
		planes[5] = normFrustumPlane(planes[5]);
	}
}
inline static void vkFrustumPlanes(
	Mat4F frustum,
	Plane3F planes[6],
	bool normalize)
{
	assert(planes != NULL);

	Plane3F plane;

	plane.normal.x = frustum.m03 + frustum.m00;
	plane.normal.y = frustum.m13 + frustum.m10;
	plane.normal.z = frustum.m23 + frustum.m20;
	plane.distance = frustum.m33 + frustum.m30;
	planes[0] = plane;

	plane.normal.x = frustum.m03 - frustum.m00;
	plane.normal.y = frustum.m13 - frustum.m10;
	plane.normal.z = frustum.m23 - frustum.m20;
	plane.distance = frustum.m33 - frustum.m30;
	planes[1] = plane;

	plane.normal.x = frustum.m03 + frustum.m01;
	plane.normal.y = frustum.m13 + frustum.m11;
	plane.normal.z = frustum.m23 + frustum.m21;
	plane.distance = frustum.m33 + frustum.m31;
	planes[2] = plane;

	plane.normal.x = frustum.m03 - frustum.m01;
	plane.normal.y = frustum.m13 - frustum.m11;
	plane.normal.z = frustum.m23 - frustum.m21;
	plane.distance = frustum.m33 - frustum.m31;
	planes[3] = plane;

	plane.normal.x = frustum.m02;
	plane.normal.y = frustum.m12;
	plane.normal.z = frustum.m22;
	plane.distance = frustum.m32;
	planes[4] = plane;

	plane.normal.x = frustum.m03 - frustum.m02;
	plane.normal.y = frustum.m13 - frustum.m12;
	plane.normal.z = frustum.m23 - frustum.m22;
	plane.distance = frustum.m33 - frustum.m32;
	planes[5] = plane;

	if (normalize == true)
	{
		planes[0] = normFrustumPlane(planes[0]);
		planes[1] = normFrustumPlane(planes[1]);
		planes[2] = normFrustumPlane(planes[2]);
		planes[3] = normFrustumPlane(planes[3]);
		planes[4] = normFrustumPlane(planes[4]);
		planes[5] = normFrustumPlane(planes[5]);
	}
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
	const Plane3F planes[6],
	Box3F box)
{
	assert(planes != NULL);

	return
		isBoxInFrustumPlane(planes[0], box) &&
		isBoxInFrustumPlane(planes[1], box) &&
		isBoxInFrustumPlane(planes[2], box) &&
		isBoxInFrustumPlane(planes[3], box) &&
		isBoxInFrustumPlane(planes[4], box) &&
		isBoxInFrustumPlane(planes[5], box);
}
