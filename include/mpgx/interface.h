#pragma once
#include "mpgx/window.h"
#include "mpgx/transformer.h"

#include "cmmt/vector.h"
#include "cmmt/camera.h"
#include "cmmt/bounding.h"

typedef enum InterfaceAnchor
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
	INTERFACE_ANCHOR_COUNT = 9,
} InterfaceAnchor;

typedef struct Interface* Interface;
typedef struct InterfaceElement* InterfaceElement;

typedef void(*OnInterfaceElementDestroy)(
	void* handle);
typedef void(*OnInterfaceElementEvent)(
	InterfaceElement element);

Interface createInterface(
	Window window,
	float scale,
	size_t capacity);
void destroyInterface(Interface interface);

bool isInterfaceEmpty(Interface interface);
Window getInterfaceWindow(Interface interface);

float getInterfaceScale(
	Interface interface);
void setInterfaceScale(
	Interface interface,
	float scale);

Camera createInterfaceCamera(
	Interface interface);

void preUpdateInterface(Interface interface);
void updateInterface(Interface interface);

InterfaceElement createInterfaceElement(
	Interface interface,
	InterfaceAnchor anchor,
	Vec3F position,
	Box2F bounds,
	Transform transform,
	OnInterfaceElementDestroy onDestroy,
	OnInterfaceElementEvent onEnter,
	OnInterfaceElementEvent onExit,
	OnInterfaceElementEvent onStay,
	void* handle);
void destroyInterfaceElement(InterfaceElement element);

Interface getInterfaceElementInterface(
	InterfaceElement element);
Transform getInterfaceElementTransform(
	InterfaceElement element);
OnInterfaceElementDestroy getInterfaceElementOnDestroy(
	InterfaceElement element);
OnInterfaceElementEvent getInterfaceElementOnEnter(
	InterfaceElement element);
OnInterfaceElementEvent getInterfaceElementOnExit(
	InterfaceElement element);
OnInterfaceElementEvent getInterfaceElementOnStay(
	InterfaceElement element);
void* getInterfaceElementHandle(
	InterfaceElement element);

InterfaceAnchor getInterfaceElementAnchor(
	InterfaceElement element);
void setInterfaceElementAnchor(
	InterfaceElement element,
	InterfaceAnchor anchor);

Vec3F getInterfaceElementPosition(
	InterfaceElement element);
void setInterfaceElementPosition(
	InterfaceElement element,
	Vec3F position);

Box2F getInterfaceElementBounds(
	InterfaceElement element);
void setInterfaceElementBounds(
	InterfaceElement element,
	Box2F bounds);
