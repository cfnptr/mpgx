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

typedef struct Interface Interface;
typedef struct InterfaceElement InterfaceElement;

typedef void(*OnInterfaceElementDestroy)(
	void* handle);
typedef void(*OnInterfaceElementEvent)(
	InterfaceElement* element);

Interface* createInterface(
	Window* window,
	float scale);
void destroyInterface(Interface* interface);

Window* getInterfaceWindow(
	const Interface* interface);

float getInterfaceScale(
	const Interface* interface);
void setInterfaceScale(
	Interface* interface,
	float scale);

Camera createInterfaceCamera(
	const Interface* interface);

void preUpdateInterface(Interface* interface);
void updateInterface(Interface* interface);

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
