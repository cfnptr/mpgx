#include "mpgx/interface.h"

#include <assert.h>
#include <stdlib.h>

struct InterfaceElement
{
	Interface* interface;
	Transform* transform;
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
	Window* window;
	float scale;
	InterfaceElement** elements;
	size_t elementCapacity;
	size_t elementCount;
	InterfaceElement* lastElement;
};

Interface* createInterface(
	Window* window,
	float scale)
{
	assert(window != NULL);

	Interface* interface = malloc(
		sizeof(Interface));

	if (interface == NULL)
		return NULL;

	InterfaceElement** elements = malloc(
		sizeof(InterfaceElement*));

	if (elements == NULL)
	{
		free(interface);
		return NULL;
	}

	interface->window = window;
	interface->scale = scale;
	interface->elements = elements;
	interface->elementCapacity = 1;
	interface->elementCount = 0;
	interface->lastElement = NULL;
	return interface;
}
void destroyInterface(Interface* interface)
{
	if (interface == NULL)
		return;

	InterfaceElement** elements = interface->elements;
	size_t elementCount = interface->elementCount;

	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement* element = elements[i];
		element->onDestroy(element->handle);
		free(element);
	}

	free(elements);
	free(interface);
}

Window* getInterfaceWindow(
	const Interface* interface)
{
	assert(interface != NULL);
	return interface->window;
}

float getInterfaceScale(
	const Interface* interface)
{
	assert(interface != NULL);
	return interface->scale;
}
void setInterfaceScale(
	Interface* interface,
	float scale)
{
	assert(interface != NULL);
	interface->scale = scale;
}

Camera createInterfaceCamera(
	const Interface* interface)
{
	assert(interface != NULL);

	Vec2U windowSize = getWindowSize(interface->window);
	float scale = interface->scale;

	Vec2F halfSize = vec2F(
		((float)windowSize.x / scale) / 2.0f,
		((float)windowSize.y / scale) / 2.0f);

	return orthographicCamera(
		-halfSize.x,
		halfSize.x,
		-halfSize.y,
		halfSize.y,
		0.0f,
		1.0f);
}

inline static Vec3F calcTransformPosition(
	Vec2F halfSize,
	Vec3F position,
	uint8_t anchor)
{
	switch (anchor)
	{
	default:
		abort();
	case CENTER_INTERFACE_ANCHOR:
		return position;
	case LEFT_INTERFACE_ANCHOR:
		return vec3F(
			position.x - halfSize.x,
			position.y,
			position.z);
	case RIGHT_INTERFACE_ANCHOR:
		return vec3F(
			position.x + halfSize.x,
			position.y,
			position.z);
	case BOTTOM_INTERFACE_ANCHOR:
		return vec3F(
			position.x,
			position.y - halfSize.y,
			position.z);
	case TOP_INTERFACE_ANCHOR:
		return vec3F(
			position.x,
			position.y + halfSize.y,
			position.z);
	case LEFT_BOTTOM_INTERFACE_ANCHOR:
		return vec3F(
			position.x - halfSize.x,
			position.y - halfSize.y,
			position.z);
	case LEFT_TOP_INTERFACE_ANCHOR:
		return vec3F(
			position.x - halfSize.x,
			position.y + halfSize.y,
			position.z);
	case RIGHT_BOTTOM_INTERFACE_ANCHOR:
		return vec3F(
			position.x + halfSize.x,
			position.y - halfSize.y,
			position.z);
	case RIGHT_TOP_INTERFACE_ANCHOR:
		return vec3F(
			position.x + halfSize.x,
			position.y + halfSize.y,
			position.z);
	}
}
void preUpdateInterface(Interface* interface)
{
	assert(interface != NULL);

	InterfaceElement** elements = interface->elements;
	size_t elementCount = interface->elementCount;

	if (elementCount == 0)
		return;

	Vec2U windowSize = getWindowSize(interface->window);
	float scale = interface->scale;

	Vec2F halfSize = vec2F(
		((float)windowSize.x / scale) / 2.0f,
		((float)windowSize.y / scale) / 2.0f);

	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement* element = elements[i];
		Transform* transform = element->transform;

		if (getTransformUpdate(transform) == false)
			continue;

		Transform* parent = getTransformParent(transform);

		while (parent != NULL)
		{
			if (getTransformUpdate(parent) == false)
				goto CONTINUE;
			parent = getTransformParent(parent);
		}

		Vec3F position = calcTransformPosition(
			halfSize,
			element->position,
			element->anchor);
		setTransformPosition(
			transform,
			position);

	CONTINUE:
		continue;
	}
}
void updateInterface(Interface* interface)
{
	assert(interface != NULL);

	InterfaceElement** elements = interface->elements;
	size_t elementCount = interface->elementCount;
	Window* window = interface->window;

	if (elementCount == 0 ||
		isWindowFocused(window) == false)
	{
		return;
	}

	float scale = interface->scale;
	Vec2U windowSize = getWindowSize(window);
	Vec2F cursor = getWindowCursorPosition(window);

	Vec2F size = vec2F(
		(float)windowSize.x / scale,
		(float)windowSize.y / scale);
	Vec2F halfSize = divValVec2F(size,2.0f);

	Vec2F cursorPosition = vec2F(
		(cursor.x / scale) - halfSize.x,
		(size.y - (cursor.y / scale)) - halfSize.y);

	InterfaceElement* newElement = NULL;

	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement* element = elements[i];
		Transform* transform = element->transform;

		if (getTransformUpdate(transform) == false)
			continue;

		Transform* parent = getTransformParent(transform);

		while (parent != NULL)
		{
			if (getTransformUpdate(parent) == false)
				goto CONTINUE;
			parent = getTransformParent(parent);
		}

		Vec3F position = getTranslationMat4F(
			getTransformModel(transform));

		Box2F bounds = element->bounds;
		bounds.minimum = addVec2F(
			bounds.minimum,
			vec2F(position.x, position.y));
		bounds.maximum = addVec2F(
			bounds.maximum,
			vec2F(position.x, position.y));

		bool colliding = isPointInBox2F(
			bounds,
			cursorPosition);

		if (colliding == false)
			continue;

		if (newElement != NULL)
		{
			if (element->position.z < newElement->position.z)
				newElement = element;
		}
		else
		{
			newElement = element;
		}

	CONTINUE:
		continue;
	}

	InterfaceElement* lastElement =
		interface->lastElement;

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

InterfaceElement* createInterfaceElement(
	Interface* interface,
	uint8_t anchor,
	Vec3F position,
	Box2F bounds,
	Transform* transform,
	OnInterfaceElementDestroy onDestroy,
	OnInterfaceElementEvent onEnter,
	OnInterfaceElementEvent onExit,
	OnInterfaceElementEvent onStay,
	void* handle)
{
	assert(interface != NULL);
	assert(anchor < INTERFACE_ANCHOR_COUNT);
	assert(onDestroy != NULL);

	InterfaceElement* element = malloc(
		sizeof(InterfaceElement));

	if (element == NULL)
		return NULL;

	element->interface = interface;
	element->transform = transform;
	element->anchor = anchor;
	element->position = position;
	element->bounds = bounds;
	element->transform = transform;
	element->onDestroy = onDestroy;
	element->onEnter = onEnter;
	element->onExit = onExit;
	element->onStay = onStay;
	element->handle = handle;

	InterfaceElement** elements = interface->elements;
	size_t elementCount = interface->elementCount;
	size_t elementCapacity = interface->elementCapacity;

	if (elementCount == elementCapacity)
	{
		elementCapacity *= 2;

		elements = realloc(
			elements,
			elementCapacity * sizeof(InterfaceElement*));

		if (elements == NULL)
		{
			destroyTransform(transform);
			free(element);
			return NULL;
		}

		interface->elements = elements;
		interface->elementCapacity = elementCapacity;
	}

	elements[elementCount] = element;
	interface->elementCount++;
	return element;
}
void destroyInterfaceElement(
	InterfaceElement* element)
{
	if (element == NULL)
		return;

	Interface* interface = element->interface;
	InterfaceElement** elements = interface->elements;
	size_t elementCount = interface->elementCount;

	for (size_t i = 0; i < elementCount; i++)
	{
		if (elements[i] != element)
			continue;

		for (size_t j = i + 1; j < elementCount; j++)
			elements[j - 1] = elements[j];

		element->onDestroy(element->handle);
		destroyTransform(element->transform);
		free(element);

		interface->elementCount--;
		return;
	}

	abort();
}

Interface* getInterfaceElementInterface(
	const InterfaceElement* element)
{
	assert(element != NULL);
	return element->interface;
}
Transform* getInterfaceElementTransform(
	const InterfaceElement* element)
{
	assert(element != NULL);
	return element->transform;
}
OnInterfaceElementDestroy getInterfaceElementOnDestroy(
	const InterfaceElement* element)
{
	assert(element != NULL);
	return element->onDestroy;
}
OnInterfaceElementEvent getInterfaceElementOnEnter(
	const InterfaceElement* element)
{
	assert(element != NULL);
	return element->onEnter;
}
OnInterfaceElementEvent getInterfaceElementOnExit(
	const InterfaceElement* element)
{
	assert(element != NULL);
	return element->onExit;
}
OnInterfaceElementEvent getInterfaceElementOnStay(
	const InterfaceElement* element)
{
	assert(element != NULL);
	return element->onStay;
}
void* getInterfaceElementHandle(
	const InterfaceElement* element)
{
	assert(element != NULL);
	return element->handle;
}

uint8_t getInterfaceElementAnchor(
	const InterfaceElement* element)
{
	assert(element != NULL);
	return element->anchor;
}
void setInterfaceElementAnchor(
	InterfaceElement* element,
	uint8_t anchor)
{
	assert(element != NULL);
	element->anchor = anchor;
}

Vec3F getInterfaceElementPosition(
	const InterfaceElement* element)
{
	assert(element != NULL);
	return element->position;
}
void setInterfaceElementPosition(
	InterfaceElement* element,
	Vec3F position)
{
	assert(element != NULL);
	element->position = position;
}

Box2F getInterfaceElementBounds(
	const InterfaceElement* element)
{
	assert(element != NULL);
	return element->bounds;
}
void setInterfaceElementBounds(
	InterfaceElement* element,
	Box2F bounds)
{
	assert(element != NULL);
	element->bounds = bounds;
}
