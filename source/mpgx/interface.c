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

#include "mpgx/interface.h"

#include <assert.h>
#include <stdlib.h>

struct InterfaceElement_T
{
	Interface interface;
	Transform transform;
	AlignmentType alignment;
	Vec3F position;
	Box2F bounds;
	bool isEnabled;
	OnInterfaceElementDestroy onDestroy;
	InterfaceElementEvents events;
	void* handle;
	bool isPressed;
};
struct Interface_T
{
	Window window;
	float scale;
	InterfaceElement* elements;
	size_t elementCapacity;
	size_t elementCount;
	InterfaceElement lastElement;
};

void destroyInterface(Interface interface)
{
	if (interface == NULL)
		return;

	assert(interface->elementCount == 0);

	free(interface->elements);
	free(interface);
}
Interface createInterface(
	Window window,
	float scale,
	size_t capacity)
{
	assert(window != NULL);
	assert(capacity != 0);

	Interface interface = calloc(1,
		sizeof(Interface_T));

	if (interface == NULL)
		return NULL;

	interface->window = window;
	interface->scale = scale;

	InterfaceElement* elements = malloc(
		sizeof(InterfaceElement) * capacity);

	if (elements == NULL)
	{
		destroyInterface(interface);
		return NULL;
	}

	interface->elements = elements;
	interface->elementCapacity = capacity;
	interface->elementCount = 0;
	interface->lastElement = NULL;
	return interface;
}

bool isInterfaceEmpty(Interface interface)
{
	assert(interface != NULL);
	return interface->elementCount == 0;
}
Window getInterfaceWindow(Interface interface)
{
	assert(interface != NULL);
	return interface->window;
}

float getInterfaceScale(
	Interface interface)
{
	assert(interface != NULL);
	return interface->scale;
}
void setInterfaceScale(
	Interface interface,
	float scale)
{
	assert(interface != NULL);
	interface->scale = scale;
}

Camera createInterfaceCamera(
	Interface interface)
{
	assert(interface != NULL);

	Vec2U windowSize = getWindowSize(interface->window);
	float scale = interface->scale;

	Vec2F halfSize = vec2F(
		((float)windowSize.x / scale) * 0.5f,
		((float)windowSize.y / scale) * 0.5f);

	return orthoCamera(
		-halfSize.x,
		halfSize.x,
		-halfSize.y,
		halfSize.y,
		0.0f,
		1.0f);
}

void preUpdateInterface(Interface interface)
{
	assert(interface != NULL);

	size_t elementCount = interface->elementCount;

	if (elementCount == 0)
		return;

	InterfaceElement* elements = interface->elements;
	Vec2U windowSize = getWindowSize(interface->window);
	float scale = interface->scale;

	Vec2F halfSize = vec2F(
		((float)windowSize.x / scale) * 0.5f,
		((float)windowSize.y / scale) * 0.5f);

	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement element = elements[i];
		Transform transform = element->transform;

		if (isTransformActive(transform) == false)
			continue;

		Transform parent = getTransformParent(transform);

		while (parent != NULL)
		{
			if (isTransformActive(parent) == false)
				goto CONTINUE;
			parent = getTransformParent(parent);
		}

		AlignmentType alignment = element->alignment;
		Vec3F position = element->position;

		switch (alignment)
		{
		default:
			abort();
		case CENTER_ALIGNMENT_TYPE:
			break;
		case LEFT_ALIGNMENT_TYPE:
			position = vec3F(
				position.x - halfSize.x,
				position.y,
				position.z);
			break;
		case RIGHT_ALIGNMENT_TYPE:
			position = vec3F(
				position.x + halfSize.x,
				position.y,
				position.z);
			break;
		case BOTTOM_ALIGNMENT_TYPE:
			position = vec3F(
				position.x,
				position.y - halfSize.y,
				position.z);
			break;
		case TOP_ALIGNMENT_TYPE:
			position = vec3F(
				position.x,
				position.y + halfSize.y,
				position.z);
			break;
		case LEFT_BOTTOM_ALIGNMENT_TYPE:
			position = vec3F(
				position.x - halfSize.x,
				position.y - halfSize.y,
				position.z);
			break;
		case LEFT_TOP_ALIGNMENT_TYPE:
			position = vec3F(
				position.x - halfSize.x,
				position.y + halfSize.y,
				position.z);
			break;
		case RIGHT_BOTTOM_ALIGNMENT_TYPE:
			position = vec3F(
				position.x + halfSize.x,
				position.y - halfSize.y,
				position.z);
			break;
		case RIGHT_TOP_ALIGNMENT_TYPE:
			position = vec3F(
				position.x + halfSize.x,
				position.y + halfSize.y,
				position.z);
			break;
		}

		setTransformPosition(
			transform,
			position);

	CONTINUE:
		continue;
	}
}
void updateInterface(Interface interface)
{
	assert(interface != NULL);

	InterfaceElement* elements = interface->elements;
	size_t elementCount = interface->elementCount;
	Window window = interface->window;

	if (elementCount == 0 ||
		isWindowFocused(window) == false)
	{
		return;
	}

	float interfaceScale = interface->scale;
	Vec2U windowSize = getWindowSize(window);
	Vec2F cursor = getWindowCursorPosition(window);

	Vec2F size = vec2F(
		(float)windowSize.x / interfaceScale,
		(float)windowSize.y / interfaceScale);
	Vec2F halfSize = divValVec2F(size, 2.0f);

	Vec2F cursorPosition = vec2F(
		(cursor.x / interfaceScale) - halfSize.x,
		(size.y - (cursor.y / interfaceScale)) - halfSize.y);

	bool isLeftButtonPressed = getWindowMouseButton(
		window, LEFT_MOUSE_BUTTON);

	InterfaceElement newElement = NULL;
	float elementDistance = INFINITY;

	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement element = elements[i];
		Transform transform = element->transform;

		if (element->isEnabled == false ||
			isTransformActive(transform) == false)
		{
			continue;
		}

		Transform parent = getTransformParent(transform);

		while (parent != NULL)
		{
			if (isTransformActive(parent) == false)
				goto CONTINUE;
			parent = getTransformParent(parent);
		}

		if (element->events.onUpdate != NULL)
			element->events.onUpdate(element);

		Vec3F position = getTranslationMat4F(
			getTransformModel(transform));
		Vec3F scale = getTransformScale(transform);

		Box2F bounds = element->bounds;

		bounds.minimum = vec2F(
			bounds.minimum.x * scale.x,
			bounds.minimum.y * scale.y);
		bounds.minimum = vec2F(
			bounds.minimum.x + position.x,
			bounds.minimum.y + position.y);
		bounds.maximum = vec2F(
			bounds.maximum.x * scale.x,
			bounds.maximum.y * scale.y);
		bounds.maximum = vec2F(
			bounds.maximum.x + position.x,
			bounds.maximum.y + position.y);

		bool colliding = isPointInBox2F(
			bounds,
			cursorPosition);

		if (colliding == false)
			continue;

		if (newElement != NULL)
		{
			if (position.z < elementDistance)
			{
				newElement = element;
				elementDistance = position.z;
			}
		}
		else
		{
			newElement = element;
		}

	CONTINUE:
		continue;
	}

	InterfaceElement lastElement = interface->lastElement;

	if (lastElement == NULL)
	{
		if (newElement != NULL)
		{
			if (newElement->events.onEnter != NULL)
				newElement->events.onEnter(newElement);
			interface->lastElement = newElement;
		}
	}
	else
	{
		if (lastElement != newElement)
		{
			if (lastElement->events.onExit != NULL)
				lastElement->events.onExit(lastElement);

			lastElement->isPressed = false;

			if (newElement != NULL &&
				newElement->events.onEnter != NULL)
			{
				newElement->events.onEnter(newElement);
			}

			interface->lastElement = newElement;
		}
		else
		{
			if (isLeftButtonPressed == true)
			{
				if (lastElement->isPressed == false)
				{
					if (lastElement->events.onPress != NULL)
						lastElement->events.onPress(lastElement);
					lastElement->isPressed = true;
				}
				else
				{
					if (lastElement->events.onStay != NULL)
						lastElement->events.onStay(lastElement);
				}
			}
			else
			{
				if (lastElement->isPressed == true)
				{
					if (lastElement->events.onRelease != NULL)
						lastElement->events.onRelease(lastElement);
					lastElement->isPressed = false;
				}
				else
				{
					if (lastElement->events.onStay != NULL)
						lastElement->events.onStay(lastElement);
				}
			}
		}
	}
}

InterfaceElement createInterfaceElement(
	Interface interface,
	Transform transform,
	AlignmentType alignment,
	Vec3F position,
	Box2F bounds,
	bool isEnabled,
	OnInterfaceElementDestroy onDestroy,
	const InterfaceElementEvents* events,
	void* handle)
{
	assert(interface != NULL);
	assert(transform != NULL);
	assert(alignment >= CENTER_ALIGNMENT_TYPE);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(onDestroy != NULL);
	assert(events != NULL);
	assert(handle != NULL);

	InterfaceElement element = malloc(
		sizeof(InterfaceElement_T));

	if (element == NULL)
		return NULL;

	element->interface = interface;
	element->transform = transform;
	element->alignment = alignment;
	element->position = position;
	element->bounds = bounds;
	element->isEnabled = isEnabled;
	element->onDestroy = onDestroy;
	element->events = *events;
	element->handle = handle;
	element->isPressed = false;

	size_t count = interface->elementCount;

	if (count == interface->elementCapacity)
	{
		size_t capacity = interface->elementCapacity * 2;

		InterfaceElement* elements = realloc(
			interface->elements,
			sizeof(InterfaceElement) * capacity);

		if (elements == NULL)
		{
			free(element);
			return NULL;
		}

		interface->elements = elements;
		interface->elementCapacity = capacity;
	}

	interface->elements[count] = element;
	interface->elementCount = count + 1;
	return element;
}
void destroyInterfaceElement(
	InterfaceElement element,
	bool _destroyTransform)
{
	if (element == NULL)
		return;

	Interface interface = element->interface;
	InterfaceElement* elements = interface->elements;
	size_t elementCount = interface->elementCount;

	for (size_t i = 0; i < elementCount; i++)
	{
		if (elements[i] != element)
			continue;

		for (size_t j = i + 1; j < elementCount; j++)
			elements[j - 1] = elements[j];

		element->onDestroy(element->handle);

		if (_destroyTransform == true)
			destroyTransform(element->transform);

		free(element);
		interface->elementCount--;
		return;
	}

	abort();
}

Interface getInterfaceElementInterface(
	InterfaceElement element)
{
	assert(element != NULL);
	return element->interface;
}
Transform getInterfaceElementTransform(
	InterfaceElement element)
{
	assert(element != NULL);
	return element->transform;
}
OnInterfaceElementDestroy getInterfaceElementOnDestroy(
	InterfaceElement element)
{
	assert(element != NULL);
	return element->onDestroy;
}
const InterfaceElementEvents* getInterfaceElementEvents(
	InterfaceElement element)
{
	assert(element != NULL);
	return &element->events;
}
void* getInterfaceElementHandle(
	InterfaceElement element)
{
	assert(element != NULL);
	return element->handle;
}

AlignmentType getInterfaceElementAnchor(
	InterfaceElement element)
{
	assert(element != NULL);
	return element->alignment;
}
void setInterfaceElementAnchor(
	InterfaceElement element,
	AlignmentType alignment)
{
	assert(element != NULL);
	assert(alignment >= CENTER_ALIGNMENT_TYPE);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	element->alignment = alignment;
}

Vec3F getInterfaceElementPosition(
	InterfaceElement element)
{
	assert(element != NULL);
	return element->position;
}
void setInterfaceElementPosition(
	InterfaceElement element,
	Vec3F position)
{
	assert(element != NULL);
	element->position = position;
}

Box2F getInterfaceElementBounds(
	InterfaceElement element)
{
	assert(element != NULL);
	return element->bounds;
}
void setInterfaceElementBounds(
	InterfaceElement element,
	Box2F bounds)
{
	assert(element != NULL);
	element->bounds = bounds;
}

bool isInterfaceElementEnabled(
	InterfaceElement element)
{
	assert(element != NULL);
	return element->isEnabled;
}
void setInterfaceElementEnabled(
	InterfaceElement element,
	bool isEnabled)
{
	assert(element != NULL);

	if (isEnabled == true)
	{
		if (element->isEnabled == false)
		{
			if (element->events.onEnable != NULL)
				element->events.onEnable(element);
			element->isEnabled = true;
		}
	}
	else
	{
		if (element->isEnabled == true)
		{
			if (element->events.onDisable != NULL)
				element->events.onDisable(element);
			element->isEnabled = false;
		}
	}
}
