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
	Transform* parent);
void destroyRenderer(
	Renderer* renderer);

Pipeline* getRendererPipeline(
	const Renderer* renderer);
Transformer* getRendererTransformer(
	const Renderer* renderer);
Transform* getRendererParent(
	const Renderer* renderer);

bool getRendererSorting(
	const Renderer* renderer);
void setRendererSorting(
	Renderer* renderer,
	bool ascendingSorting);

void executeRenderer(
	Renderer* renderer,
	Camera camera);

Render* createRender(
	Renderer* renderer,
	bool draw,
	Vector3F position,
	Vector3F scale,
	Quaternion rotation,
	uint8_t rotationType,
	Render* parent,
	DestroyRender destroyFunction,
	RenderCommand renderFunction,
	void* handle);
void destroyRender(
	Render* render);

Renderer* getRenderRenderer(
	const Render* render);
Transform* getRenderTransform(
	const Render* render);
void* getRenderHandle(
	const Render* render);

bool getRenderDraw(
	const Render* render);
void setRenderDraw(
	Render* render,
	bool value);

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
