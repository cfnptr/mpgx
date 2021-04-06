#pragma once
#include "mpgx/window.h"
#include "mpgx/transformer.h"

struct FreeCamera;

struct FreeCamera* createFreeCamera(
	struct Window* window,
	struct Transform* transform);
void destroyFreeCamera(
	struct FreeCamera* freeCamera);

struct Window* getFreeCameraWindow(
	struct FreeCamera* freeCamera);


struct Transform* getFreeCameraTransform(
	struct FreeCamera* freeCamera);
void setFreeCameraTransform(
	struct FreeCamera* freeCamera,
	struct Transform* transform);

void updateFreeCamera(
	struct FreeCamera* freeCamera);
