#pragma once
#include "mpgx/window.h"
#include "mpgx/camera.h"
#include "mpgx/bounding.h"
#include "mpgx/transformer.h"

enum INTERFACE_ANCHOR
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
};

struct Interface;
struct InterfaceElement;

typedef void(*DestroyInterfaceElement)(
	void* element);
typedef void(*OnInterfaceElementEnter)(
	struct InterfaceElement* element);
typedef void(*OnInterfaceElementExit)(
	struct InterfaceElement* element);
typedef void(*OnInterfaceElementStay)(
	struct InterfaceElement* element);

struct Interface* createInterface(
	struct Window* window,
	struct Transformer* transformer);
void destroyInterface(
	struct Interface* interface);

union Camera executeInterface(
	struct Interface* interface);

// TODO: rotation

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
	void* handle);
void destroyInterfaceElement(
	struct InterfaceElement* element);

struct Interface* getInterfaceElementInterface(
	const struct InterfaceElement* element);
struct Transform* getInterfaceElementTransform(
	const struct InterfaceElement* element);
void* getInterfaceElementHandle(
	const struct InterfaceElement* element);

bool getInterfaceElementUpdate(
	const struct InterfaceElement* element);
void setInterfaceElementUpdate(
	struct InterfaceElement* element,
	bool update);

uint8_t getInterfaceElementAnchor(
	const struct InterfaceElement* element);
void setInterfaceElementAnchor(
	struct InterfaceElement* element,
	uint8_t anchor);

struct Vector3F getInterfaceElementPosition(
	const struct InterfaceElement* element);
void setInterfaceElementPosition(
	struct InterfaceElement* element,
	struct Vector3F position);

struct BoundingBox2F getInterfaceElementBounds(
	const struct InterfaceElement* element);
void setInterfaceElementBounds(
	struct InterfaceElement* element,
	struct BoundingBox2F bounds);
