#pragma once
#include "mpgx/text.h"
#include "mpgx/window.h"
#include "mpgx/camera.h"
#include "mpgx/transformer.h"

struct Renderer;
struct Render;

typedef void(*DestroyRender)(
	struct Render*);
typedef void(*RenderCommand)(
	struct Render*,
	struct Pipeline*,
	const struct Matrix4F*,
	const struct Matrix4F*,
	const struct Matrix4F*,
	const struct Matrix4F*);

struct Renderer* createRenderer(
	struct Window* window,
	bool ascendingSorting,
	enum CameraType cameraType,
	union Camera camera,
	struct Transform* transform,
	struct Pipeline* pipeline);
void destroyRenderer(
	struct Renderer* renderer);

struct Window* getRendererWindow(
	const struct Renderer* renderer);
bool getRendererAscendingSort(
	const struct Renderer* renderer);
enum CameraType getRendererCameraType(
	const struct Renderer* renderer);
struct Transform* getRendererTransformer(
	const struct Renderer* renderer);
struct Pipeline* getRendererPipeline(
	const struct Renderer* renderer);

union Camera getRendererCamera(
	const struct Renderer* renderer);
void setRendererCamera(
	struct Renderer* renderer,
	union Camera camera);

void executeRenderer(
	struct Renderer* renderer);

struct Render* createRender(
	struct Renderer* renderer,
	bool render,
	struct Transform* transform,
	DestroyRender destroyFunction,
	RenderCommand renderFunction,
	void* handle);
void destroyRender(
	struct Render* render);

struct Renderer* getRenderRenderer(
	const struct Render* render);
struct Transform* getRenderTransform(
	const struct Render* render);
struct Renderer* getRenderHandle(
	const struct Render* render);

bool getRenderRender(
	const struct Render* render);
void setRenderRender(
	struct Render* render,
	bool value);

struct Render* createColorRender(
	struct Renderer* renderer,
	bool render,
	struct Transform* transform,
	struct Mesh* mesh);
struct Render* createTextRender(
	struct Renderer* renderer,
	bool render,
	struct Transform* transform,
	struct Text* text);
