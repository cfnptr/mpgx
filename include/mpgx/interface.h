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
typedef void(*OnInterfaceElementEvent)(
	struct InterfaceElement* element);

struct Interface* createInterface(
	struct Window* window,
	struct Transformer* transformer,
	float scale);
void destroyInterface(
	struct Interface* interface);

struct Window* getInterfaceWindow(
	const struct Interface* interface);
struct Transformer* getInterfaceTransformer(
	const struct Interface* interface);

union Camera executeInterface(
	struct Interface* interface);

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
	void* handle);
void destroyInterfaceElement(
	struct InterfaceElement* element);

struct Interface* getInterfaceElementInterface(
	const struct InterfaceElement* element);
struct Transform* getInterfaceElementTransform(
	const struct InterfaceElement* element);
void* getInterfaceElementHandle(
	const struct InterfaceElement* element);

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

struct InterfaceElement* getInterfaceElementParent(
	const struct InterfaceElement* element);
void setInterfaceElementParent(
	struct InterfaceElement* element,
	struct InterfaceElement* parent);
