#pragma once
#include "mpgx/renderer.h"
#include "mpgx/pipelines/diffuse_pipeline.h"

Renderer* createDiffuseRenderer(
	Transform* transform,
	Pipeline* pipeline,
	uint8_t sortingType);
Render* createDiffuseRender(
	Renderer* renderer,
	Transform* transform,
	Box3F bounding,
	Mesh* mesh);

Mesh* getDiffuseRenderMesh(
	const Render* render);
void setDiffuseRenderMesh(
	Render* render,
	Mesh* mesh);
