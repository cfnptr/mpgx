// Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
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
	OnInterfaceElementDestroy onDestroy;
	InterfaceElementEvents events;
	void* handle;
	Transform transform;
	Vec3F position;
	Box2F bounds;
	AlignmentType alignment;
	bool isEnabled;
	bool isPressed;
};
struct Interface_T
{
	Window window;
	InterfaceElement* elements;
	size_t elementCapacity;
	size_t elementCount;
	InterfaceElement lastElement;
	float scale;
#ifndef NDEBUG
	bool isEnumerating;
#endif
};

void destroyInterface(Interface interface)
{
	if (!interface)
		return;

	assert(interface->elementCount == 0);
	assert(!interface->isEnumerating);

	free(interface->elements);
	free(interface);
}
Interface createInterface(
	Window window,
	float scale,
	size_t capacity)
{
	assert(window);
	assert(scale > 0.0f);
	assert(capacity > 0);

	Interface interface = calloc(1,
		sizeof(Interface_T));

	if (!interface)
		return NULL;

	interface->window = window;
	interface->scale = scale;
#ifndef NDEBUG
	interface->isEnumerating = false;
#endif

	InterfaceElement* elements = malloc(
		sizeof(InterfaceElement) * capacity);

	if (!elements)
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
	assert(interface);
	return interface->elementCount == 0;
}
Window getInterfaceWindow(Interface interface)
{
	assert(interface);
	return interface->window;
}

float getInterfaceScale(
	Interface interface)
{
	assert(interface);
	return interface->scale;
}
void setInterfaceScale(
	Interface interface,
	float scale)
{
	assert(interface);
	assert(scale > 0.0f);
	interface->scale = scale;
}

size_t getInterfaceElementCount(
	Interface interface)
{
	assert(interface);
	return interface->elementCount;
}
void enumerateInterface(
	Interface interface,
	void(*onItem)(InterfaceElement))
{
	assert(interface);
	assert(onItem);

#ifndef NDEBUG
	interface->isEnumerating = true;
#endif

	InterfaceElement* elements = interface->elements;
	size_t elementCount = interface->elementCount;

	for (size_t i = 0; i < elementCount; i++)
		onItem(elements[i]);

#ifndef NDEBUG
	interface->isEnumerating = false;
#endif
}
void destroyAllInterfaceElements(
	Interface interface,
	bool destroyTransforms)
{
	assert(interface);
	assert(!interface->isEnumerating);

	InterfaceElement* elements = interface->elements;
	size_t elementCount = interface->elementCount;

	if (elementCount == 0)
		return;

	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement element = elements[i];
		element->onDestroy(element->handle);

		if (destroyTransforms)
			destroyTransform(element->transform);

		free(element);
	}

	interface->elementCount = 0;
}

Camera createInterfaceCamera(
	Interface interface)
{
	assert(interface);

	Vec2I windowSize = getWindowSize(interface->window);
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
	assert(interface);

	size_t elementCount = interface->elementCount;

	if (elementCount == 0)
		return;

	InterfaceElement* elements = interface->elements;
	Vec2I windowSize = getWindowSize(interface->window);
	float scale = interface->scale;

	Vec2F halfSize = vec2F(
		((float)windowSize.x / scale) * 0.5f,
		((float)windowSize.y / scale) * 0.5f);

	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement element = elements[i];
		Transform transform = element->transform;

		if (!isTransformActive(transform))
			continue;

		Transform parent = getTransformParent(transform);

		while (parent)
		{
			if (!isTransformActive(parent))
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
	assert(interface);

	InterfaceElement* elements = interface->elements;
	size_t elementCount = interface->elementCount;
	Window window = interface->window;

	if (elementCount == 0 || !isWindowFocused(window))
		return;

	float interfaceScale = interface->scale;
	Vec2I windowSize = getWindowSize(window);
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

		if (!element->isEnabled || !isTransformActive(transform))
			continue;

		Transform parent = getTransformParent(transform);

		while (parent)
		{
			if (!isTransformActive(parent))
				goto CONTINUE;
			parent = getTransformParent(parent);
		}

		if (element->events.onUpdate)
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

		if (!isPointInBox2F(bounds, cursorPosition))
			continue;

		if (newElement)
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

	if (lastElement)
	{
		if (lastElement != newElement)
		{
			if (lastElement->events.onExit)
				lastElement->events.onExit(lastElement);

			lastElement->isPressed = false;

			if (newElement && newElement->events.onEnter)
			{
				newElement->events.onEnter(newElement);
			}

			interface->lastElement = newElement;
		}
		else
		{
			if (isLeftButtonPressed)
			{
				if (lastElement->isPressed)
				{
					if (lastElement->events.onStay)
						lastElement->events.onStay(lastElement);
				}
				else
				{
					if (lastElement->events.onPress)
						lastElement->events.onPress(lastElement);
					lastElement->isPressed = true;
				}
			}
			else
			{
				if (lastElement->isPressed)
				{
					if (lastElement->events.onRelease)
						lastElement->events.onRelease(lastElement);
					lastElement->isPressed = false;
				}
				else
				{
					if (lastElement->events.onStay)
						lastElement->events.onStay(lastElement);
				}
			}
		}
	}
	else
	{
		if (newElement)
		{
			if (newElement->events.onEnter)
				newElement->events.onEnter(newElement);
			interface->lastElement = newElement;
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
	assert(interface);
	assert(transform);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(onDestroy);
	assert(events);
	assert(handle);
	assert(!interface->isEnumerating);

	InterfaceElement element = malloc(
		sizeof(InterfaceElement_T));

	if (!element)
		return NULL;

	element->interface = interface;
	element->onDestroy = onDestroy;
	element->events = *events;
	element->handle = handle;
	element->transform = transform;
	element->position = position;
	element->bounds = bounds;
	element->alignment = alignment;
	element->isEnabled = isEnabled;
	element->isPressed = false;

	size_t count = interface->elementCount;

	if (count == interface->elementCapacity)
	{
		size_t capacity = interface->elementCapacity * 2;

		InterfaceElement* elements = realloc(
			interface->elements,
			sizeof(InterfaceElement) * capacity);

		if (!elements)
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
	if (!element)
		return;

	assert(!element->interface->isEnumerating);

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

		if (_destroyTransform)
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
	assert(element);
	return element->interface;
}
Transform getInterfaceElementTransform(
	InterfaceElement element)
{
	assert(element);
	return element->transform;
}
OnInterfaceElementDestroy getInterfaceElementOnDestroy(
	InterfaceElement element)
{
	assert(element);
	return element->onDestroy;
}
const InterfaceElementEvents* getInterfaceElementEvents(
	InterfaceElement element)
{
	assert(element);
	return &element->events;
}
void* getInterfaceElementHandle(
	InterfaceElement element)
{
	assert(element);
	return element->handle;
}

AlignmentType getInterfaceElementAnchor(
	InterfaceElement element)
{
	assert(element);
	return element->alignment;
}
void setInterfaceElementAnchor(
	InterfaceElement element,
	AlignmentType alignment)
{
	assert(element);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	element->alignment = alignment;
}

Vec3F getInterfaceElementPosition(
	InterfaceElement element)
{
	assert(element);
	return element->position;
}
void setInterfaceElementPosition(
	InterfaceElement element,
	Vec3F position)
{
	assert(element);
	element->position = position;
}

Box2F getInterfaceElementBounds(
	InterfaceElement element)
{
	assert(element);
	return element->bounds;
}
void setInterfaceElementBounds(
	InterfaceElement element,
	Box2F bounds)
{
	assert(element);
	element->bounds = bounds;
}

bool isInterfaceElementEnabled(
	InterfaceElement element)
{
	assert(element);
	return element->isEnabled;
}
void setInterfaceElementEnabled(
	InterfaceElement element,
	bool isEnabled)
{
	assert(element);

	if (isEnabled)
	{
		if (!element->isEnabled)
		{
			if (element->events.onEnable)
				element->events.onEnable(element);
			element->isEnabled = true;
		}
	}
	else
	{
		if (element->isEnabled)
		{
			if (element->events.onDisable)
				element->events.onDisable(element);
			element->isEnabled = false;
		}
	}
}
