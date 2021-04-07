#pragma once
#include "mpgx/window.h"
#include "mpgx/camera.h"
#include "mpgx/transformer.h"

#include "cmmt/vector.h"
#include "cmmt/bounding.h"

typedef enum INTERFACE_ANCHOR
{
	CENTER_INTERFACE_ANCHOR = 0,
	LEFT_INTERFACE_ANCHOR = 1,
	RIGHT_INTERFACE_ANCHOR = 2,
	BOTTOM_INTERFACE_ANCHOR = 3,
	TOP_INTERFACE_ANCHOR = 4,
	LEFT_BOTTOM_INTERFACE_ANCHOR = 5,
	LEFT_TOP_INTERFACE_ANCHOR = 6,
	RIGHT_BOTTOM_INTERFACE_ANCHOR = 7,
	RIGHT_TOP_INTERFACE_ANCHOR = 8,
} INTERFACE_ANCHOR;

typedef struct Interface Interface;
typedef struct InterfaceElement InterfaceElement;

typedef void(*DestroyInterfaceElement)(
	void* element);
typedef void(*OnInterfaceElementEvent)(
	InterfaceElement* element);

Interface* createInterface(
	Window* window,
	Transformer* transformer,
	float scale);
void destroyInterface(
	Interface* interface);

Window* getInterfaceWindow(
	const Interface* interface);
Transformer* getInterfaceTransformer(
	const Interface* interface);

Camera executeInterface(
	Interface* interface);

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
	void* handle);
void destroyInterfaceElement(
	InterfaceElement* element);

Interface* getInterfaceElementInterface(
	const InterfaceElement* element);
Transform* getInterfaceElementTransform(
	const InterfaceElement* element);
void* getInterfaceElementHandle(
	const InterfaceElement* element);

uint8_t getInterfaceElementAnchor(
	const InterfaceElement* element);
void setInterfaceElementAnchor(
	InterfaceElement* element,
	uint8_t anchor);

Vector3F getInterfaceElementPosition(
	const InterfaceElement* element);
void setInterfaceElementPosition(
	InterfaceElement* element,
	Vector3F position);

Box2F getInterfaceElementBounds(
	const InterfaceElement* element);
void setInterfaceElementBounds(
	InterfaceElement* element,
	Box2F bounds);

InterfaceElement* getInterfaceElementParent(
	const InterfaceElement* element);
void setInterfaceElementParent(
	InterfaceElement* element,
	InterfaceElement* parent);
