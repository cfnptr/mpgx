#pragma once
#include "mpgx/renderer.h"
#include "mpgx/pipelines/sprite_pipeline.h"

Renderer createSpriteRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
	bool useCulling,
	size_t capacity);
Render createSpriteRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
	Mesh mesh);

Vec4F getSpriteRenderColor(
	Render render);
void setSpriteRenderColor(
	Render render,
	Vec4F color);

Mesh getSpriteRenderMesh(
	Render render);
void setSpriteRenderMesh(
	Render render,
	Mesh mesh);
