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

// !!!!!!!!!!!!!!!!
// NOT YET WORKING
// !!!!!!!!!!!!!!!!

#include "mpgx/defines.h"
#include "mpgx/free_camera.h"
#include "mpgx/primitives/cube_primitive.h"
#include "mpgx/pipelines/ray_color_pipeline.h"

#include "cmmt/angle.h"

#include <stdio.h>
#include <string.h>

#define APPLICATION_NAME "MPGX - Ray Tracing Example"

// Camera fly: WASD + RMB

typedef struct Client_T
{
	Window window;
	Transformer transformer;
	FreeCamera freeCamera;
	RayScene rayScene;
	RayPipeline colorPipeline;
} Client_T;

typedef Client_T* Client;

static void onWindowUpdate(void* handle)
{
	Client client = (Client)handle;
	Window window = client->window;
	FreeCamera freeCamera = client->freeCamera;

	updateFreeCamera(freeCamera);
	updateTransformer(client->transformer);

	bool result = beginWindowRecord(window);

	if (result == false)
		return;

	Framebuffer framebuffer =
		getWindowFramebuffer(window);

	FramebufferClear clearValues[2];
	clearValues[0].color = zeroLinearColor;
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	Camera camera = getFreeCamera(freeCamera);
	Mat4F invProj = invMat4F(perspZeroOneMat4F(
		camera.persp.fieldOfView,
		camera.persp.aspectRatio,
		camera.persp.nearClipPlane,
		camera.persp.farClipPlane));
	setColorRayPipelineInvProj(
		client->colorPipeline,
		invProj);

	Mat4F invView = invMat4F(getTransformModel(
		getFreeCameraTransform(freeCamera)));
	setColorRayPipelineInvView(
		client->colorPipeline,
		invView);

	bindRayPipeline(client->colorPipeline);
	tracePipelineRays(client->colorPipeline);

	beginFramebufferRender(
		framebuffer,
		clearValues,
		2);

	endFramebufferRender(framebuffer);

	endWindowRecord(window);
}

inline static RayScene createRaySceneInstance(
	Window window)
{
	Buffer vertexBuffer = createBuffer(
		window,
		VERTEX_BUFFER_TYPE,
		cubeTriangleVertices,
		sizeof(cubeTriangleVertices),
		true);

	if (vertexBuffer == NULL)
		return NULL;

	Buffer indexBuffer = createBuffer(
		window,
		INDEX_BUFFER_TYPE,
		cubeTriangleIndices,
		sizeof(cubeTriangleIndices),
		true);

	if (indexBuffer == NULL)
	{
		destroyBuffer(vertexBuffer);
		return NULL;
	}

	RayMesh rayMesh = createRayMesh(
		window,
		sizeof(Vec3F),
		UINT16_INDEX_TYPE,
		vertexBuffer,
		indexBuffer);

	if (rayMesh == NULL)
	{
		destroyBuffer(indexBuffer);
		destroyBuffer(vertexBuffer);
		return NULL;
	}

	RayScene rayScene = createRayScene(
		window,
		&rayMesh,
		1);

	if (rayScene == NULL)
	{
		destroyRayMesh(rayMesh, true);
		return NULL;
	}

	return rayScene;
}
inline static void destroyRaySceneInstance(
	RayScene rayScene)
{
	if (rayScene == NULL)
		return;

	RayMesh rayMesh = getRaySceneMeshes(rayScene)[0];
	destroyRayScene(rayScene);
	destroyRayMesh(rayMesh, true);
}

inline static RayPipeline createRayColorPipelineInstance(
	Window window,
	RayScene rayScene)
{
	const char* generationShaderPath = "resources/shaders/vulkan/color.rgen.spv";
	const char* missShaderPath = "resources/shaders/vulkan/color.rmiss.spv";
	const char* closestHitShaderPath = "resources/shaders/vulkan/color.rchit.spv";

	Shader generationShader = createShaderFromFile(
		window,
		RAY_GENERATION_SHADER_TYPE,
		generationShaderPath);

	if (generationShader == NULL)
		return NULL;

	Shader missShader = createShaderFromFile(
		window,
		RAY_MISS_SHADER_TYPE,
		missShaderPath);

	if (missShader == NULL)
	{
		destroyShader(generationShader);
		return NULL;
	}

	Shader closestHitShader = createShaderFromFile(
		window,
		RAY_CLOSEST_HIT_SHADER_TYPE,
		closestHitShaderPath);

	if (closestHitShader == NULL)
	{
		destroyShader(missShader);
		destroyShader(generationShader);
		return NULL;
	}

	RayPipeline rayPipeline = createRayColorPipeline(
		window,
		generationShader,
		missShader,
		closestHitShader,
		rayScene);

	if (rayPipeline == NULL)
		return NULL;

	return rayPipeline;
}
inline static void destroyRayColorPipelineInstance(
	RayPipeline rayPipeline)
{
	if (rayPipeline == NULL)
		return;

	destroyRayPipeline(rayPipeline, true);
}

inline static Client createClient()
{
	Client client = malloc(
		sizeof(Client_T));

	if (client == NULL)
		return NULL;

	Window window;

	MpgxResult mpgxResult = createWindow(
		VULKAN_GRAPHICS_API,
		defaultWindowSize,
		APPLICATION_NAME,
		onWindowUpdate,
		client,
		false,
		true,
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

	RayScene rayScene = createRaySceneInstance(window);

	if (rayScene == NULL)
	{
		destroyFreeCamera(freeCamera);
		destroyTransformer(transformer);
		destroyWindow(window);
		free(client);
		return NULL;
	}

	RayPipeline colorPipeline = createRayColorPipelineInstance(
		window,
		rayScene);

	if (colorPipeline == NULL)
	{
		destroyRaySceneInstance(rayScene);
		destroyFreeCamera(freeCamera);
		destroyTransformer(transformer);
		destroyWindow(window);
		free(client);
		return NULL;
	}

	client->window = window;
	client->transformer = transformer;
	client->freeCamera = freeCamera;
	client->rayScene = rayScene;
	client->colorPipeline = colorPipeline;

	showWindow(window);
	return client;
}
inline static void destroyClient(Client client)
{
	if (client == NULL)
		return;

	destroyRayColorPipelineInstance(client->colorPipeline);
	destroyRaySceneInstance(client->rayScene);
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
	bool result = initializeGraphics(
		APPLICATION_NAME,
		MPGX_VERSION_MAJOR,
		MPGX_VERSION_MINOR,
		MPGX_VERSION_PATCH);

	if (result == false)
		return EXIT_FAILURE;

	Client client = createClient();

	if (client == NULL)
		return EXIT_FAILURE;

	updateClient(client);
	destroyClient(client);

	terminateGraphics();
	return EXIT_SUCCESS;
}
