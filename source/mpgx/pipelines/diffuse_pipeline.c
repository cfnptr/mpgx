#include "mpgx/pipelines/diffuse_pipeline.h"
#include "mpgx/_source/pipeline.h"
#include "mpgx/_source/buffer.h"

#include <string.h>

// TODO: recreate buffers on framebuffer resize

typedef struct UniformBuffer
{
	Vec4F objectColor;
	Vec4F ambientColor;
	Vec4F lightColor;
	Vec4F lightDirection;
} UniformBuffer;
typedef struct VkPipelineHandle
{
	Mat4F mvp;
	Mat4F normal;
	UniformBuffer u;
#if MPGX_SUPPORT_VULKAN
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	Buffer* uniformBuffers;
	VkDescriptorSet* descriptorSets;
	uint32_t bufferCount;
#endif
} VkPipelineHandle;
typedef struct GlPipelineHandle
{
	Mat4F mvp;
	Mat4F normal;
	UniformBuffer u;
	GLint mvpLocation;
	GLint normalLocation;
	Buffer uniformBuffer;
} GlPipelineHandle;
typedef union PipelineHandle
{
	VkPipelineHandle vk;
	GlPipelineHandle gl;
} PipelineHandle;

#if MPGX_SUPPORT_VULKAN
inline static VkDescriptorSetLayout createVkDescriptorSetLayout(
	VkDevice device)
{
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {
		0,
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		1,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		NULL,
	};
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		NULL,
		0,
		1,
		&descriptorSetLayoutBinding,
	};

	VkDescriptorSetLayout descriptorSetLayout;

	VkResult result = vkCreateDescriptorSetLayout(
		device,
		&descriptorSetLayoutCreateInfo,
		NULL,
		&descriptorSetLayout);

	if(result != VK_SUCCESS)
		return NULL;

	return descriptorSetLayout;
}
inline static VkDescriptorPool createVkDescriptorPool(
	VkDevice device,
	uint32_t bufferCount)
{
	VkDescriptorPoolSize descriptorPoolSize = {
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		bufferCount,
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		0,
		bufferCount,
		1,
		&descriptorPoolSize,
	};

	VkDescriptorPool descriptorPool;

	VkResult result = vkCreateDescriptorPool(
		device,
		&descriptorPoolCreateInfo,
		NULL,
		&descriptorPool);

	if (result != VK_SUCCESS)
		return NULL;

	return descriptorPool;
}
inline static Buffer* createVkUniformBuffers(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandPool transferCommandPool,
	Window window,
	uint32_t bufferCount)
{
	Buffer* buffers = malloc(
		bufferCount * sizeof(Buffer));

	if (buffers == NULL)
		return NULL;

	for (uint32_t i = 0; i < bufferCount; i++)
	{
		Buffer buffer = createVkBuffer(
			device,
			allocator,
			transferQueue,
			transferCommandPool,
			0,
			window,
			UNIFORM_BUFFER_TYPE,
			NULL,
			sizeof(UniformBuffer),
			false);

		if (buffer == NULL)
		{
			for (uint32_t j = 0; j < i; j++)
			{
				destroyVkBuffer(
					allocator,
					buffers[j]);
			}

			free(buffers);
			return NULL;
		}

		buffers[i] = buffer;
	}

	return buffers;
}
inline static void destroyVkUniformBuffers(
	VmaAllocator allocator,
	uint32_t bufferCount,
	Buffer* uniformBuffers)
{
	for (uint32_t i = 0; i < bufferCount; i++)
	{
		destroyVkBuffer(
			allocator,
			uniformBuffers[i]);
	}

	free(uniformBuffers);
}
inline static VkDescriptorSet* createVkDescriptorSets(
	VkDevice device,
	VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	uint32_t bufferCount,
	Buffer* uniformBuffers)
{
	VkDescriptorSetLayout* descriptorSetLayouts = malloc(
		bufferCount * sizeof(VkDescriptorSetLayout));

	if (descriptorSetLayouts == NULL)
		return NULL;

	for (uint32_t i = 0; i < bufferCount; i++)
		descriptorSetLayouts[i] = descriptorSetLayout;

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo ={
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		descriptorPool,
		bufferCount,
		descriptorSetLayouts,
	};

	VkDescriptorSet* descriptorSets = malloc(
		bufferCount * sizeof(VkDescriptorSet));

	if (descriptorSets == NULL)
	{
		free(descriptorSetLayouts);
		return NULL;
	}

	VkResult result = vkAllocateDescriptorSets(
		device,
		&descriptorSetAllocateInfo,
		descriptorSets);

	free(descriptorSetLayouts);

	if (result != VK_SUCCESS)
	{
		free(descriptorSets);
		return NULL;
	}

	for (uint32_t i = 0; i < bufferCount; i++)
	{
		VkDescriptorBufferInfo descriptorBufferInfo = {
			uniformBuffers[i]->vk.handle,
			0,
			sizeof(UniformBuffer),
		};
		VkWriteDescriptorSet writeDescriptorSet = {
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			NULL,
			descriptorSets[i],
			0,
			0,
			1,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			NULL,
			&descriptorBufferInfo,
			NULL,
		};

		vkUpdateDescriptorSets(
			device,
			1,
			&writeDescriptorSet,
			0,
			NULL);
	}

	return descriptorSets;
}

static void onVkPipelineHandleDestroy(
	Window window,
	void* handle)
{
	PipelineHandle* pipelineHandle = (PipelineHandle*)handle;
	VkWindow vkWindow = getVkWindow(window);
	VkDevice device = vkWindow->device;

	free(pipelineHandle->vk.descriptorSets);
	destroyVkUniformBuffers(
		vkWindow->allocator,
		pipelineHandle->vk.bufferCount,
		pipelineHandle->vk.uniformBuffers);
	vkDestroyDescriptorPool(
		device,
		pipelineHandle->vk.descriptorPool,
		NULL);
	vkDestroyDescriptorSetLayout(
		device,
		pipelineHandle->vk.descriptorSetLayout,
		NULL);
	free(pipelineHandle);
}
static void onVkPipelineHandleBind(Pipeline pipeline)
{
	PipelineHandle* handle = pipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(pipeline->vk.window);
	uint32_t bufferIndex = vkWindow->bufferIndex;
	Buffer buffer = handle->vk.uniformBuffers[bufferIndex];

	setVkBufferData(
		vkWindow->allocator,
		buffer->vk.allocation,
		&handle->vk.u,
		sizeof(UniformBuffer),
		0);
	vkCmdBindDescriptorSets(
		vkWindow->currenCommandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline->vk.layout,
		0,
		1,
		&handle->vk.descriptorSets[bufferIndex],
		0,
		NULL);
}
static void onVkPipelineUniformsSet(Pipeline pipeline)
{
	PipelineHandle* handle = pipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(pipeline->vk.window);
	VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;

	vkCmdPushConstants(
		commandBuffer,
		pipeline->vk.layout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(Mat4F),
		&handle->vk.mvp);
	vkCmdPushConstants(
		commandBuffer,
		pipeline->vk.layout,
		VK_SHADER_STAGE_VERTEX_BIT,
		sizeof(Mat4F),
		sizeof(Mat4F),
		&handle->vk.normal);
}
inline static Pipeline createVkPipelineHandle(
	Window window,
	Shader vertexShader,
	Shader fragmentShader)
{
	PipelineHandle* handle = malloc(
		sizeof(PipelineHandle));

	if (handle == NULL)
		return NULL;

	Vec2U framebufferSize =
		getWindowFramebufferSize(window);

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	VkWindow vkWindow = getVkWindow(window);
	VkDevice device = vkWindow->device;

	VkDescriptorSetLayout descriptorSetLayout =
		createVkDescriptorSetLayout(device);

	if (descriptorSetLayout == NULL)
	{
		free(handle);
		return NULL;
	}

	VkVertexInputBindingDescription vertexInputBindingDescription = {
		0,
		sizeof(Vec3F) * 2,
		VK_VERTEX_INPUT_RATE_VERTEX,
	};
	VkVertexInputAttributeDescription vertexInputAttributeDescriptions[2] = {
		{
			0,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			0,
		},
		{
			1,
			0,
			VK_FORMAT_R32G32B32_SFLOAT,
			sizeof(Vec3F),
		},
	};
	VkPushConstantRange pushConstantRange = {
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(Mat4F) * 2,
	};

	VkPipelineCreateInfo createInfo = {
		1,
		&vertexInputBindingDescription,
		2,
		vertexInputAttributeDescriptions,
		1,
		&descriptorSetLayout,
		1,
		&pushConstantRange,
	};
	Pipeline pipeline = createPipeline(
		window,
		DIFFUSE_PIPELINE_NAME,
		shaders,
		2,
		TRIANGLE_LIST_DRAW_MODE,
		FILL_POLYGON_MODE,
		BACK_CULL_MODE,
		LESS_COMPARE_OPERATION,
		true,
		true,
		true,
		true,
		false,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		vec4U(0, 0,
			framebufferSize.x,
			framebufferSize.y),
		vec2F(
			DEFAULT_MIN_DEPTH_RANGE,
			DEFAULT_MAX_DEPTH_RANGE),
		vec4U(0, 0,
			framebufferSize.x,
			framebufferSize.y),
		onVkPipelineHandleDestroy,
		onVkPipelineHandleBind,
		onVkPipelineUniformsSet,
		handle,
		&createInfo);

	if (pipeline == NULL)
	{
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			NULL);
		free(handle);
		return NULL;
	}

	uint32_t bufferCount = vkWindow->swapchain->bufferCount;

	VkDescriptorPool descriptorPool = createVkDescriptorPool(
		device,
		bufferCount);

	if (descriptorPool == NULL)
	{
		destroyPipeline(
			pipeline,
			false);
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			NULL);
		free(handle);
		return NULL;
	}

	Buffer* uniformBuffers = createVkUniformBuffers(
		device,
		vkWindow->allocator,
		vkWindow->graphicsQueue,
		vkWindow->transferCommandPool,
		window,
		bufferCount);

	if (uniformBuffers == NULL)
	{
		vkDestroyDescriptorPool(
			device,
			descriptorPool,
			NULL);
		destroyPipeline(
			pipeline,
			false);
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			NULL);
		free(handle);
		return NULL;
	}

	VkDescriptorSet* descriptorSets = createVkDescriptorSets(
		device,
		descriptorSetLayout,
		descriptorPool,
		bufferCount,
		uniformBuffers);

	if (descriptorSets == NULL)
	{
		destroyVkUniformBuffers(
			vkWindow->allocator,
			bufferCount,
			uniformBuffers);
		vkDestroyDescriptorPool(
			device,
			descriptorPool,
			NULL);
		destroyPipeline(
			pipeline,
			false);
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			NULL);
		free(handle);
		return NULL;
	}

	Vec3F lightDirection = normVec3F(
		vec3F(1.0f, -3.0f, 6.0f));

	handle->vk.mvp = identMat4F();
	handle->vk.normal = identMat4F();
	handle->vk.u.objectColor = oneVec4F();
	handle->vk.u.ambientColor = valVec4F(0.5f);
	handle->vk.u.lightColor = oneVec4F();
	handle->vk.u.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
	handle->vk.descriptorSetLayout = descriptorSetLayout;
	handle->vk.descriptorPool = descriptorPool;
	handle->vk.uniformBuffers = uniformBuffers;
	handle->vk.descriptorSets = descriptorSets;
	handle->vk.bufferCount = bufferCount;
	return pipeline;
}
#endif

static void onGlPipelineHandleDestroy(
	Window window,
	void* handle)
{
	PipelineHandle* pipelineHandle =
		(PipelineHandle*)handle;
	destroyGlBuffer(
		pipelineHandle->gl.uniformBuffer);
	free(pipelineHandle);
}
static void onGlPipelineHandleBind(Pipeline pipeline)
{
	PipelineHandle* handle = pipeline->gl.handle;
	Buffer uniformBuffer = handle->gl.uniformBuffer;

	setGlBufferData(
		uniformBuffer->gl.glType,
		uniformBuffer->gl.handle,
		&handle->gl.u,
		sizeof(UniformBuffer),
		0);

	glBindBufferBase(
		GL_UNIFORM_BUFFER,
		0,
		uniformBuffer->gl.handle);
	assertOpenGL();
}
static void onGlPipelineUniformsSet(Pipeline pipeline)
{
	PipelineHandle* handle = pipeline->gl.handle;

	glUniformMatrix4fv(
		handle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&handle->gl.mvp);
	glUniformMatrix4fv(
		handle->gl.normalLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&handle->gl.normal);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F) * 2,
		0);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F) * 2,
		(const void*)sizeof(Vec3F));

	assertOpenGL();
}
inline static Pipeline createGlPipelineHandle(
	Window window,
	Shader vertexShader,
	Shader fragmentShader)
{
	PipelineHandle* handle = malloc(
		sizeof(PipelineHandle));

	if (handle == NULL)
		return NULL;

	Vec2U framebufferSize =
		getWindowFramebufferSize(window);

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	Pipeline pipeline = createPipeline(
		window,
		DIFFUSE_PIPELINE_NAME,
		shaders,
		2,
		TRIANGLE_LIST_DRAW_MODE,
		FILL_POLYGON_MODE,
		BACK_CULL_MODE,
		LESS_COMPARE_OPERATION,
		true,
		true,
		true,
		true,
		false,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		vec4U(0, 0,
			framebufferSize.x,
			framebufferSize.y),
		vec2F(
			DEFAULT_MIN_DEPTH_RANGE,
			DEFAULT_MAX_DEPTH_RANGE),
		zeroVec4U(),
		onGlPipelineHandleDestroy,
		onGlPipelineHandleBind,
		onGlPipelineUniformsSet,
		handle,
		NULL);

	if (pipeline == NULL)
	{
		free(handle);
		return NULL;
	}

	GLuint glHandle = pipeline->gl.glHandle;

	GLint mvpLocation = getGlUniformLocation(
		glHandle,
		"u_MVP");

	if (mvpLocation == GL_NULL_UNIFORM_LOCATION)
	{
		destroyPipeline(
			pipeline,
			false);
		free(handle);
		return NULL;
	}

	GLint normalLocation = getGlUniformLocation(
		glHandle,
		"u_Normal");

	if (normalLocation == GL_NULL_UNIFORM_LOCATION)
	{
		destroyPipeline(
			pipeline,
			false);
		free(handle);
		return NULL;
	}

	GLuint uniformBlockIndex = getGlUniformBlockIndex(
		glHandle,
		"UniformBuffer");

	if (uniformBlockIndex == GL_INVALID_INDEX)
	{
		destroyPipeline(
			pipeline,
			false);
		free(handle);
		return NULL;
	}

	glUniformBlockBinding(
		glHandle,
		uniformBlockIndex,
		0);

	assertOpenGL();

	Buffer uniformBuffer = createGlBuffer(
		window,
		UNIFORM_BUFFER_TYPE,
		NULL,
		sizeof(UniformBuffer),
		false);

	if (uniformBuffer == NULL)
	{
		destroyPipeline(
			pipeline,
			false);
		free(handle);
		return NULL;
	}

	Vec3F lightDirection = normVec3F(
		vec3F(1.0f, -3.0f, 6.0f));

	handle->gl.mvp = identMat4F();
	handle->gl.normal = identMat4F();
	handle->gl.u.objectColor = oneVec4F();
	handle->gl.u.ambientColor = valVec4F(0.5f);
	handle->gl.u.lightColor = oneVec4F();
	handle->gl.u.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
	handle->gl.mvpLocation = mvpLocation;
	handle->gl.normalLocation = normalLocation;
	handle->gl.uniformBuffer = uniformBuffer;
	return pipeline;
}

Pipeline createDiffusePipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(getShaderType(vertexShader) == VERTEX_SHADER_TYPE);
	assert(getShaderType(fragmentShader) == FRAGMENT_SHADER_TYPE);
	assert(getShaderWindow(vertexShader) == window);
	assert(getShaderWindow(fragmentShader) == window);

	uint8_t api = getWindowGraphicsAPI(window);

	Pipeline pipeline;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		pipeline = createVkPipelineHandle(
			window,
			vertexShader,
			fragmentShader);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		pipeline = createGlPipelineHandle(
			window,
			vertexShader,
			fragmentShader);
	}
	else
	{
		return NULL;
	}

	if (pipeline == NULL)
		return NULL;

	return pipeline;
}

Mat4F getDiffusePipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.mvp;
}
void setDiffusePipelineMvp(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.mvp = mvp;
}

Mat4F getDiffusePipelineNormal(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.normal;
}
void setDiffusePipelineNormal(
	Pipeline pipeline,
	Mat4F normal)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.normal = normal;
}

Vec4F getDiffusePipelineObjectColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.u.objectColor;
}
void setDiffusePipelineObjectColor(
	Pipeline pipeline,
	Vec4F objectColor)
{
	assert(pipeline != NULL);
	assert(objectColor.x >= 0.0f &&
		objectColor.y >= 0.0f &&
		objectColor.z >= 0.0f &&
		objectColor.w >= 0.0f);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.u.objectColor = objectColor;
}

Vec4F getDiffusePipelineAmbientColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.u.ambientColor;
}
void setDiffusePipelineAmbientColor(
	Pipeline pipeline,
	Vec4F ambientColor)
{
	assert(pipeline != NULL);
	assert(ambientColor.x >= 0.0f &&
		ambientColor.y >= 0.0f &&
		ambientColor.z >= 0.0f &&
		ambientColor.w >= 0.0f);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.u.ambientColor = ambientColor;
}

Vec4F getDiffusePipelineLightColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.u.lightColor;
}
void setDiffusePipelineLightColor(
	Pipeline pipeline,
	Vec4F lightColor)
{
	assert(pipeline != NULL);
	assert(lightColor.x >= 0.0f &&
		lightColor.y >= 0.0f &&
		lightColor.z >= 0.0f &&
		lightColor.w >= 0.0f);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.u.lightColor = lightColor;
}

Vec3F getDiffusePipelineLightDirection(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	Vec4F lightDirection =
		pipelineHandle->vk.u.lightDirection;
	return vec3F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z);
}
void setDiffusePipelineLightDirection(
	Pipeline pipeline,
	Vec3F lightDirection)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	lightDirection = normVec3F(lightDirection);
	pipelineHandle->vk.u.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
}
