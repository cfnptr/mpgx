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
#include "mpgx/text.h"
#include "mpgx/transformer.h"

#include "cmmt/camera.h"
#include "cmmt/bounding.h"

// TODO: add element stretching type
// to do automatic message window stretching

typedef struct Interface* Interface;
typedef struct InterfaceElement* InterfaceElement;

typedef void(*OnInterfaceElementDestroy)(
	void* handle);
typedef void(*OnInterfaceElementEvent)(
	InterfaceElement element);

typedef struct InterfaceElementEvents
{
	OnInterfaceElementEvent onUpdate;
	OnInterfaceElementEvent onEnable;
	OnInterfaceElementEvent onDisable;
	OnInterfaceElementEvent onEnter;
	OnInterfaceElementEvent onExit;
	OnInterfaceElementEvent onStay;
	OnInterfaceElementEvent onPress;
	OnInterfaceElementEvent onRelease;
} InterfaceElementEvents;

Interface createInterface(
	Window window,
	float scale,
	size_t capacity);
void destroyInterface(Interface interface);

bool isInterfaceEmpty(Interface interface);
Window getInterfaceWindow(Interface interface);

float getInterfaceScale(
	Interface interface);
void setInterfaceScale(
	Interface interface,
	float scale);

Camera createInterfaceCamera(
	Interface interface);

void preUpdateInterface(Interface interface);
void updateInterface(Interface interface);

InterfaceElement createInterfaceElement(
	Interface interface,
	Transform transform,
	AlignmentType alignment,
	Vec3F position,
	Box2F bounds,
	bool isEnabled,
	OnInterfaceElementDestroy onDestroy,
	const InterfaceElementEvents* events,
	void* handle);
void destroyInterfaceElement(
	InterfaceElement element,
	bool destroyTransform);

Interface getInterfaceElementInterface(
	InterfaceElement element);
Transform getInterfaceElementTransform(
	InterfaceElement element);
OnInterfaceElementDestroy getInterfaceElementOnDestroy(
	InterfaceElement element);
const InterfaceElementEvents* getInterfaceElementEvents(
	InterfaceElement element);
void* getInterfaceElementHandle(
	InterfaceElement element);

AlignmentType getInterfaceElementAlignment(
	InterfaceElement element);
void setInterfaceElementAlignment(
	InterfaceElement element,
	AlignmentType alignment);

Vec3F getInterfaceElementPosition(
	InterfaceElement element);
void setInterfaceElementPosition(
	InterfaceElement element,
	Vec3F position);

Box2F getInterfaceElementBounds(
	InterfaceElement element);
void setInterfaceElementBounds(
	InterfaceElement element,
	Box2F bounds);

bool isInterfaceElementEnabled(
	InterfaceElement element);
void setInterfaceElementEnabled(
	InterfaceElement element,
	bool isEnabled);
