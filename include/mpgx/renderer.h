#pragma once
#include "mpgx/text.h"
#include "mpgx/window.h"
#include "mpgx/camera.h"
#include "mpgx/transformer.h"

typedef struct Renderer Renderer;
typedef struct Render Render;

typedef void(*DestroyRender)(
	void* render);
typedef void(*RenderCommand)(
	Render* render,
	Pipeline* pipeline,
	const Matrix4F* model,
	const Matrix4F* view,
	const Matrix4F* proj,
	const Matrix4F* viewProj,
	const Matrix4F* mvp);

// TODO: frustum culling

Renderer* createRenderer(
	Pipeline* pipeline,
	Transformer* transformer,
	bool ascendingSorting,
	Transform* transform,
	DestroyRender destroyFunction,
	RenderCommand renderFunction);
void destroyRenderer(Renderer* renderer);

Pipeline* getRendererPipeline(
	const Renderer* renderer);
Transformer* getRendererTransformer(
	const Renderer* renderer);
Transform* getRendererTransform(
	const Renderer* renderer);
DestroyRender getRendererDestroyFunction(
	const Renderer* renderer);
RenderCommand getRendererRenderFunction(
	const Renderer* renderer);

bool getRendererSorting(
	const Renderer* renderer);
void setRendererSorting(
	Renderer* renderer,
	bool ascendingSorting);

void updateRenderer(
	Renderer* renderer,
	Camera camera);

Render* createRender(
	Renderer* renderer,
	Vector3F position,
	Vector3F scale,
	Quaternion rotation,
	uint8_t rotationType,
	Render* parent,
	bool update,
	void* handle);
void destroyRender(Render* render);

Renderer* getRenderRenderer(const Render* render);
Transform* getRenderTransform(const Render* render);
void* getRenderHandle(const Render* render);

Vector3F getRenderPosition(
	const Render* render);
void setRenderPosition(
	Render* render,
	Vector3F position);

Vector3F getRenderScale(
	const Render* render);
void setRenderScale(
	Render* render,
	Vector3F scale);

Quaternion getRenderRotation(
	const Render* render);
void setRenderRotation(
	Render* render,
	Quaternion rotation);

Render* getRenderParent(
	const Render* render);
void setRenderParent(
	Render* render,
	Render* parent);

bool getRenderUpdate(
	const Render* render);
void setRenderUpdate(
	Render* render,
	bool update);
