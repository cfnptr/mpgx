// Copyright 2020-2021 Nikita Fediuchin. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "mpgx/window.h"
#include "mpgx/transformer.h"

#include "cmmt/camera.h"

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

Vec3F getFreeCameraPosition(
	FreeCamera freeCamera);
void setFreeCameraPosition(
	FreeCamera freeCamera,
	Vec3F position);

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
