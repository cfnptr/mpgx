#include "mpgx/renderers/gradsky_renderer.h"
#include <string.h>

typedef struct GradSkyRender
{
	Vec4F color;
	Mesh mesh;
} GradSkyRender;

// TODO:

Renderer createGradSkyRenderer(
	Transform transform,
	Pipeline pipeline,
	uint8_t sortingType,
	size_t capacity)
{

}
Render createGradSkyRender(
	Renderer renderer,
	Transform transform,
	Box3F bounding,
	Vec4F color,
	Mesh mesh)
{

}

Vec4F getGradSkyRenderColor(
	Render render)
{

}
void setGradSkyRenderColor(
	Render render,
	Vec4F color)
{

}

Mesh getGradSkyRenderMesh(
	Render render)
{

}
void setGradSkyRenderMesh(
	Render render,
	Mesh mesh)
{

}
