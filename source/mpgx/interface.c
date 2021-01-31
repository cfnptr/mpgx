#include "mpgx/interface.h"

#include <assert.h>
#include <stdlib.h>

struct InterfaceElement
{
	struct Interface* interface;
	struct Transform* transform;
	uint8_t anchor;
	struct Vector3F position;
	struct BoundingBox2F bounds;
	struct InterfaceElement* parent;
	DestroyInterfaceElement destroyFunction;
	OnInterfaceElementEvent onEnterFunction;
	OnInterfaceElementEvent onExitFunction;
	OnInterfaceElementEvent onStayFunction;
	void* handle;
};
struct Interface
{
	struct Window* window;
	struct Transformer* transformer;
	float scale;
	struct InterfaceElement** elements;
	size_t elementCapacity;
	size_t elementCount;
	struct InterfaceElement* lastElement;
};

struct Interface* createInterface(
	struct Window* window,
	struct Transformer* transformer,
	float scale)
{
	assert(window != NULL);
	assert(transformer != NULL);

	struct Interface* interface = malloc(
		sizeof(struct Interface));

	if (interface == NULL)
		return NULL;

	struct InterfaceElement** elements = malloc(
		sizeof(struct InterfaceElement*));

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
	struct Interface* interface)
{
	if (interface == NULL)
		return;

	size_t elementCount =
		interface->elementCount;
	struct InterfaceElement** elements =
		interface->elements;

	for (size_t i = 0; i < elementCount; i++)
		free(elements[i]);

	free(elements);
	free(interface);
}

struct Window* getInterfaceWindow(
	const struct Interface* interface)
{
	assert(interface != NULL);
	return interface->window;
}
struct Transformer* getInterfaceTransformer(
	const struct Interface* interface)
{
	assert(interface != NULL);
	return interface->transformer;
}

inline static struct Vector3F calcTransformPosition(
	float halfWidth,
	float halfHeight,
	uint8_t anchor,
	struct Vector3F position)
{
	switch (anchor)
	{
	default:
		abort();
	case CENTER_INTERFACE_ANCHOR:
		return position;
	case LEFT_INTERFACE_ANCHOR:
		return createVector3F(
			position.x - halfWidth,
			position.y,
			position.z);
	case RIGHT_INTERFACE_ANCHOR:
		return createVector3F(
			position.x + halfWidth,
			position.y,
			position.z);
	case BOTTOM_INTERFACE_ANCHOR:
		return createVector3F(
			position.x,
			position.y - halfHeight,
			position.z);
	case TOP_INTERFACE_ANCHOR:
		return createVector3F(
			position.x,
			position.y + halfHeight,
			position.z);
	case LEFT_BOTTOM_INTERFACE_ANCHOR:
		return createVector3F(
			position.x - halfWidth,
			position.y - halfHeight,
			position.z);
	case LEFT_TOP_INTERFACE_ANCHOR:
		return createVector3F(
			position.x - halfWidth,
			position.y + halfHeight,
			position.z);
	case RIGHT_BOTTOM_INTERFACE_ANCHOR:
		return createVector3F(
			position.x + halfWidth,
			position.y - halfHeight,
			position.z);
	case RIGHT_TOP_INTERFACE_ANCHOR:
		return createVector3F(
			position.x + halfWidth,
			position.y + halfHeight,
			position.z);
	}
}
inline static void updateElementPositions(
	float halfWidth,
	float halfHeight,
	struct InterfaceElement** elements,
	size_t elementCount)
{
	for (size_t i = 0; i < elementCount; i++)
	{
		struct InterfaceElement* element =
			elements[i];
		struct Vector3F transformPosition = calcTransformPosition(
			halfWidth,
			halfHeight,
			element->anchor,
			element->position);
		struct InterfaceElement* parent =
			element->parent;

		while (parent != NULL)
		{
			struct Vector3F parentPosition = calcTransformPosition(
				halfWidth,
				halfHeight,
				parent->anchor,
				parent->position);
			transformPosition = addVector3F(
				transformPosition,
				parentPosition);
			parent = parent->parent;
		}

		setTransformPosition(
			element->transform,
			transformPosition);
	}
}
union Camera executeInterface(
	struct Interface* interface)
{
	assert(interface != NULL);

	size_t elementCount =
		interface->elementCount;
	struct InterfaceElement** elements =
		interface->elements;

	struct Window* window =
		interface->window;
	float scale =
		interface->scale;
	struct Vector2I windowSize =
		getWindowSize(window);

	struct Vector2F size;
	size.x = (float)windowSize.x / scale;
	size.y = (float)windowSize.y / scale;

	float halfWidth = size.x / 2.0f;
	float halfHeight = size.y / 2.0f;

	union Camera camera = createOrthographicCamera(
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

	struct Vector2F cursor =
		getWindowCursorPosition(window);

	struct Vector2F cursorPosition = createVector2F(
		(cursor.x / scale) - halfWidth,
		(size.y - (cursor.y / scale)) - halfHeight);

	updateElementPositions(
		halfWidth,
		halfHeight,
		elements,
		elementCount);

	struct InterfaceElement* newElement = NULL;

	for (size_t i = 0; i < elementCount; i++)
	{
		struct InterfaceElement* element =
			elements[i];

		struct Vector3F position = getTransformPosition(
			element->transform);
		struct BoundingBox2F bounds =
			element->bounds;
		bounds.minimum = addVector2F(
			bounds.minimum,
			createVector2F(position.x, position.y));
		bounds.maximum = addVector2F(
			bounds.maximum,
			createVector2F(position.x, position.y));

		bool colliding = isPointCollidingBoundingBox2F(
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

	struct InterfaceElement* lastElement =
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

struct InterfaceElement* createInterfaceElement(
	struct Interface* interface,
	uint8_t anchor,
	struct Vector3F position,
	struct BoundingBox2F bounds,
	struct InterfaceElement* parent,
	DestroyInterfaceElement destroyFunction,
	OnInterfaceElementEvent onEnterFunction,
	OnInterfaceElementEvent onExitFunction,
	OnInterfaceElementEvent onStayFunction,
	void* handle)
{
	assert(interface != NULL);
	assert(destroyFunction != NULL);

#ifndef NDEBUG
	if (parent != NULL)
	{
		assert(interface ==
			parent->interface);
	}
#endif

	struct InterfaceElement* element = malloc(
		sizeof(struct InterfaceElement));

	if (element == NULL)
		return NULL;

	struct Transform* transformParent;

	if (parent != NULL)
	{
		assert(interface == parent->interface);
		transformParent = parent->transform;
	}
	else
	{
		transformParent = NULL;
	}

	struct Transform* transform = createTransform(
		interface->transformer,
		createZeroVector3F(),
		createOneVector3F(),
		createOneQuaternion(),
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
		struct InterfaceElement** elements = realloc(
			interface->elements,
			capacity * sizeof(struct InterfaceElement*));

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
	struct InterfaceElement* element)
{
	if (element == NULL)
		return;

	struct Interface* interface =
		element->interface;
	size_t elementCount =
		interface->elementCount;
	struct InterfaceElement** elements =
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

struct Interface* getInterfaceElementInterface(
	const struct InterfaceElement* element)
{
	assert(element != NULL);
	return element->interface;
}
struct Transform* getInterfaceElementTransform(
	const struct InterfaceElement* element)
{
	assert(element != NULL);
	return element->transform;
}
void* getInterfaceElementHandle(
	const struct InterfaceElement* element)
{
	assert(element != NULL);
	return element->handle;
}

uint8_t getInterfaceElementAnchor(
	const struct InterfaceElement* element)
{
	assert(element != NULL);
	return element->anchor;
}
void setInterfaceElementAnchor(
	struct InterfaceElement* element,
	uint8_t anchor)
{
	assert(element != NULL);
	element->anchor = anchor;
}

struct Vector3F getInterfaceElementPosition(
	const struct InterfaceElement* element)
{
	assert(element != NULL);
	return element->position;
}
void setInterfaceElementPosition(
	struct InterfaceElement* element,
	struct Vector3F position)
{
	assert(element != NULL);
	element->position = position;
}

struct BoundingBox2F getInterfaceElementBounds(
	const struct InterfaceElement* element)
{
	assert(element != NULL);
	return element->bounds;
}
void setInterfaceElementBounds(
	struct InterfaceElement* element,
	struct BoundingBox2F bounds)
{
	assert(element != NULL);
	element->bounds = bounds;
}

struct InterfaceElement* getInterfaceElementParent(
	const struct InterfaceElement* element)
{
	assert(element != NULL);
	return element->parent;
}
void setInterfaceElementParent(
	struct InterfaceElement* element,
	struct InterfaceElement* parent)
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
