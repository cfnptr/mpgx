#include "mpgx/interface.h"

#include <assert.h>
#include <stdlib.h>

struct InterfaceElement
{
	Interface* interface;
	Transform* transform;
	uint8_t anchor;
	Vector3F position;
	Box2F bounds;
	InterfaceElement* parent;
	DestroyInterfaceElement destroyFunction;
	OnInterfaceElementEvent onEnterFunction;
	OnInterfaceElementEvent onExitFunction;
	OnInterfaceElementEvent onStayFunction;
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

void destroyInterface(
	Interface* interface)
{
	if (interface == NULL)
		return;

	size_t elementCount =
		interface->elementCount;
	InterfaceElement** elements =
		interface->elements;

	for (size_t i = 0; i < elementCount; i++)
		free(elements[i]);

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

inline static Vector3F calcTransformPosition(
	float halfWidth,
	float halfHeight,
	uint8_t anchor,
	Vector3F position)
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
	for (size_t i = 0; i < elementCount; i++)
	{
		InterfaceElement* element =
			elements[i];
		Vector3F transformPosition = calcTransformPosition(
			halfWidth,
			halfHeight,
			element->anchor,
			element->position);
		InterfaceElement* parent =
			element->parent;

		while (parent != NULL)
		{
			Vector3F parentPosition = calcTransformPosition(
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
Camera executeInterface(
	Interface* interface)
{
	assert(interface != NULL);

	size_t elementCount =
		interface->elementCount;
	InterfaceElement** elements =
		interface->elements;

	Window* window =
		interface->window;
	float scale =
		interface->scale;
	Vector2I windowSize =
		getWindowSize(window);

	Vector2F size;
	size.x = (float)windowSize.x / scale;
	size.y = (float)windowSize.y / scale;

	float halfWidth = size.x / 2.0f;
	float halfHeight = size.y / 2.0f;

	Camera camera = createOrthographicCamera(
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

	Vector2F cursor =
		getWindowCursorPosition(window);

	Vector2F cursorPosition = vec2F(
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
		InterfaceElement* element =
			elements[i];

		Vector3F position = getTransformPosition(
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
			if (newElement->onEnterFunction != NULL)
				newElement->onEnterFunction(newElement);
			interface->lastElement = newElement;
		}
	}
	else
	{
		if (lastElement != newElement)
		{
			if (lastElement->onExitFunction != NULL)
				lastElement->onExitFunction(lastElement);

			if (newElement != NULL &&
				newElement->onEnterFunction != NULL)
			{
				newElement->onEnterFunction(newElement);
			}

			interface->lastElement = newElement;
		}
		else
		{
			if (lastElement->onStayFunction != NULL)
				lastElement->onStayFunction(lastElement);
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
	Vector3F position,
	Box2F bounds,
	InterfaceElement* parent,
	DestroyInterfaceElement destroyFunction,
	OnInterfaceElementEvent onEnterFunction,
	OnInterfaceElementEvent onExitFunction,
	OnInterfaceElementEvent onStayFunction,
	void* handle)
{
	assert(interface != NULL);
	assert(anchor < INTERFACE_ANCHOR_COUNT);
	assert(destroyFunction != NULL);

#ifndef NDEBUG
	if (parent != NULL)
	{
		assert(interface ==
			parent->interface);
	}
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
		transformParent);

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
	element->destroyFunction = destroyFunction;
	element->onEnterFunction = onEnterFunction;
	element->onExitFunction = onExitFunction;
	element->onStayFunction = onStayFunction;
	element->handle = handle;

	if (interface->elementCount ==
		interface->elementCapacity)
	{
		size_t capacity =
			interface->elementCapacity * 2;
		InterfaceElement** elements = realloc(
			interface->elements,
			capacity * sizeof(InterfaceElement*));

		if (elements == NULL)
		{
			destroyTransform(transform);
			free(element);
			return NULL;
		}

		interface->elements = elements;
		interface->elementCapacity = capacity;
	}

	interface->elements[
		interface->elementCount] = element;
	interface->elementCount++;
	return element;
}
void destroyInterfaceElement(
	InterfaceElement* element)
{
	if (element == NULL)
		return;

	Interface* interface =
		element->interface;
	size_t elementCount =
		interface->elementCount;
	InterfaceElement** elements =
		interface->elements;

	for (size_t i = 0; i < elementCount; i++)
	{
		if (elements[i] == element)
		{
			for (size_t j = i + 1; j < elementCount; j++)
				elements[j - 1] = elements[j];

			element->destroyFunction(element->handle);
			destroyTransform(element->transform);
			free(element);

			interface->elementCount--;
			return;
		}
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

Vector3F getInterfaceElementPosition(
	const InterfaceElement* element)
{
	assert(element != NULL);
	return element->position;
}
void setInterfaceElementPosition(
	InterfaceElement* element,
	Vector3F position)
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
