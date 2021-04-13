#pragma once
#include "mpgx/window.h"
#include "mpgx/transformer.h"

typedef struct FreeCamera FreeCamera;

FreeCamera* createFreeCamera(
	Window* window,
	Transform* transform,
	float moveSpeed,
	float viewSpeed);
void destroyFreeCamera(FreeCamera* freeCamera);

Window* getFreeCameraWindow(FreeCamera* freeCamera);

Transform* getFreeCameraTransform(
	FreeCamera* freeCamera);
void setFreeCameraTransform(
	FreeCamera* freeCamera,
	Transform* transform);

float getFreeCameraMoveSpeed(
	FreeCamera* freeCamera);
void setFreeCameraMoveSpeed(
	FreeCamera* freeCamera,
	float moveSpeed);

float getFreeCameraViewSpeed(
	FreeCamera* freeCamera);
void setFreeCameraViewSpeed(
	FreeCamera* freeCamera,
	float viewSpeed);

void updateFreeCamera(FreeCamera* freeCamera);
