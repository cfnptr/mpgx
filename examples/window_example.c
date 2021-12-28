// Copyright 2020-2021 Nikita Fediuchin. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "mpgx/defines.h"
#include "mpgx/free_camera.h"
#include "mpgx/renderers/diffuse_renderer.h"
#include "mpgx/primitives/cube_primitive.h"

#include "cmmt/angle.h"

#include <stdio.h>
#include <string.h>

#define APPLICATION_NAME "MPGX - Window Example"

// Camera fly: WASD + RMB

typedef struct Client_T
{
	Window window;
	Transformer transformer;
	FreeCamera freeCamera;
	GraphicsRendererData data;
	GraphicsRenderer diffuseRenderer;
	GraphicsRender diffuseRender;
} Client_T;

typedef Client_T* Client;

inline static void rotateRender(
	double time,
	GraphicsRender graphicsRender)
{
	Quat rotation = eulerQuat(vec3F(
		sinf((float)time),
		cosf((float)time),
		0.0f));
	Transform transform = getGraphicsRenderTransform(
		graphicsRender);
	setTransformRotation(
		transform,
		rotation);
}
static void onWindowUpdate(void* handle)
{
	Client client = (Client)handle;
	Window window = client->window;
	FreeCamera freeCamera = client->freeCamera;

	rotateRender(
		getWindowUpdateTime(window),
		client->diffuseRender);

	updateFreeCamera(freeCamera);
	updateTransformer(client->transformer);

	if (beginWindowRecord(window) == false)
		return;

	Transform cameraTransform =
		getFreeCameraTransform(freeCamera);
	GraphicsRendererData* data = &client->data;

	createGraphicsRenderData(
		window,
		getTransformModel(cameraTransform),
		getFreeCamera(freeCamera),
		data,
		true);

	Framebuffer framebuffer =
		getWindowFramebuffer(window);

	FramebufferClear clearValues[2];
	clearValues[0].color = zeroLinearColor;
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	beginFramebufferRender(
		framebuffer,
		clearValues,
		2);
	drawGraphicsRenderer(
		client->diffuseRenderer,
		data);
	endFramebufferRender(framebuffer);

	endWindowRecord(window);
}

inline static GraphicsRenderer createDiffuseRendererInstance(Window window)
{
	const char* vertexShaderPath;
	const char* fragmentShaderPath;

	GraphicsAPI api = getWindowGraphicsAPI(window);

	if (api == VULKAN_GRAPHICS_API)
	{
		vertexShaderPath = "resources/shaders/vulkan/diffuse.vert.spv";
		fragmentShaderPath = "resources/shaders/vulkan/diffuse.frag.spv";
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		vertexShaderPath = "resources/shaders/opengl/diffuse.vert";
		fragmentShaderPath = "resources/shaders/opengl/diffuse.frag";
	}
	else
	{
		abort();
	}

	Shader vertexShader = createShaderFromFile(
		window,
		VERTEX_SHADER_TYPE,
		vertexShaderPath);

	if (vertexShader == NULL)
		return NULL;

	Shader fragmentShader = createShaderFromFile(
		window,
		FRAGMENT_SHADER_TYPE,
		fragmentShaderPath);

	if (fragmentShader == NULL)
	{
		destroyShader(vertexShader);
		return NULL;
	}

	GraphicsPipeline graphicsPipeline = createDiffusePipeline(
		getWindowFramebuffer(window),
		vertexShader,
		fragmentShader);

	if (graphicsPipeline == NULL)
	{
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	GraphicsRenderer graphicsRenderer = createDiffuseRenderer(
		graphicsPipeline,
		ASCENDING_GRAPHICS_RENDER_SORTING,
		true,
		MPGX_DEFAULT_CAPACITY);

	if (graphicsRenderer == NULL)
	{
		destroyGraphicsPipeline(
			graphicsPipeline,
			true);
		return NULL;
	}

	return graphicsRenderer;
}
inline static void destroyDiffuseRendererInstance(
	GraphicsRenderer diffuseRenderer)
{
	if (diffuseRenderer == NULL)
		return;

	GraphicsPipeline graphicsPipeline =
		getGraphicsRendererPipeline(diffuseRenderer);
	destroyGraphicsPipeline(
		graphicsPipeline,
		true);
	destroyGraphicsRenderer(diffuseRenderer);
}

inline static GraphicsRender createDiffuseRenderInstance(
	Window window,
	Transformer transformer,
	GraphicsRenderer diffuseRenderer)
{
	Vec3F position = vec3F(0.0f, 0.0f, 4.0f);

	Transform transform = createTransform(
		transformer,
		position,
		oneVec3F,
		oneQuat,
		SPIN_ROTATION_TYPE,
		NULL,
		true);

	if (transform == NULL)
		return NULL;

	Buffer vertexBuffer = createBuffer(
		window,
		VERTEX_BUFFER_TYPE,
		cubeTriangleVerticesNormals,
		sizeof(cubeTriangleVerticesNormals),
		true);

	if (vertexBuffer == NULL)
	{
		destroyTransform(transform);
		return NULL;
	}

	Buffer indexBuffer = createBuffer(
		window,
		INDEX_BUFFER_TYPE,
		cubeTriangleIndices,
		sizeof(cubeTriangleIndices),
		true);

	if (indexBuffer == NULL)
	{
		destroyBuffer(vertexBuffer);
		destroyTransform(transform);
		return NULL;
	}

	GraphicsMesh graphicsMesh = createGraphicsMesh(
		window,
		UINT16_INDEX_TYPE,
		sizeof(cubeTriangleIndices) / sizeof(uint16_t),
		0,
		vertexBuffer,
		indexBuffer);

	if (graphicsMesh == NULL)
	{
		destroyBuffer(indexBuffer);
		destroyBuffer(vertexBuffer);
		destroyTransform(transform);
		return NULL;
	}

	Box3F bounding = posExtBox3F(
		zeroVec3F,
		oneVec3F);
	GraphicsRender graphicsRender = createDiffuseRender(
		diffuseRenderer,
		transform,
		bounding,
		graphicsMesh);

	if (graphicsRender == NULL)
	{
		destroyGraphicsMesh(
			graphicsMesh,
			true);
		destroyTransform(transform);
		return NULL;
	}

	return graphicsRender;
}
inline static void destroyDiffuseRenderInstance(
	GraphicsRender diffuseRender)
{
	if (diffuseRender == NULL)
		return;

	GraphicsMesh graphicsMesh =
		getDiffuseRenderMesh(diffuseRender);
	destroyGraphicsRender(diffuseRender, true);
	destroyGraphicsMesh(graphicsMesh, true);
}

inline static Client createClient()
{
	Client client = malloc(
		sizeof(Client_T));

	if (client == NULL)
		return NULL;

	Window window;

	MpgxResult mpgxResult = createAnyWindow(
		defaultWindowSize,
		APPLICATION_NAME,
		onWindowUpdate,
		client,
		false,
		false,
		false,
		&window);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		printf("MPGX Error: %s.",
			mpgxResultToString(mpgxResult));
		free(client);
		return NULL;
	}

	Transformer transformer = createTransformer(
		MPGX_DEFAULT_CAPACITY);

	if (transformer == NULL)
	{
		destroyWindow(window);
		free(client);
		return NULL;
	}

	FreeCamera freeCamera = createFreeCamera(
		getWindowFramebuffer(window),
		transformer,
		1.0f,
		1.0f,
		degToRadF(60.0f),
		0.01f,
		100.0f);

	if (freeCamera == NULL)
	{
		destroyTransformer(transformer);
		destroyWindow(window);
		free(client);
		return NULL;
	}

	GraphicsRenderer diffuseRenderer = createDiffuseRendererInstance(window);

	if (diffuseRenderer == NULL)
	{
		destroyFreeCamera(freeCamera);
		destroyTransformer(transformer);
		destroyWindow(window);
		free(client);
		return NULL;
	}

	GraphicsRender diffuseRender = createDiffuseRenderInstance(
		window,
		transformer,
		diffuseRenderer);

	if (diffuseRender == NULL)
	{
		destroyDiffuseRendererInstance(diffuseRenderer);
		destroyFreeCamera(freeCamera);
		destroyTransformer(transformer);
		destroyWindow(window);
		free(client);
		return NULL;
	}

	client->window = window;
	client->transformer = transformer;
	client->freeCamera = freeCamera;
	client->diffuseRenderer = diffuseRenderer;
	client->diffuseRender = diffuseRender;

	memset(&client->data,
		0, sizeof(GraphicsRendererData));

	showWindow(window);
	return client;
}
inline static void destroyClient(Client client)
{
	if (client == NULL)
		return;

	destroyDiffuseRenderInstance(client->diffuseRender);
	destroyDiffuseRendererInstance(client->diffuseRenderer);
	destroyFreeCamera(client->freeCamera);
	destroyTransformer(client->transformer);
	destroyWindow(client->window);
	free(client);
}

inline static void updateClient(Client client)
{
	updateWindow(client->window);
}

int main()
{
	MpgxResult mpgxResult = initializeGraphics(
		APPLICATION_NAME,
		MPGX_VERSION_MAJOR,
		MPGX_VERSION_MINOR,
		MPGX_VERSION_PATCH);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return EXIT_FAILURE;

	Client client = createClient();

	if (client == NULL)
		return EXIT_FAILURE;

	updateClient(client);
	destroyClient(client);

	terminateGraphics();
	return EXIT_SUCCESS;
}
