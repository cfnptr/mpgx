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
#include "cmmt/quaternion.h"
#include <stdint.h>

typedef struct Transformer* Transformer;
typedef struct Transform* Transform;

typedef enum RotationType
{
	NO_ROTATION_TYPE = 0,
	SPIN_ROTATION_TYPE = 1,
	ORBIT_ROTATION_TYPE = 2,
	ROTATION_TYPE_COUNT = 3,
} RotationType;

Transformer createTransformer(size_t capacity);
void destroyTransformer(Transformer transformer);

bool isTransformerEmpty(Transformer transformer);

void enumerateTransformer(
	Transformer transformer,
	void(*onItem)(Transform));

Transform createTransform(
	Transformer transformer,
	Vec3F position,
	Vec3F scale,
	Quat rotation,
	RotationType rotationType,
	Transform parent,
	bool isActive);
void destroyTransform(Transform transform);

Transformer getTransformTransformer(
	Transform transform);

Vec3F getTransformPosition(
	Transform transform);
void setTransformPosition(
	Transform transform,
	Vec3F position);

Vec3F getTransformScale(
	Transform transform);
void setTransformScale(
	Transform transform,
	Vec3F scale);

Quat getTransformRotation(
	Transform transform);
void setTransformRotation(
	Transform transform,
	Quat rotation);

RotationType getTransformRotationType(
	Transform transform);
void setTransformRotationType(
	Transform transform,
	RotationType rotationType);

Transform getTransformParent(
	Transform transform);
void setTransformParent(
	Transform transform,
	Transform parent);

bool isTransformActive(
	Transform transform);
void setTransformActive(
	Transform transform,
	bool isActive);

Mat4F getTransformModel(
	Transform transform);

void updateTransformer(
	Transformer transformer);
