#pragma once
#include "mpgx/window.h"
#include "mpgx/camera.h"
#include "mpgx/transformer.h"

#include "cmmt/vector.h"
#include "cmmt/bounding.h"

typedef enum INTERFACE_ANCHOR
{
	CENTER_INTERFACE_ANCHOR,
	LEFT_INTERFACE_ANCHOR,
	RIGHT_INTERFACE_ANCHOR,
	BOTTOM_INTERFACE_ANCHOR,
	TOP_INTERFACE_ANCHOR,
	LEFT_BOTTOM_INTERFACE_ANCHOR,
	LEFT_TOP_INTERFACE_ANCHOR,
	RIGHT_BOTTOM_INTERFACE_ANCHOR,
	RIGHT_TOP_INTERFACE_ANCHOR,
	INTERFACE_ANCHOR_COUNT,
} INTERFACE_ANCHOR;

// TODO: add default button element, input box, etc..

typedef struct Interface Interface;
typedef struct InterfaceElement InterfaceElement;

typedef void(*OnInterfaceElementDestroy)(
	void* element);
typedef void(*OnInterfaceElementEvent)(
	InterfaceElement* element);

Interface* createInterface(
	Window* window,
	Transformer* transformer,
	float scale);
void destroyInterface(Interface* interface);

Window* getInterfaceWindow(
	const Interface* interface);
Transformer* getInterfaceTransformer(
	const Interface* interface);

float getInterfaceScale(
	const Interface* interface);
void setInterfaceScale(
	Interface* interface,
	float scale);

Camera updateInterface(Interface* interface);

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
	void* handle);
void destroyInterfaceElement(
	InterfaceElement* element);

Interface* getInterfaceElementInterface(
	const InterfaceElement* element);
Transform* getInterfaceElementTransform(
	const InterfaceElement* element);
OnInterfaceElementDestroy getInterfaceElementOnDestroy(
	const InterfaceElement* element);
OnInterfaceElementEvent getInterfaceElementOnEnter(
	const InterfaceElement* element);
OnInterfaceElementEvent getInterfaceElementOnExit(
	const InterfaceElement* element);
OnInterfaceElementEvent getInterfaceElementOnStay(
	const InterfaceElement* element);
void* getInterfaceElementHandle(
	const InterfaceElement* element);

uint8_t getInterfaceElementAnchor(
	const InterfaceElement* element);
void setInterfaceElementAnchor(
	InterfaceElement* element,
	uint8_t anchor);

Vec3F getInterfaceElementPosition(
	const InterfaceElement* element);
void setInterfaceElementPosition(
	InterfaceElement* element,
	Vec3F position);

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

bool getInterfaceElementUpdate(
	const InterfaceElement* element);
void setInterfaceElementUpdate(
	InterfaceElement* element,
	bool update);
