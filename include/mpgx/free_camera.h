#pragma once
#include "mpgx/window.h"
#include "mpgx/camera.h"
#include "mpgx/transformer.h"

typedef struct FreeCamera* FreeCamera;

FreeCamera createFreeCamera(
	Window window,
	Transformer transformer,
	float moveSpeed,
	float viewSpeed,
	float fieldOfView,
	float nearClipPlane,
	float farClipPlane);
void destroyFreeCamera(FreeCamera freeCamera);

Window getFreeCameraWindow(FreeCamera freeCamera);

Transform getFreeCameraTransform(
	FreeCamera freeCamera);
void setFreeCameraTransform(
	FreeCamera freeCamera,
	Transform transform);

Vec2F getFreeCameraRotation(
	FreeCamera freeCamera);
void setFreeCameraRotation(
	FreeCamera freeCamera,
	Vec2F rotation);

float getFreeCameraMoveSpeed(
	FreeCamera freeCamera);
void setFreeCameraMoveSpeed(
	FreeCamera freeCamera,
	float moveSpeed);

float getFreeCameraViewSpeed(
	FreeCamera freeCamera);
void setFreeCameraViewSpeed(
	FreeCamera freeCamera,
	float viewSpeed);

float getFreeCameraFieldOfView(
	FreeCamera freeCamera);
void setFreeCameraFieldOfView(
	FreeCamera freeCamera,
	float fieldOfView);

float getFreeCameraNearClipPlane(
	FreeCamera freeCamera);
void setFreeCameraNearClipPlane(
	FreeCamera freeCamera,
	float nearClipPlane);

float getFreeCameraFarClipPlane(
	FreeCamera freeCamera);
void setFreeCameraFarClipPlane(
	FreeCamera freeCamera,
	float farClipPlane);

void updateFreeCamera(FreeCamera freeCamera);
Camera getFreeCamera(FreeCamera freeCamera);
