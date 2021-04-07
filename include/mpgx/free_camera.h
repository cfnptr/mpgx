#pragma once
#include "mpgx/window.h"
#include "mpgx/transformer.h"

typedef struct FreeCamera FreeCamera;

FreeCamera* createFreeCamera(
	Window* window,
	Transform* transform);
void destroyFreeCamera(
	FreeCamera* freeCamera);

Window* getFreeCameraWindow(
	FreeCamera* freeCamera);


Transform* getFreeCameraTransform(
	FreeCamera* freeCamera);
void setFreeCameraTransform(
	FreeCamera* freeCamera,
	Transform* transform);

void updateFreeCamera(
	FreeCamera* freeCamera);
