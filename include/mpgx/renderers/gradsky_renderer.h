#pragma once
#include "mpgx/renderer.h"
#include "mpgx/pipelines/gradsky_pipeline.h"

Renderer createGradSkyRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
	size_t capacity);
Render createGradSkyRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
	Vec3F sunDirection,
	Mesh mesh);

Vec4F getGradSkyRenderColor(
	Render render);
void setGradSkyRenderColor(
	Render render,
	Vec4F color);

Vec3F getGradSkyRenderSunDirection(
	Render render);
void setGradSkyRenderSunDirection(
	Render render,
	Vec3F sunDirection);

Mesh getGradSkyRenderMesh(
	Render render);
void setGradSkyRenderMesh(
	Render render,
	Mesh mesh);
