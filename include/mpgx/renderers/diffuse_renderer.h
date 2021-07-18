#pragma once
#include "mpgx/renderer.h"
#include "mpgx/pipelines/diffuse_pipeline.h"

Renderer createDiffuseRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
	size_t capacity);
Render createDiffuseRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Mesh mesh);

Mesh getDiffuseRenderMesh(
	Render render);
void setDiffuseRenderMesh(
	Render render,
	Mesh mesh);