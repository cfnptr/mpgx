#include "mpgx/interface.h"

#include <assert.h>
#include <stdlib.h>

struct InterfaceElement
{
	struct Interface* interface;
	struct Transform* transform;
	bool update;
	uint8_t anchor;
	struct Vector3F position;
	struct BoundingBox2F bounds;
	DestroyInterfaceElement destroyFunction;
	OnInterfaceElementEnter onEnterFunction;
	OnInterfaceElementExit onExitFunction;
	OnInterfaceElementStay onStayFunction;
	void* handle;
};
struct Interface
{
	struct Window* window;
	struct Transformer* transformer;
	struct InterfaceElement** elements;
	size_t elementCapacity;
	size_t elementCount;
	struct InterfaceElement* lastElement;
};

struct Interface* createInterface(
	struct Window* window,
	struct Transformer* transformer)
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

inline static struct Vector2F calcTransformPosition(
	uint8_t anchor,
	struct Vector3F elementPosition,
	float halfWidth,
	float halfHeight)
{
	struct Vector2F transformPosition;

	switch (anchor)
	{
	default:
		abort();
	case CENTER_INTERFACE_ANCHOR:
		transformPosition.x = elementPosition.x;
		transformPosition.y = elementPosition.y;
		break;
	case LEFT_INTERFACE_ANCHOR:
		transformPosition.x = elementPosition.x - halfWidth;
		transformPosition.y = elementPosition.y;
		break;
	case RIGHT_INTERFACE_ANCHOR:
		transformPosition.x = elementPosition.x + halfWidth;
		transformPosition.y = elementPosition.y;
		break;
	case BOTTOM_INTERFACE_ANCHOR:
		transformPosition.x = elementPosition.x;
		transformPosition.y = elementPosition.y - halfHeight;
		break;
	case TOP_INTERFACE_ANCHOR:
		transformPosition.x = elementPosition.x;
		transformPosition.y = elementPosition.y + halfHeight;
		break;
	case LEFT_BOTTOM_INTERFACE_ANCHOR:
		transformPosition.x = elementPosition.x - halfWidth;
		transformPosition.y = elementPosition.y - halfHeight;
		break;
	case LEFT_TOP_INTERFACE_ANCHOR:
		transformPosition.x = elementPosition.x - halfWidth;
		transformPosition.y = elementPosition.y + halfHeight;
		break;
	case RIGHT_BOTTOM_INTERFACE_ANCHOR:
		transformPosition.x = elementPosition.x + halfWidth;
		transformPosition.y = elementPosition.y - halfHeight;
		break;
	case RIGHT_TOP_INTERFACE_ANCHOR:
		transformPosition.x = elementPosition.x + halfWidth;
		transformPosition.y = elementPosition.y + halfHeight;
		break;
	}

	return transformPosition;
}
union Camera executeInterface(
	struct Interface* interface)
{
	assert(interface != NULL);

	struct Window* window =
		interface->window;

	size_t width, height;

	getWindowSize(
		window,
		&width,
		&height);

	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;

	double cursorX, cursorY;

	getWindowCursorPosition(
		window,
		&cursorX,
		&cursorY);

	struct Vector2F cursorPosition = createVector2F(
		(float)cursorX,
		(float)cursorY);

	size_t elementCount =
		interface->elementCount;
	struct InterfaceElement** elements =
		interface->elements;

	struct InterfaceElement* newElement = NULL;

	for (size_t i = 0; i < elementCount; i++)
	{
		struct InterfaceElement* element =
			elements[i];

		if (element->update == false)
			continue;

		struct Vector2F position = calcTransformPosition(
			element->anchor,
			element->position,
			halfWidth,
			halfHeight);

		struct BoundingBox2F bounds =
			element->bounds;
		bounds.minimum = addVector2F(
			bounds.minimum,
			position);
		bounds.maximum = addVector2F(
			bounds.maximum,
			position);

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
			if (newElement->onEnterFunction != NULL)
				newElement->onEnterFunction(newElement);
			interface->lastElement = newElement;
		}
		else
		{
			if (lastElement->onStayFunction != NULL)
				lastElement->onStayFunction(lastElement);
		}
	}

	for (size_t i = 0; i < elementCount; i++)
	{
		struct InterfaceElement* element =
			elements[i];

		struct Vector3F elementPosition =
			element->position;
		struct Vector2F position = calcTransformPosition(
			element->anchor,
			elementPosition,
			halfWidth,
			halfHeight);
		struct Vector3F transformPosition = createVector3F(
			position.x,
			position.y,
			elementPosition.z);

		setTransformPosition(
			element->transform,
			transformPosition);
	}

	return createOrthographicCamera(
		width / -2.0f,
		width / 2.0f,
		height / -2.0f,
		height / 2.0f,
		0.0f,
		1.0f);
}

struct InterfaceElement* createInterfaceElement(
	struct Interface* interface,
	bool update,
	uint8_t anchor,
	struct BoundingBox2F bounds,
	struct Vector3F position,
	struct Vector3F scale,
	struct Quaternion rotation,
	struct Transform* parent,
	DestroyInterfaceElement destroyFunction,
	OnInterfaceElementEnter onEnterFunction,
	OnInterfaceElementExit onExitFunction,
	OnInterfaceElementStay onStayFunction,
	void* handle)
{
	assert(interface != NULL);
	assert(destroyFunction != NULL);

	struct InterfaceElement* element = malloc(
		sizeof(struct InterfaceElement));

	if (element == NULL)
		return NULL;

	struct Transform* transform = createTransform(
		interface->transformer,
		createZeroVector3F(),
		scale,
		rotation,
		parent);

	if (transform == NULL)
	{
		free(element);
		return NULL;
	}

	element->interface = interface;
	element->transform = transform;
	element->update = update;
	element->anchor = anchor;
	element->position = position;
	element->bounds = bounds;
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

bool getInterfaceElementUpdate(
	const struct InterfaceElement* element)
{
	assert(element != NULL);
	return element->update;
}
void setInterfaceElementUpdate(
	struct InterfaceElement* element,
	bool update)
{
	assert(element != NULL);
	element->update = update;
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
