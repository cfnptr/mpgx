#pragma once
#include "mpgx/renderer.h"
#include "mpgx/pipelines/gradsky_pipeline.h"

Renderer createGradSkyRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
	bool useCulling,
	size_t capacity);
Render createGradSkyRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	float sunHeight,
	Mesh mesh);

float getGradSkyRenderSunHeight(
	Render render);
void setGradSkyRenderSunHeight(
	Render render,
	float sunHeight);

Mesh getGradSkyRenderMesh(
	Render render);
void setGradSkyRenderMesh(
	Render render,
	Mesh mesh);
