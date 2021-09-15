#include "mpgx/free_camera.h"
#include "mpgx/renderers/diffuse_renderer.h"
#include "mpgx/primitives/cube_primitive.h"

#include "cmmt/angle.h"
#include <string.h>

// Camera fly: WASD + RMB

typedef struct Client
{
	Window window;
	Transformer transformer;
	FreeCamera freeCamera;
	RenderData renderData;
	Renderer diffuseRenderer;
	Render diffuseRender;
} Client;

static void onWindowUpdate(void* handle)
{
	Client* client = (Client*)handle;
	Window window = client->window;
	FreeCamera freeCamera = client->freeCamera;

	updateFreeCamera(freeCamera);
	updateTransformer(client->transformer);

	Transform cameraTransform =
		getFreeCameraTransform(freeCamera);
	RenderData* renderData = &client->renderData;

	createRenderData(
		window,
		getTransformModel(cameraTransform),
		getFreeCamera(freeCamera),
		renderData,
		true);

	beginWindowRender(window);

	clearFramebuffer(
		window,
		true,
		true,
		false,
		zeroVec4F());

	drawRenderer(
		client->diffuseRenderer,
		renderData);

	endWindowRender(window);
}

inline static Renderer createDiffuseRendererInstance(
	Window window,
	Transform cameraTransform)
{
	const char* vertexShaderPath;
	const char* fragmentShaderPath;

	uint8_t api = getWindowGraphicsAPI(window);

	if (api == VULKAN_GRAPHICS_API)
	{
		// TODO:
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		vertexShaderPath = "resources/shaders/opengl/diffuse.vert";
		fragmentShaderPath = "resources/shaders/opengl/diffuse.frag";
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

	Pipeline pipeline = createDiffusePipeline(
		window,
		vertexShader,
		fragmentShader,
		TRIANGLES_DRAW_MODE);

	if (pipeline == NULL)
	{
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	Renderer renderer = createDiffuseRenderer(
		cameraTransform,
		pipeline,
		ASCENDING_RENDER_SORTING,
		true,
		1);

	if (renderer == NULL)
	{
		destroyPipeline(pipeline);
		destroyShader(fragmentShader);
		destroyShader(vertexShader);
		return NULL;
	}

	return renderer;
}
inline static void destroyDiffuseRendererInstance(
	Renderer diffuseRenderer)
{
	if (diffuseRenderer == NULL)
		return;

	Pipeline pipeline = getRendererPipeline(diffuseRenderer);
	Shader fragmentShader = getDiffusePipelineFragmentShader(pipeline);
	Shader vertexShader = getDiffusePipelineVertexShader(pipeline);

	destroyPipeline(pipeline);
	destroyShader(fragmentShader);
	destroyShader(vertexShader);
	destroyRenderer(diffuseRenderer);
}

inline static Render createDiffuseRenderInstance(
	Window window,
	Transformer transformer,
	Renderer diffuseRenderer)
{
	Vec3F position = vec3F(0.0f, 0.0f, 4.0f);

	Transform transform = createTransform(
		transformer,
		position,
		oneVec3F(),
		oneQuat(),
		SPIN_ROTATION_TYPE,
		NULL,
		true);

	if (transform == NULL)
		return NULL;

	Buffer vertexBuffer = createBuffer(
		window,
		VERTEX_BUFFER_TYPE,
		cubeVertNorm,
		sizeof(cubeVertNorm),
		true);

	if (vertexBuffer == NULL)
	{
		destroyTransform(transform);
		return NULL;
	}

	Buffer indexBuffer = createBuffer(
		window,
		INDEX_BUFFER_TYPE,
		cubeInd,
		sizeof(cubeInd),
		true);

	if (indexBuffer == NULL)
	{
		destroyBuffer(vertexBuffer);
		destroyTransform(transform);
		return NULL;
	}

	Mesh mesh = createMesh(
		window,
		UINT16_DRAW_INDEX,
		sizeof(cubeInd) / sizeof(uint16_t),
		0,
		vertexBuffer,
		indexBuffer);

	if (mesh == NULL)
	{
		destroyBuffer(indexBuffer);
		destroyBuffer(vertexBuffer);
		destroyTransform(transform);
		return NULL;
	}

	Box3F bounding = posExtBox3F(
		zeroVec3F(),
		oneVec3F());
	Render render = createDiffuseRender(
		diffuseRenderer,
		transform,
		bounding,
		mesh);

	if (render == NULL)
	{
		destroyMesh(mesh);
		destroyBuffer(indexBuffer);
		destroyBuffer(vertexBuffer);
		destroyTransform(transform);
		return NULL;
	}

	return render;
}
inline static void destroyDiffuseRenderInstance(
	Render diffuseRender)
{
	if (diffuseRender == NULL)
		return;

	Mesh mesh = getDiffuseRenderMesh(diffuseRender);
	Buffer indexBuffer = getMeshIndexBuffer(mesh);
	Buffer vertexBuffer = getMeshVertexBuffer(mesh);

	destroyMesh(mesh);
	destroyBuffer(indexBuffer);
	destroyBuffer(vertexBuffer);
	destroyRender(diffuseRender);
}

inline static Client* createClient()
{
	Client* client = malloc(
		sizeof(Client));

	if (client == NULL)
		return NULL;

	Window window = createAnyWindow(
		defaultWindowSize,
		"MPGX Window Example",
		onWindowUpdate,
		client,
		false,
		1,
		1,
		1,
		1,
		1,
		1,
		1);

	if (window == NULL)
	{
		free(client);
		return NULL;
	}

	Transformer transformer = createTransformer(1);

	if (transformer == NULL)
	{
		destroyWindow(window);
		free(client);
		return NULL;
	}

	FreeCamera freeCamera = createFreeCamera(
		window,
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

	Transform cameraTransform = getFreeCameraTransform(freeCamera);

	Renderer diffuseRenderer = createDiffuseRendererInstance(
		window,
		cameraTransform);

	if (diffuseRenderer == NULL)
	{
		destroyFreeCamera(freeCamera);
		destroyTransformer(transformer);
		destroyWindow(window);
		free(client);
		return NULL;
	}

	Render diffuseRender = createDiffuseRenderInstance(
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

	memset(
		&client->renderData,
		0,
		sizeof(RenderData));

	showWindow(window);
	return client;
}
inline static void destroyClient(Client* client)
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

inline static void updateClient(Client* client)
{
	updateWindow(client->window);
}

int main()
{
	if (initializeGraphics() == false)
		return EXIT_FAILURE;

	Client* client = createClient();

	if (client == NULL)
		return EXIT_FAILURE;

	updateClient(client);
	destroyClient(client);

	terminateGraphics();
	return EXIT_SUCCESS;
}