#include "mpgx/interface.h"

#include <assert.h>
#include <stdlib.h>

struct InterfaceElement
{
	Interface interface;
	Transform transform;
	uint8_t anchor;
	Vec3F position;
	Box2F bounds;
	OnInterfaceElementDestroy onDestroy;
	OnInterfaceElementEvent onEnter;
	OnInterfaceElementEvent onExit;
	OnInterfaceElementEvent onStay;
	void* handle;
};
struct Interface
{
	Window window;
	float scale;
	InterfaceElement* elements;
	size_t elementCapacity;
	size_t elementCount;
	InterfaceElement lastElement;
};

Interface createInterface(
	Window window,
	float scale,
	size_t capacity)
{
	assert(window != NULL);
	assert(capacity != 0);

	Interface interface = malloc(
		sizeof(struct Interface));

	if (interface == NULL)
		return NULL;

	InterfaceElement* elements = malloc(
		sizeof(InterfaceElement) * capacity);

	if (elements == NULL)
	{
		free(interface);
		return NULL;
	}

	interface->window = window;
	interface->scale = scale;
	interface->elements = elements;
	interface->elementCapacity = capacity;
	interface->elementCount = 0;
	interface->lastElement = NULL;
	return interface;
}
void destroyInterface(Interface interface)
{
	if (interface == NULL)
		return;

	InterfaceElement* elements = interface->elements;
	size_t elementCount = interface->elementCount;

	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement element = elements[i];

		if (element->onDestroy != NULL)
			element->onDestroy(element->handle);

		free(element);
	}

	free(elements);
	free(interface);
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
		((float)windowSize.x / scale) / 2.0f,
		((float)windowSize.y / scale) / 2.0f);

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
		((float)windowSize.x / scale) / 2.0f,
		((float)windowSize.y / scale) / 2.0f);

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

		Vec3F position = element->position;

		switch (element->anchor)
		{
		default:
			abort();
		case CENTER_INTERFACE_ANCHOR:
			break;
		case LEFT_INTERFACE_ANCHOR:
			position = vec3F(
				position.x - halfSize.x,
				position.y,
				position.z);
			break;
		case RIGHT_INTERFACE_ANCHOR:
			position = vec3F(
				position.x + halfSize.x,
				position.y,
				position.z);
			break;
		case BOTTOM_INTERFACE_ANCHOR:
			position = vec3F(
				position.x,
				position.y - halfSize.y,
				position.z);
			break;
		case TOP_INTERFACE_ANCHOR:
			position = vec3F(
				position.x,
				position.y + halfSize.y,
				position.z);
			break;
		case LEFT_BOTTOM_INTERFACE_ANCHOR:
			position = vec3F(
				position.x - halfSize.x,
				position.y - halfSize.y,
				position.z);
			break;
		case LEFT_TOP_INTERFACE_ANCHOR:
			position = vec3F(
				position.x - halfSize.x,
				position.y + halfSize.y,
				position.z);
			break;
		case RIGHT_BOTTOM_INTERFACE_ANCHOR:
			position = vec3F(
				position.x + halfSize.x,
				position.y - halfSize.y,
				position.z);
			break;
		case RIGHT_TOP_INTERFACE_ANCHOR:
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

	InterfaceElement newElement = NULL;
	float elementDistance = INFINITY;

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

		Mat4F model = getTransformModel(transform);
		Vec3F position = getTranslationMat4F(model);
		Vec3F scale = getScaleMat4F(model);

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
			if (newElement->onEnter != NULL)
				newElement->onEnter(newElement);
			interface->lastElement = newElement;
		}
	}
	else
	{
		if (lastElement != newElement)
		{
			if (lastElement->onExit != NULL)
				lastElement->onExit(lastElement);

			if (newElement != NULL &&
				newElement->onEnter != NULL)
			{
				newElement->onEnter(newElement);
			}

			interface->lastElement = newElement;
		}
		else
		{
			if (lastElement->onStay != NULL)
				lastElement->onStay(lastElement);
		}
	}
}

InterfaceElement createInterfaceElement(
	Interface interface,
	uint8_t anchor,
	Vec3F position,
	Box2F bounds,
	Transform transform,
	OnInterfaceElementDestroy onDestroy,
	OnInterfaceElementEvent onEnter,
	OnInterfaceElementEvent onExit,
	OnInterfaceElementEvent onStay,
	void* handle)
{
	assert(interface != NULL);
	assert(anchor < INTERFACE_ANCHOR_COUNT);
	assert(transform != NULL);

	InterfaceElement element = malloc(
		sizeof(struct InterfaceElement));

	if (element == NULL)
		return NULL;

	element->interface = interface;
	element->transform = transform;
	element->anchor = anchor;
	element->position = position;
	element->bounds = bounds;
	element->onDestroy = onDestroy;
	element->onEnter = onEnter;
	element->onExit = onExit;
	element->onStay = onStay;
	element->handle = handle;

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
	InterfaceElement element)
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

		if (element->onDestroy != NULL)
			element->onDestroy(element->handle);

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
OnInterfaceElementEvent getInterfaceElementOnEnter(
	InterfaceElement element)
{
	assert(element != NULL);
	return element->onEnter;
}
OnInterfaceElementEvent getInterfaceElementOnExit(
	InterfaceElement element)
{
	assert(element != NULL);
	return element->onExit;
}
OnInterfaceElementEvent getInterfaceElementOnStay(
	InterfaceElement element)
{
	assert(element != NULL);
	return element->onStay;
}
void* getInterfaceElementHandle(
	InterfaceElement element)
{
	assert(element != NULL);
	return element->handle;
}

uint8_t getInterfaceElementAnchor(
	InterfaceElement element)
{
	assert(element != NULL);
	return element->anchor;
}
void setInterfaceElementAnchor(
	InterfaceElement element,
	uint8_t anchor)
{
	assert(element != NULL);
	element->anchor = anchor;
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
