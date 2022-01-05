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

// !!!!!!!!!!!!!!!!
// NOT YET WORKING
// !!!!!!!!!!!!!!!!

#include "mpgx/defines.h"
#include "mpgx/free_camera.h"
#include "mpgx/primitives/cube_primitive.h"
#include "mpgx/pipelines/ray_tracing_color_pipeline.h"

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
	RayTracingScene rayTracingScene;
	RayTracingPipeline colorPipeline;
} Client_T;

typedef Client_T* Client;

static void onWindowUpdate(void* handle)
{
	Client client = (Client)handle;
	Window window = client->window;
	FreeCamera freeCamera = client->freeCamera;

	updateFreeCamera(freeCamera);
	updateTransformer(client->transformer);

	MpgxResult mpgxResult = beginWindowRecord(window);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
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
	setRayTracingColorPipelineInvProj(
		client->colorPipeline,
		invProj);

	Mat4F invView = invMat4F(getTransformModel(
		getFreeCameraTransform(freeCamera)));
	setRayTracingColorPipelineInvView(
		client->colorPipeline,
		invView);

	bindRayTracingPipeline(client->colorPipeline);
	traceRayTracingPipeline(client->colorPipeline);

	beginFramebufferRender(
		framebuffer,
		clearValues,
		2);

	endFramebufferRender(framebuffer);
	endWindowRecord(window);
}

inline static MpgxResult createRayTracingSceneInstance(
	Window window,
	RayTracingScene* rayTracingScene)
{
	Buffer vertexBuffer;

	MpgxResult mpgxResult = createBuffer(
		window,
		VERTEX_BUFFER_TYPE,
		GPU_ONLY_BUFFER_USAGE,
		NO_BUFFER_FLAG,
		cubeTriangleVertices,
		sizeof(cubeTriangleVertices),
		&vertexBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	Buffer indexBuffer;

	mpgxResult = createBuffer(
		window,
		INDEX_BUFFER_TYPE,
		GPU_ONLY_BUFFER_USAGE,
		NO_BUFFER_FLAG,
		cubeTriangleIndices,
		sizeof(cubeTriangleIndices),
		&indexBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyBuffer(vertexBuffer);
		return mpgxResult;
	}

	RayTracingMesh mesh;

	mpgxResult = createRayTracingMesh(
		window,
		sizeof(Vec3F),
		UINT16_INDEX_TYPE,
		vertexBuffer,
		indexBuffer,
		&mesh);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyBuffer(indexBuffer);
		destroyBuffer(vertexBuffer);
		return mpgxResult;
	}

	RayTracingScene scene;

	mpgxResult = createRayTracingScene(
		window,
		&mesh,
		1,
		&scene);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyRayTracingMesh(mesh, true);
		return mpgxResult;
	}

	*rayTracingScene = scene;
	return SUCCESS_MPGX_RESULT;
}
inline static void destroyRayTracingSceneInstance(
	RayTracingScene rayTracingScene)
{
	if (rayTracingScene == NULL)
		return;

	RayTracingMesh rayTracingMesh = getRayTracingSceneMeshes(
		rayTracingScene)[0];
	destroyRayTracingScene(rayTracingScene);
	destroyRayTracingMesh(rayTracingMesh, true);
}

inline static MpgxResult createRayTracingColorPipelineInstance(
	Window window,
	RayTracingScene rayTracingScene,
	RayTracingPipeline* rayTracingPipeline)
{
	const char* generationShaderPath = "resources/shaders/vulkan/color.rgen.spv";
	const char* missShaderPath = "resources/shaders/vulkan/color.rmiss.spv";
	const char* closestHitShaderPath = "resources/shaders/vulkan/color.rchit.spv";

	Shader generationShader;

	MpgxResult mpgxResult = createShaderFromFile(
		window,
		RAY_GENERATION_SHADER_TYPE,
		generationShaderPath,
		&generationShader);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	Shader missShader;

	mpgxResult = createShaderFromFile(
		window,
		RAY_MISS_SHADER_TYPE,
		missShaderPath,
		&missShader);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyShader(generationShader);
		return mpgxResult;
	}

	Shader closestHitShader;

	mpgxResult = createShaderFromFile(
		window,
		RAY_CLOSEST_HIT_SHADER_TYPE,
		closestHitShaderPath,
		&closestHitShader);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyShader(missShader);
		destroyShader(generationShader);
		return mpgxResult;
	}

	RayTracingPipeline pipeline;

	mpgxResult = createRayTracingColorPipeline(
		window,
		generationShader,
		missShader,
		closestHitShader,
		rayTracingScene,
		&pipeline);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyShader(closestHitShader);
		destroyShader(missShader);
		destroyShader(generationShader);
		return mpgxResult;
	}

	*rayTracingPipeline = pipeline;
	return SUCCESS_MPGX_RESULT;
}
inline static void destroyRayTracingColorPipelineInstance(
	RayTracingPipeline colorPipeline)
{
	if (colorPipeline == NULL)
		return;

	destroyRayTracingPipeline(
		colorPipeline,
		true);
}

inline static Client createClient()
{
	Client client = malloc(
		sizeof(Client_T));

	if (client == NULL)
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

	mpgxResult = createWindow(
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

	RayTracingScene rayTracingScene;

	mpgxResult = createRayTracingSceneInstance(
		window,
		&rayTracingScene);

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

	RayTracingPipeline colorPipeline;

	mpgxResult = createRayTracingColorPipelineInstance(
		window,
		rayTracingScene,
		&colorPipeline);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		printf("MPGX Error: %s.",
			mpgxResultToString(mpgxResult));
		destroyRayTracingSceneInstance(rayTracingScene);
		destroyFreeCamera(freeCamera);
		destroyTransformer(transformer);
		destroyWindow(window);
		free(client);
		return NULL;
	}

	client->window = window;
	client->transformer = transformer;
	client->freeCamera = freeCamera;
	client->rayTracingScene = rayTracingScene;
	client->colorPipeline = colorPipeline;

	showWindow(window);
	return client;
}
inline static void destroyClient(Client client)
{
	if (client == NULL)
		return;

	destroyRayTracingColorPipelineInstance(client->colorPipeline);
	destroyRayTracingSceneInstance(client->rayTracingScene);
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

	if (client == NULL)
		return EXIT_FAILURE;

	updateClient(client);
	destroyClient(client);
	return EXIT_SUCCESS;
}
