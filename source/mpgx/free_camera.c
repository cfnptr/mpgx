#include "mpgx/free_camera.h"
#include "cmmt/angle.h"

#include <stdlib.h>
#include <assert.h>

struct FreeCamera
{
	Window window;
	Transform transform;
	Vec2F rotation;
	Vec2F lastCursorPosition;
	float moveSpeed;
	float viewSpeed;
	float fieldOfView;
	float nearClipPlane;
	float farClipPlane;
};

FreeCamera createFreeCamera(
	Window window,
	Transformer transformer,
	float moveSpeed,
	float viewSpeed,
	float fieldOfView,
	float nearClipPlane,
	float farClipPlane)
{
	assert(window != NULL);
	assert(transformer != NULL);

	FreeCamera freeCamera = malloc(
		sizeof(struct FreeCamera));

	if (freeCamera == NULL)
		return NULL;

	Transform transform = createTransform(
		transformer,
		zeroVec3F(),
		oneVec3F(),
		oneQuat(),
		ORBIT_ROTATION_TYPE,
		NULL,
		true);

	if (transform == NULL)
	{
		free(freeCamera);
		return NULL;
	}

	freeCamera->window = window;
	freeCamera->transform = transform;
	freeCamera->rotation = zeroVec2F();
	freeCamera->lastCursorPosition =
		getWindowCursorPosition(window);
	freeCamera->moveSpeed = moveSpeed;
	freeCamera->viewSpeed = viewSpeed;
	freeCamera->fieldOfView = fieldOfView;
	freeCamera->nearClipPlane = nearClipPlane;
	freeCamera->farClipPlane = farClipPlane;
	return freeCamera;
}
void destroyFreeCamera(FreeCamera freeCamera)
{
	if (freeCamera == NULL)
		return;

	destroyTransform(freeCamera->transform);
	free(freeCamera);
}

Window getFreeCameraWindow(FreeCamera freeCamera)
{
	assert(freeCamera != NULL);
	return freeCamera->window;
}

Transform getFreeCameraTransform(
	FreeCamera freeCamera)
{
	assert(freeCamera != NULL);
	return freeCamera->transform;
}
void setFreeCameraTransform(
	FreeCamera freeCamera,
	Transform transform)
{
	assert(freeCamera != NULL);
	assert(transform != NULL);
	freeCamera->transform = transform;
}

Vec2F getFreeCameraRotation(
	FreeCamera freeCamera)
{
	assert(freeCamera != NULL);
	return freeCamera->rotation;
}
void setFreeCameraRotation(
	FreeCamera freeCamera,
	Vec2F rotation)
{
	assert(freeCamera != NULL);

	if (rotation.y > degToRadF(89.9f))
		rotation.y = degToRadF(89.9f);
	else if (rotation.y < degToRadF(-89.9f))
		rotation.y = degToRadF(-89.9f);

	freeCamera->rotation = rotation;
}

float getFreeCameraMoveSpeed(
	FreeCamera freeCamera)
{
	assert(freeCamera != NULL);
	return freeCamera->moveSpeed;
}
void setFreeCameraMoveSpeed(
	FreeCamera freeCamera,
	float moveSpeed)
{
	assert(freeCamera != NULL);
	freeCamera->moveSpeed = moveSpeed;
}

float getFreeCameraViewSpeed(
	FreeCamera freeCamera)
{
	assert(freeCamera != NULL);
	return freeCamera->viewSpeed;
}
void setFreeCameraViewSpeed(
	FreeCamera freeCamera,
	float viewSpeed)
{
	assert(freeCamera != NULL);
	freeCamera->viewSpeed = viewSpeed;
}

float getFreeCameraFieldOfView(
	FreeCamera freeCamera)
{
	assert(freeCamera != NULL);
	return freeCamera->fieldOfView;
}
void setFreeCameraFieldOfView(
	FreeCamera freeCamera,
	float fieldOfView)
{
	assert(freeCamera != NULL);
	freeCamera->fieldOfView = fieldOfView;
}

float getFreeCameraNearClipPlane(
	FreeCamera freeCamera)
{
	assert(freeCamera != NULL);
	return freeCamera->nearClipPlane;
}
void setFreeCameraNearClipPlane(
	FreeCamera freeCamera,
	float nearClipPlane)
{
	assert(freeCamera != NULL);
	freeCamera->nearClipPlane = nearClipPlane;
}

float getFreeCameraFarClipPlane(
	FreeCamera freeCamera)
{
	assert(freeCamera != NULL);
	return freeCamera->farClipPlane;
}
void setFreeCameraFarClipPlane(
	FreeCamera freeCamera,
	float farClipPlane)
{
	assert(freeCamera != NULL);
	freeCamera->farClipPlane = farClipPlane;
}

void updateFreeCamera(FreeCamera freeCamera)
{
	assert(freeCamera != NULL);
	Window window = freeCamera->window;

	if (isWindowFocused(window) == false)
		return;

	if (getWindowMouseButton(window, RIGHT_MOUSE_BUTTON) == true)
	{
		setWindowCursorMode(
			window,
			LOCKED_CURSOR_MODE);

		float deltaTime = (float)getWindowDeltaTime(window);
		Transform transform = freeCamera->transform;
		Vec2F rotation = freeCamera->rotation;
		Vec2F lastCursorPosition = freeCamera->lastCursorPosition;
		float moveSpeed = freeCamera->moveSpeed * 2.0f;
		float viewSpeed = freeCamera->viewSpeed / 200.0f;
		Vec2F cursorPosition = getWindowCursorPosition(window);

		if (lastCursorPosition.x == 0 && lastCursorPosition.y == 0)
			lastCursorPosition = cursorPosition;

		rotation.x += (cursorPosition.x - lastCursorPosition.x) * viewSpeed;
		rotation.y += (cursorPosition.y - lastCursorPosition.y) * viewSpeed;

		if (rotation.y > degToRadF(89.9f))
			rotation.y = degToRadF(89.9f);
		else if (rotation.y < degToRadF(-89.9f))
			rotation.y = degToRadF(-89.9f);

		freeCamera->rotation = rotation;
		freeCamera->lastCursorPosition = cursorPosition;

		Quat transformRotation = eulerQuat(vec3F(
			rotation.y,
			rotation.x,
			0.0f));
		setTransformRotation(
			transform,
			transformRotation);

		Vec3F translation = zeroVec3F();

		if (getWindowKeyboardKey(window, A_KEYBOARD_KEY))
			translation.x = RIGHT_AXIS_VALUE * deltaTime * moveSpeed;
		else if (getWindowKeyboardKey(window, D_KEYBOARD_KEY))
			translation.x = LEFT_AXIS_VALUE * deltaTime * moveSpeed;
		if (getWindowKeyboardKey(window, LEFT_SHIFT_KEYBOARD_KEY))
			translation.y = TOP_AXIS_VALUE * deltaTime * moveSpeed;
		else if (getWindowKeyboardKey(window, SPACE_KEYBOARD_KEY))
			translation.y = BOTTOM_AXIS_VALUE * deltaTime * moveSpeed;
		if (getWindowKeyboardKey(window, S_KEYBOARD_KEY))
			translation.z = FRONT_AXIS_VALUE * deltaTime * moveSpeed;
		else if (getWindowKeyboardKey(window, W_KEYBOARD_KEY))
			translation.z = BACK_AXIS_VALUE * deltaTime * moveSpeed;

		translation = dotVecQuat3F(
			transformRotation,
			translation);

		Vec3F transformPosition =
			getTransformPosition(transform);
		transformPosition = addVec3F(
			transformPosition,
			translation);
		setTransformPosition(
			transform,
			transformPosition);
	}
	else
	{
		setWindowCursorMode(
			window,
			DEFAULT_CURSOR_MODE);
		freeCamera->lastCursorPosition = zeroVec2F();
	}
}
Camera getFreeCamera(FreeCamera freeCamera)
{
	assert(freeCamera != NULL);

	Vec2U framebufferSize = getWindowFramebufferSize(
		freeCamera->window);

	return perspectiveCamera(
		freeCamera->fieldOfView,
		(float)framebufferSize.x / (float)framebufferSize.y,
		freeCamera->nearClipPlane,
		freeCamera->farClipPlane);
}
