#pragma once
#include "mpgx/window.h"
#include "mpgx/camera.h"
#include "mpgx/transformer.h"

typedef struct FreeCamera FreeCamera;

FreeCamera* createFreeCamera(
	Window* window,
	Transformer* transformer,
	float moveSpeed,
	float viewSpeed,
	float fieldOfView,
	float nearClipPlane,
	float farClipPlane);
void destroyFreeCamera(FreeCamera* freeCamera);

Window* getFreeCameraWindow(FreeCamera* freeCamera);

Transform* getFreeCameraTransform(
	const FreeCamera* freeCamera);
void setFreeCameraTransform(
	FreeCamera* freeCamera,
	Transform* transform);

float getFreeCameraMoveSpeed(
	const FreeCamera* freeCamera);
void setFreeCameraMoveSpeed(
	FreeCamera* freeCamera,
	float moveSpeed);

float getFreeCameraViewSpeed(
	const FreeCamera* freeCamera);
void setFreeCameraViewSpeed(
	FreeCamera* freeCamera,
	float viewSpeed);

float getFreeCameraFieldOfView(
	const FreeCamera* freeCamera);
void setFreeCameraFieldOfView(
	FreeCamera* freeCamera,
	float fieldOfView);

float getFreeCameraNearClipPlane(
	const FreeCamera* freeCamera);
void setFreeCameraNearClipPlane(
	FreeCamera* freeCamera,
	float nearClipPlane);

float getFreeCameraFarClipPlane(
	const FreeCamera* freeCamera);
void setFreeCameraFarClipPlane(
	FreeCamera* freeCamera,
	float farClipPlane);

void updateFreeCamera(FreeCamera* freeCamera);
Camera getFreeCamera(const FreeCamera* freeCamera);
