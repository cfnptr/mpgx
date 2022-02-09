// Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
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
		(float)sin(time),
		(float)cos(time),
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

	rotateRender(getWindowUpdateTime(window),
		client->diffuseRender);

	updateFreeCamera(freeCamera);
	updateTransformer(client->transformer);

	MpgxResult mpgxResult = beginWindowRecord(window);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
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

inline static MpgxResult createDiffuseRendererInstance(
	Window window,
	GraphicsRenderer* diffuseRenderer)
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

	Shader vertexShader;

	MpgxResult mpgxResult = createShaderFromFile(
		window,
		VERTEX_SHADER_TYPE,
		vertexShaderPath,
		&vertexShader);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	Shader fragmentShader;

	mpgxResult = createShaderFromFile(
		window,
		FRAGMENT_SHADER_TYPE,
		fragmentShaderPath,
		&fragmentShader);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyShader(vertexShader);
		return mpgxResult;
	}

	GraphicsPipeline pipeline;

	mpgxResult = createDiffusePipeline(
		getWindowFramebuffer(window),
		vertexShader,
		fragmentShader,
		&pipeline);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return mpgxResult;
	}

	GraphicsRenderer renderer = createDiffuseRenderer(
		pipeline,
		ASCENDING_GRAPHICS_RENDER_SORTING,
		true,
		MPGX_DEFAULT_CAPACITY,
		NULL);

	if (!renderer)
	{
		destroyGraphicsPipeline(
			pipeline,
			true);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	*diffuseRenderer = renderer;
	return SUCCESS_MPGX_RESULT;
}
inline static void destroyDiffuseRendererInstance(
	GraphicsRenderer diffuseRenderer)
{
	if (!diffuseRenderer)
		return;

	GraphicsPipeline graphicsPipeline =
		getGraphicsRendererPipeline(diffuseRenderer);
	destroyGraphicsPipeline(
		graphicsPipeline,
		true);
	destroyGraphicsRenderer(diffuseRenderer);
}

inline static MpgxResult createDiffuseRenderInstance(
	Window window,
	Transformer transformer,
	GraphicsRenderer diffuseRenderer,
	GraphicsRender* diffuseRender)
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

	if (!transform)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	Buffer vertexBuffer;

	MpgxResult mpgxResult = createBuffer(
		window,
		VERTEX_BUFFER_TYPE,
		GPU_ONLY_BUFFER_USAGE,
		cubeTriangleVerticesNormals,
		sizeof(cubeTriangleVerticesNormals),
		&vertexBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyTransform(transform);
		return mpgxResult;
	}

	Buffer indexBuffer;

	mpgxResult = createBuffer(
		window,
		INDEX_BUFFER_TYPE,
		GPU_ONLY_BUFFER_USAGE,
		cubeTriangleIndices,
		sizeof(cubeTriangleIndices),
		&indexBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyBuffer(vertexBuffer);
		destroyTransform(transform);
		return mpgxResult;
	}

	GraphicsMesh mesh;

	mpgxResult = createGraphicsMesh(
		window,
		UINT16_INDEX_TYPE,
		sizeof(cubeTriangleIndices) / sizeof(uint16_t),
		0,
		vertexBuffer,
		indexBuffer,
		&mesh);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyBuffer(indexBuffer);
		destroyBuffer(vertexBuffer);
		destroyTransform(transform);
		return mpgxResult;
	}

	Box3F bounding = posExtBox3F(
		zeroVec3F,
		oneVec3F);
	GraphicsRender render = createDiffuseRender(
		diffuseRenderer,
		transform,
		bounding,
		mesh);

	if (!render)
	{
		destroyGraphicsMesh(
			mesh,
			true);
		destroyTransform(transform);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	*diffuseRender = render;
	return SUCCESS_MPGX_RESULT;
}
inline static void destroyDiffuseRenderInstance(
	GraphicsRender diffuseRender)
{
	if (!diffuseRender)
		return;

	GraphicsMesh graphicsMesh =
		getDiffuseRenderMesh(diffuseRender);
	destroyGraphicsRender(
		diffuseRender,
		true);
	destroyGraphicsMesh(
		graphicsMesh,
		true);
}

inline static Client createClient()
{
	Client client = malloc(
		sizeof(Client_T));

	if (!client)
		return NULL;

	MpgxResult mpgxResult = initializeGraphics(
		APPLICATION_NAME,
		MPGX_VERSION_MAJOR,
		MPGX_VERSION_MINOR,
		MPGX_VERSION_PATCH);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		printf("MPGX Error: %s.",
			mpgxResultToString(mpgxResult));
		free(client);
		return NULL;
	}

	Window window;

	mpgxResult = createAnyWindow(
		defaultWindowSize,
		APPLICATION_NAME,
		onWindowUpdate,
		client,
		true,
		false,
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
		MPGX_DEFAULT_CAPACITY,
		NULL);

	if (!transformer)
	{
		destroyWindow(window);
		free(client);
		return NULL;
	}

	FreeCamera freeCamera = createDefaultFreeCamera(
		getWindowFramebuffer(window),
		transformer);

	if (!freeCamera)
	{
		destroyTransformer(transformer);
		destroyWindow(window);
		free(client);
		return NULL;
	}

	GraphicsRenderer diffuseRenderer;

	mpgxResult = createDiffuseRendererInstance(
		window,
		&diffuseRenderer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		printf("MPGX Error: %s.",
			mpgxResultToString(mpgxResult));
		destroyFreeCamera(freeCamera);
		destroyTransformer(transformer);
		destroyWindow(window);
		free(client);
		return NULL;
	}

	GraphicsRender diffuseRender;

	mpgxResult = createDiffuseRenderInstance(
		window,
		transformer,
		diffuseRenderer,
		&diffuseRender);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		printf("MPGX Error: %s.",
			mpgxResultToString(mpgxResult));
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
	if (!client)
		return;

	destroyDiffuseRenderInstance(client->diffuseRender);
	destroyDiffuseRendererInstance(client->diffuseRenderer);
	destroyFreeCamera(client->freeCamera);
	destroyTransformer(client->transformer);
	destroyWindow(client->window);
	terminateGraphics();
	free(client);
}

inline static void updateClient(Client client)
{
	updateWindow(client->window);
}

int main()
{
	Client client = createClient();

	if (!client)
		return EXIT_FAILURE;

	updateClient(client);
	destroyClient(client);
	return EXIT_SUCCESS;
}
