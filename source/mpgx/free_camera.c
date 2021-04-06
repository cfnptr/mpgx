#include "mpgx/free_camera.h"
#include "cmmt/angle.h"

#include <stdlib.h>
#include <assert.h>

struct FreeCamera
{
	struct Window* window;
	struct Transform* transform;
	struct Vec2F rotation;
	struct Vec2F lastCursorPosition;
};

struct FreeCamera* createFreeCamera(
	struct Window* window,
	struct Transform* transform)
{
	assert(window != NULL);
	assert(transform != NULL);

	struct FreeCamera* freeCamera = malloc(
		sizeof(struct FreeCamera));

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
	struct FreeCamera* freeCamera)
{
	if (freeCamera == NULL)
		return;

	free(freeCamera);
}

struct Window* getFreeCameraWindow(
	struct FreeCamera* freeCamera)
{
	assert(freeCamera != NULL);
	return freeCamera->window;
}
struct Transform* getFreeCameraTransform(
	struct FreeCamera* freeCamera)
{
	assert(freeCamera != NULL);
	return freeCamera->transform;
}
void setFreeCameraTransform(
	struct FreeCamera* freeCamera,
	struct Transform* transform)
{
	assert(freeCamera != NULL);
	assert(transform != NULL);
	freeCamera->transform = transform;
}

void updateFreeCamera(
	struct FreeCamera* freeCamera)
{
	assert(freeCamera != NULL);

	struct Window* window =
		freeCamera->window;

	if (getWindowMouseButton(window, RIGHT_MOUSE_BUTTON))
	{
		setWindowCursorMode(
			window,
			LOCKED_CURSOR_MODE);

		float deltaTime = (float)
			getWindowDeltaTime(window);
		struct Transform* transform =
			freeCamera->transform;
		struct Vec2F rotation =
			freeCamera->rotation;
		struct Vec2F lastCursorPosition =
			freeCamera->lastCursorPosition;
		struct Vec2F cursorPosition =
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

		struct Quat transformRotation = eulerQuat(vec3F(
			rotation.y,
			rotation.x,
			0.0f));
		setTransformRotation(
			transform,
			transformRotation);

		struct Vec3F translation = zeroVec3F();

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

		struct Vec3F transformPosition =
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

