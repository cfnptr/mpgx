#include "mpgx/free_camera.h"
#include "cmmt/angle.h"

#include <stdlib.h>
#include <assert.h>

struct FreeCamera
{
	Window* window;
	Transform* transform;
	Vector2F rotation;
	Vector2F lastCursorPosition;
};

FreeCamera* createFreeCamera(
	Window* window,
	Transform* transform)
{
	assert(window != NULL);
	assert(transform != NULL);

	FreeCamera* freeCamera = malloc(
		sizeof(FreeCamera));

	if (freeCamera == NULL)
		return NULL;

	freeCamera->window = window;
	freeCamera->transform = transform;
	freeCamera->rotation = zeroVec2F();
	freeCamera->lastCursorPosition =
		getWindowCursorPosition(window);
	return freeCamera;
}
void destroyFreeCamera(
	FreeCamera* freeCamera)
{
	if (freeCamera == NULL)
		return;

	free(freeCamera);
}

Window* getFreeCameraWindow(
	FreeCamera* freeCamera)
{
	assert(freeCamera != NULL);
	return freeCamera->window;
}
Transform* getFreeCameraTransform(
	FreeCamera* freeCamera)
{
	assert(freeCamera != NULL);
	return freeCamera->transform;
}
void setFreeCameraTransform(
	FreeCamera* freeCamera,
	Transform* transform)
{
	assert(freeCamera != NULL);
	assert(transform != NULL);
	freeCamera->transform = transform;
}

void updateFreeCamera(
	FreeCamera* freeCamera)
{
	assert(freeCamera != NULL);

	Window* window =
		freeCamera->window;

	if (getWindowMouseButton(window, RIGHT_MOUSE_BUTTON))
	{
		setWindowCursorMode(
			window,
			LOCKED_CURSOR_MODE);

		float deltaTime = (float)
			getWindowDeltaTime(window);
		Transform* transform =
			freeCamera->transform;
		Vector2F rotation =
			freeCamera->rotation;
		Vector2F lastCursorPosition =
			freeCamera->lastCursorPosition;
		Vector2F cursorPosition =
			getWindowCursorPosition(window);

		if (lastCursorPosition.x == 0 && lastCursorPosition.y == 0)
			lastCursorPosition = cursorPosition;

		rotation.x += (cursorPosition.x - lastCursorPosition.x) / 100.0f;
		rotation.y += (cursorPosition.y - lastCursorPosition.y) / 100.0f;

		if (rotation.y > degToRadF(89.9f))
			rotation.y = degToRadF(89.9f);
		else if (rotation.y < degToRadF(-89.9f))
			rotation.y = degToRadF(-89.9f);

		freeCamera->rotation = rotation;
		freeCamera->lastCursorPosition = cursorPosition;

		Quaternion transformRotation = eulerQuat(vec3F(
			rotation.y,
			rotation.x,
			0.0f));
		setTransformRotation(
			transform,
			transformRotation);

		Vector3F translation = zeroVec3F();

		if (getWindowKeyboardKey(window, A_KEYBOARD_KEY))
			translation.x = RIGHT_AXIS_VALUE * deltaTime;
		else if (getWindowKeyboardKey(window, D_KEYBOARD_KEY))
			translation.x = LEFT_AXIS_VALUE * deltaTime;
		if (getWindowKeyboardKey(window, LEFT_SHIFT_KEYBOARD_KEY))
			translation.y = TOP_AXIS_VALUE * deltaTime;
		else if (getWindowKeyboardKey(window, SPACE_KEYBOARD_KEY))
			translation.y = BOTTOM_AXIS_VALUE * deltaTime;
		if (getWindowKeyboardKey(window, S_KEYBOARD_KEY))
			translation.z = FRONT_AXIS_VALUE * deltaTime;
		else if (getWindowKeyboardKey(window, W_KEYBOARD_KEY))
			translation.z = BACK_AXIS_VALUE * deltaTime;

		translation = dotVecQuat3F(
			transformRotation,
			translation);

		Vector3F transformPosition =
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

