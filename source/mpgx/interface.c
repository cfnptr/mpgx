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
	InterfaceElement* parent;
	OnInterfaceElementDestroy onDestroy;
	OnInterfaceElementEvent onEnter;
	OnInterfaceElementEvent onExit;
	OnInterfaceElementEvent onStay;
	void* handle;
};
struct Interface
{
	Window* window;
	Transformer* transformer;
	float scale;
	InterfaceElement** elements;
	size_t elementCapacity;
	size_t elementCount;
	InterfaceElement* lastElement;
};

Interface* createInterface(
	Window* window,
	Transformer* transformer,
	float scale)
{
	assert(window != NULL);
	assert(transformer != NULL);

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
	interface->transformer = transformer;
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
Transformer* getInterfaceTransformer(
	const Interface* interface)
{
	assert(interface != NULL);
	return interface->transformer;
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

inline static Vec3F calcTransformPosition(
	float halfWidth,
	float halfHeight,
	uint8_t anchor,
	Vec3F position)
{
	switch (anchor)
	{
	default:
		abort();
	case CENTER_INTERFACE_ANCHOR:
		return position;
	case LEFT_INTERFACE_ANCHOR:
		return vec3F(
			position.x - halfWidth,
			position.y,
			position.z);
	case RIGHT_INTERFACE_ANCHOR:
		return vec3F(
			position.x + halfWidth,
			position.y,
			position.z);
	case BOTTOM_INTERFACE_ANCHOR:
		return vec3F(
			position.x,
			position.y - halfHeight,
			position.z);
	case TOP_INTERFACE_ANCHOR:
		return vec3F(
			position.x,
			position.y + halfHeight,
			position.z);
	case LEFT_BOTTOM_INTERFACE_ANCHOR:
		return vec3F(
			position.x - halfWidth,
			position.y - halfHeight,
			position.z);
	case LEFT_TOP_INTERFACE_ANCHOR:
		return vec3F(
			position.x - halfWidth,
			position.y + halfHeight,
			position.z);
	case RIGHT_BOTTOM_INTERFACE_ANCHOR:
		return vec3F(
			position.x + halfWidth,
			position.y - halfHeight,
			position.z);
	case RIGHT_TOP_INTERFACE_ANCHOR:
		return vec3F(
			position.x + halfWidth,
			position.y + halfHeight,
			position.z);
	}
}
inline static void updateElementPositions(
	float halfWidth,
	float halfHeight,
	InterfaceElement** elements,
	size_t elementCount)
{
	// TODO: check for update flag

	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement* element = elements[i];

		Vec3F transformPosition = calcTransformPosition(
			halfWidth,
			halfHeight,
			element->anchor,
			element->position);
		InterfaceElement* parent =
			element->parent;

		while (parent != NULL)
		{
			Vec3F parentPosition = calcTransformPosition(
				halfWidth,
				halfHeight,
				parent->anchor,
				parent->position);
			transformPosition = addVec3F(
				transformPosition,
				parentPosition);
			parent = parent->parent;
		}

		setTransformPosition(
			element->transform,
			transformPosition);
	}
}
Camera updateInterface(Interface* interface)
{
	assert(interface != NULL);

	// TODO: check for update flag

	InterfaceElement** elements = interface->elements;
	size_t elementCount = interface->elementCount;
	Window* window = interface->window;
	float scale = interface->scale;
	Vec2U windowSize = getWindowSize(window);

	Vec2F size;
	size.x = (float)windowSize.x / scale;
	size.y = (float)windowSize.y / scale;

	float halfWidth = size.x / 2.0f;
	float halfHeight = size.y / 2.0f;

	Camera camera = orthographicCamera(
		-halfWidth,
		halfWidth,
		-halfHeight,
		halfHeight,
		0.0f,
		1.0f);

	if (elementCount == 0)
		return camera;

	bool focused = isWindowFocused(window);

	if (focused == false)
		return camera;

	Vec2F cursor = getWindowCursorPosition(window);

	Vec2F cursorPosition = vec2F(
		(cursor.x / scale) - halfWidth,
		(size.y - (cursor.y / scale)) - halfHeight);

	updateElementPositions(
		halfWidth,
		halfHeight,
		elements,
		elementCount);

	InterfaceElement* newElement = NULL;

	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement* element = elements[i];

		Vec3F position = getTransformPosition(
			element->transform);
		Box2F bounds =
			element->bounds;
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

	updateElementPositions(
		halfWidth,
		halfHeight,
		elements,
		elementCount);

	return camera;
}

InterfaceElement* createInterfaceElement(
	Interface* interface,
	uint8_t anchor,
	Vec3F position,
	Box2F bounds,
	InterfaceElement* parent,
	bool update,
	OnInterfaceElementDestroy onDestroy,
	OnInterfaceElementEvent onEnter,
	OnInterfaceElementEvent onExit,
	OnInterfaceElementEvent onStay,
	void* handle)
{
	assert(interface != NULL);
	assert(anchor < INTERFACE_ANCHOR_COUNT);
	assert(onDestroy != NULL);

#ifndef NDEBUG
	if (parent != NULL)
		assert(interface == parent->interface);
#endif

	InterfaceElement* element = malloc(
		sizeof(InterfaceElement));

	if (element == NULL)
		return NULL;

	Transform* transformParent;

	if (parent != NULL)
	{
		assert(interface == parent->interface);
		transformParent = parent->transform;
	}
	else
	{
		transformParent = NULL;
	}

	Transform* transform = createTransform(
		interface->transformer,
		zeroVec3F(),
		oneVec3F(),
		oneQuat(),
		SPIN_ROTATION_TYPE,
		transformParent,
		update);

	if (transform == NULL)
	{
		free(element);
		return NULL;
	}

	element->interface = interface;
	element->transform = transform;
	element->anchor = anchor;
	element->position = position;
	element->bounds = bounds;
	element->parent = parent;
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

InterfaceElement* getInterfaceElementParent(
	const InterfaceElement* element)
{
	assert(element != NULL);
	return element->parent;
}
void setInterfaceElementParent(
	InterfaceElement* element,
	InterfaceElement* parent)
{
	assert(element != NULL);
	element->parent = parent;

	if (parent != NULL)
	{
		assert(element->interface ==
			parent->interface);

		setTransformParent(
			element->transform,
			parent->transform);
	}
	else
	{
		setTransformParent(
			element->transform,
			NULL);
	}
}

bool getInterfaceElementUpdate(
	const InterfaceElement* element)
{
	assert(element != NULL);

	return getTransformUpdate(
		element->transform);
}
void setInterfaceElementUpdate(
	InterfaceElement* element,
	bool update)
{
	assert(element != NULL);

	setTransformUpdate(
		element->transform,
		update);
}
