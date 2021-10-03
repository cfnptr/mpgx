#pragma once
#include "mpgx/renderer.h"
#include "mpgx/pipelines/gradsky_pipeline.h"

Renderer createGradSkyRenderer(
	Transform transform,
	Pipeline pipeline,
	RenderSorting sorting,
	bool useCulling,
	size_t capacity);
Render createGradSkyRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Mesh mesh);

Mesh getGradSkyRenderMesh(
	Render render);
void setGradSkyRenderMesh(
	Render render,
	Mesh mesh);
