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

#include "mpgx/text.h"
#include "mpgx/_source/pipeline.h"
#include "mpgx/_source/image.h"
#include "mpgx/_source/sampler.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include <assert.h>

// TODO: make better look
// TODO: possibly bake in separated text pipeline thread

// TODO: on integrated GPU system occurs artifacts
// due to the text vertex buffer change during frame rendering,
// because memory is shared.

struct Font
{
	uint8_t* data;
	FT_Face face;
};

struct Text
{
	Font font;
	Pipeline pipeline;
	uint32_t fontSize;
	char* data;
	size_t dataSize;
	bool isConstant;
	Image texture;
	Mesh mesh;
	Vec2F textSize;
	size_t uniCharCount;
#if MPGX_SUPPORT_VULKAN
	VkDescriptorPool descriptorPool;
	VkDescriptorSet* descriptorSets;
#endif
};

typedef struct Glyph
{
	uint32_t uniChar;
	Vec4F position;
	Vec4F texCoords;
	float advance;
} Glyph;

typedef struct VertexPushConstants
{
	Mat4F mvp;
} VertexPushConstants;
typedef struct FragmentPushConstants
{
	LinearColor color;
} FragmentPushConstants;
typedef struct BasePipelineHandle
{
	Window window;
	Image texture;
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
} BasePipelineHandle;
typedef struct VkPipelineHandle
{
	Window window;
	Image texture;
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
#if MPGX_SUPPORT_VULKAN
	VkDescriptorSetLayout descriptorSetLayout;
	uint32_t bufferCount;
#endif
} VkPipelineHandle;
typedef struct GlPipelineHandle
{
	Window window;
	Image texture;
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
	GLint mvpLocation;
	GLint colorLocation;
	GLint textureLocation;
} GlPipelineHandle;
typedef union PipelineHandle
{
	BasePipelineHandle base;
	VkPipelineHandle vk;
	GlPipelineHandle gl;
} PipelineHandle;

Font createFont(
	const void* _data,
	size_t size)
{
	assert(_data != NULL);
	assert(size != 0);

	Font font = malloc(
		sizeof(struct Font));

	if (font == NULL)
		return NULL;

	uint8_t* data = malloc(
		size * sizeof(uint8_t));

	if (data == NULL)
		return NULL;

	memcpy(
		data,
		_data,
		size);

	FT_Library ftLibrary =
		(FT_Library)getFtLibrary();

	FT_Face face;

	FT_Error result = FT_New_Memory_Face(
		ftLibrary,
		data,
		(FT_Long)size,
		0,
		&face);

	if (result != 0)
	{
		free(data);
		free(font);
		return NULL;
	}

	result = FT_Select_Charmap(
		face,
		FT_ENCODING_UNICODE);

	if (result != 0)
	{
		FT_Done_Face(face);
		free(data);
		free(font);
		return NULL;
	}

	font->data = data;
	font->face = face;
	return font;
}
Font createFontFromFile(
	const void* filePath)
{
	assert(filePath != NULL);

	Font font = malloc(
		sizeof(struct Font));

	if (font == NULL)
		return NULL;

	FT_Library ftLibrary =
		(FT_Library)getFtLibrary();

	FT_Face face;

	FT_Error result = FT_New_Face(
		ftLibrary,
		filePath,
		0,
		&face);

	if (result != 0)
	{
		free(font);
		return NULL;
	}

	result = FT_Select_Charmap(
		face,
		FT_ENCODING_UNICODE);

	if (result != 0)
	{
		FT_Done_Face(face);
		free(font);
		return NULL;
	}

	font->data = NULL;
	font->face = face;
	return font;
}
void destroyFont(Font font)
{
	if (font == NULL)
		return;

	FT_Done_Face(font->face);
	free(font->data);
	free(font);
}

static int compareGlyph(
	const void* a,
	const void* b)
{
	if (((Glyph*)a)->uniChar <
		((Glyph*)b)->uniChar)
	{
		return -1;
	}
	if (((Glyph*)a)->uniChar ==
		((Glyph*)b)->uniChar)
	{
		return 0;
	}
	if (((Glyph*)a)->uniChar >
		((Glyph*)b)->uniChar)
	{
		return 1;
	}

	abort();
}
inline static size_t getTextUniCharCount(
	const char* data,
	size_t dataLength)
{
	size_t uniCharCount = 0;

	for (size_t i = 0; i < dataLength;)
	{
		if ((data[i] & 0b10000000) == 0)
		{
			i += 1;
		}
		else if ((data[i] & 0b11100000) == 0b11000000 &&
			(data[i + 1] & 0b11000000) == 0b10000000)
		{
			i += 2;
		}
		else if ((data[i] & 0b11110000) == 0b11100000 &&
			(data[i + 1] & 0b11000000) == 0b10000000 &&
			(data[i + 2] & 0b11000000) == 0b10000000)
		{
			i += 3;
		}
		else if ((data[i] & 0b11111000) == 0b11110000 &&
			(data[i + 1] & 0b11000000) == 0b10000000 &&
			(data[i + 2] & 0b11000000) == 0b10000000 &&
			(data[i + 3] & 0b11000000) == 0b10000000)
		{
			i += 4;
		}
		else
		{
			return 0;
		}

		uniCharCount++;
	}

	return uniCharCount;
}
inline static uint32_t* createTextUniChars(
	const char* data,
	size_t dataLength,
	size_t uniCharCount)
{
	uint32_t* uniChars = malloc(
		uniCharCount * sizeof(uint32_t));

	if (uniChars == NULL)
		return NULL;

	for (size_t i = 0, j = 0; i < dataLength; j++)
	{
		if ((data[i] & 0b10000000) == 0)
		{
			uniChars[j] = (uint32_t)data[i];
			i += 1;
		}
		else if ((data[i] & 0b11100000) == 0b11000000)
		{
			uniChars[j] =
				(uint32_t)(data[i] & 0b00011111) << 6 |
				(uint32_t)(data[i + 1] & 0b00111111);
			i += 2;
		}
		else if ((data[i] & 0b11110000) == 0b11100000)
		{
			uniChars[j] =
				(uint32_t)(data[i] & 0b00001111) << 12 |
				(uint32_t)(data[i + 1] & 0b00111111) << 6 |
				(uint32_t)(data[i + 2] & 0b00111111);
			i += 3;
		}
		else if ((data[i] & 0b11111000) == 0b11110000)
		{
			uniChars[j] =
				(uint32_t)(data[i] & 0b00000111) << 18 |
				(uint32_t)(data[i + 1] & 0b00111111) << 12 |
				(uint32_t)(data[i + 2] & 0b00111111) << 6 |
				(uint32_t)(data[i + 3] & 0b00111111);
			i += 4;
		}
		else
		{
			free(uniChars);
			return NULL;
		}
	}

	return uniChars;
}
inline static bool createTextGlyphs(
	const uint32_t* uniChars,
	size_t uniCharCount,
	Glyph** _glyphs,
	size_t* _glyphCount)
{
	size_t glyphCount = 0;

	Glyph* glyphs = malloc(
		uniCharCount * sizeof(Glyph));

	if (glyphs == NULL)
		return false;

	for (size_t i = 0; i < uniCharCount; i++)
	{
		uint32_t uniChar = uniChars[i];

		if (uniChar == '\n')
			continue;

		Glyph searchGlyph;
		searchGlyph.uniChar = uniChar;

		Glyph* glyph = bsearch(
			&searchGlyph,
			glyphs,
			glyphCount,
			sizeof(Glyph),
			compareGlyph);

		if (glyph == NULL)
		{
			glyphs[glyphCount].uniChar = uniChar;
			glyphCount++;

			qsort(
				glyphs,
				glyphCount,
				sizeof(Glyph),
				compareGlyph);
		}
	}

	*_glyphs = glyphs;
	*_glyphCount = glyphCount;
	return true;
}
inline static bool createTextPixels(
	FT_Face face,
	uint32_t fontSize,
	Glyph* glyphs,
	size_t glyphCount,
	uint32_t textPixelLength,
	float* _newLineAdvance,
	uint8_t** _pixels,
	size_t* _pixelCount,
	uint32_t* _pixelLength)
{
	uint32_t glyphLength = (uint32_t)sqrtf((float)glyphCount) + 1;
	uint32_t pixelLength = glyphLength * fontSize;
	size_t pixelCount = (size_t)pixelLength * pixelLength;

	uint8_t* pixels = calloc(
		pixelCount,
		sizeof(uint8_t));

	if (pixels == NULL)
		return false;

	FT_Error result = FT_Set_Pixel_Sizes(
		face,
		0,
		(FT_UInt)fontSize);

	if (result != 0)
	{
		free(pixels);
		return false;
	}

	float newLineAdvance =
		((float)face->size->metrics.height / 64.0f) /
		(float)fontSize;

	if (textPixelLength < pixelLength)
		textPixelLength = pixelLength;

	for (size_t i = 0; i < glyphCount; i++)
	{
		Glyph glyph;
		glyph.uniChar = glyphs[i].uniChar;

		FT_UInt charIndex = FT_Get_Char_Index(
			face,
			glyph.uniChar);
		result = FT_Load_Glyph(
			face,
			charIndex,
			FT_LOAD_RENDER);

		if (result != 0)
		{
			free(pixels);
			return false;
		}

		FT_GlyphSlot glyphSlot = face->glyph;
		uint8_t* bitmap = glyphSlot->bitmap.buffer;

		size_t pixelPosY = (i / glyphLength);
		size_t pixelPosX = (i - pixelPosY * glyphLength);

		pixelPosX *= fontSize;
		pixelPosY *= fontSize;

		size_t glyphWidth = glyphSlot->bitmap.width;
		size_t glyphHeight = glyphSlot->bitmap.rows;

		glyph.position.x = (float)glyphSlot->bitmap_left / (float)fontSize;
		glyph.position.y = ((float)glyphSlot->bitmap_top - (float)glyphHeight) / (float)fontSize;
		glyph.position.z = glyph.position.x + (float)glyphWidth / (float)fontSize;
		glyph.position.w = glyph.position.y + (float)glyphHeight /(float)fontSize;
		glyph.texCoords.x = (float)pixelPosX / (float)textPixelLength;
		glyph.texCoords.y = (float)pixelPosY / (float)textPixelLength;
		glyph.texCoords.z = glyph.texCoords.x + (float)glyphWidth / (float)textPixelLength;
		glyph.texCoords.w = glyph.texCoords.y + (float)glyphHeight / (float)textPixelLength;
		glyph.advance = ((float)glyphSlot->advance.x / 64.0f) / (float)fontSize;

		glyphs[i] = glyph;

		for (size_t y = 0; y < glyphHeight; y++)
		{
			for (size_t x = 0; x < glyphWidth; x++)
			{
				pixels[(y + pixelPosY) * pixelLength + (x + pixelPosX)] =
					bitmap[y * glyphWidth + x];
			}
		}
	}

	*_newLineAdvance = newLineAdvance;
	*_pixels = pixels;
	*_pixelCount = pixelCount;
	*_pixelLength = pixelLength;
	return true;
}
inline static bool createTextVertices( // TODO: use mapBuffer here, also detect empty glyphs and skip vertices
	const uint32_t* uniChars,
	size_t uniCharCount,
	const Glyph* glyphs,
	size_t glyphCount,
	float newLineAdvance,
	float** _vertices,
	size_t* _vertexCount,
	Vec2F* _textSize)
{
	size_t vertexCount =
		uniCharCount * 16;
	float* vertices = malloc(
		vertexCount * sizeof(float));
	Vec2F textSize = zeroVec2F;

	if (vertices == NULL)
		return false;

	size_t vertexIndex = 0;
	Vec2F vertexOffset = zeroVec2F;

	for (size_t i = 0; i < uniCharCount; i++)
	{
		uint32_t uniChar = uniChars[i];

		if (uniChar == '\n')
		{
			if (textSize.x < vertexOffset.x)
				textSize.x = vertexOffset.x;

			vertexOffset.y -= newLineAdvance;
			vertexOffset.x = 0.0f;
			continue;
		}

		Glyph searchGlyph;
		searchGlyph.uniChar = uniChar;

		Glyph* glyph = bsearch(
			&searchGlyph,
			glyphs,
			glyphCount,
			sizeof(Glyph),
			compareGlyph);

		if (glyph == NULL)
		{
			free(vertices);
			return false;
		}

		Vec4F position = vec4F(
			vertexOffset.x + glyph->position.x,
			vertexOffset.y + glyph->position.y,
			vertexOffset.x + glyph->position.z,
			vertexOffset.y + glyph->position.w);
		Vec4F texCoords = glyph->texCoords;

		vertices[vertexIndex + 0] = position.x;
		vertices[vertexIndex + 1] = position.y;
		vertices[vertexIndex + 2] = texCoords.x;
		vertices[vertexIndex + 3] = texCoords.w;
		vertices[vertexIndex + 4] = position.x;
		vertices[vertexIndex + 5] = position.w;
		vertices[vertexIndex + 6] = texCoords.x;
		vertices[vertexIndex + 7] = texCoords.y;
		vertices[vertexIndex + 8] = position.z;
		vertices[vertexIndex + 9] = position.w;
		vertices[vertexIndex + 10] = texCoords.z;
		vertices[vertexIndex + 11] = texCoords.y;
		vertices[vertexIndex + 12] = position.z;
		vertices[vertexIndex + 13] = position.y;
		vertices[vertexIndex + 14] = texCoords.z;
		vertices[vertexIndex + 15] = texCoords.w;

		vertexIndex += 16;
		vertexOffset.x += glyph->advance;
	}

	if (textSize.x < vertexOffset.x)
		textSize.x = vertexOffset.x;

	textSize.y = -vertexOffset.y;

	*_vertices = vertices;
	*_vertexCount = vertexCount;
	*_textSize = textSize;
	return true;
}
inline static bool createTextIndices(
	size_t uniCharCount,
	uint32_t** _indices,
	size_t* _indexCount)
{
	size_t indexCount =
		uniCharCount * 6;
	uint32_t* indices = malloc(
		indexCount * sizeof(uint32_t));

	if (indices == NULL)
		return false;

	for (size_t i = 0, j = 0; i < indexCount; i += 6, j += 4)
	{
		indices[i + 0] = (uint32_t)j + 0;
		indices[i + 1] = (uint32_t)j + 1;
		indices[i + 2] = (uint32_t)j + 2;
		indices[i + 3] = (uint32_t)j + 0;
		indices[i + 4] = (uint32_t)j + 2;
		indices[i + 5] = (uint32_t)j + 3;
	}

	*_indices = indices;
	*_indexCount = indexCount;
	return true;
}

#if MPGX_SUPPORT_VULKAN
inline static VkDescriptorPool createVkDescriptorPool(
	VkDevice device,
	uint32_t bufferCount)
{
	VkDescriptorPoolSize descriptorPoolSizes[1] = {
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			bufferCount * 2,
		},
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		bufferCount * 2,
		1,
		descriptorPoolSizes,
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
inline static VkDescriptorSet* createVkDescriptorSets(
	VkDevice device,
	VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	uint32_t bufferCount,
	VkSampler sampler,
	VkImageView imageView)
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
		VkDescriptorImageInfo descriptorImageInfos[1] = {
			{
				sampler,
				imageView,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			},
		};
		VkWriteDescriptorSet writeDescriptorSets[1] = {
			{
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				NULL,
				descriptorSets[i],
				0,
				0,
				1,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				descriptorImageInfos,
				NULL,
				NULL,
			},
		};

		vkUpdateDescriptorSets(
			device,
			1,
			writeDescriptorSets,
			0,
			NULL);
	}

	return descriptorSets;
}
#endif

Text createText(
	Pipeline pipeline,
	Font font,
	uint32_t fontSize,
	const char* _data,
	bool isConstant)
{
	assert(pipeline != NULL);
	assert(font != NULL);
	assert(fontSize > 0);
	assert(_data != NULL);

	assert(strcmp(
		pipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);

	Text text = malloc(sizeof(struct Text));

	if (text == NULL)
		return NULL;

	size_t dataLength = strlen(_data);

	size_t uniCharCount = getTextUniCharCount(
		_data,
		dataLength);

	Window window = pipeline->base.framebuffer->base.window;
	GraphicsAPI api = getWindowGraphicsAPI(window);

	if (uniCharCount == 0)
	{
		size_t dataSize = 1;

		char* data = malloc(
			dataSize * sizeof(char));

		if (data == NULL)
		{
			free(text);
			return NULL;
		}

		data[0] = '\0';

		void* pixels = NULL;

		Image texture = createImage(
			window,
			IMAGE_2D_TYPE,
			R8_UNORM_IMAGE_FORMAT,
			(const void**)&pixels,
			vec3U(fontSize, fontSize, 1),
			1,
			isConstant,
			false);

		if (texture == NULL)
		{
			free(data);
			free(text);
			return NULL;
		}

		Buffer vertexBuffer = createBuffer(
			window,
			VERTEX_BUFFER_TYPE,
			NULL,
			16,
			isConstant);

		if (vertexBuffer == NULL)
		{
			destroyImage(texture);
			free(data);
			free(text);
			return NULL;
		}

		Buffer indexBuffer = createBuffer(
			window,
			INDEX_BUFFER_TYPE,
			NULL,
			6,
			isConstant);

		if (indexBuffer == NULL)
		{
			destroyBuffer(vertexBuffer);
			destroyImage(texture);
			free(data);
			free(text);
			return NULL;
		}

		Mesh mesh = createMesh(
			window,
			UINT32_DRAW_INDEX,
			0,
			0,
			vertexBuffer,
			indexBuffer);

		if (mesh == NULL)
		{
			destroyBuffer(indexBuffer);
			destroyBuffer(vertexBuffer);
			destroyImage(texture);
			free(data);
			free(text);
			return NULL;
		}

#if MPGX_SUPPORT_VULKAN
		if (api == VULKAN_GRAPHICS_API)
		{
			VkWindow vkWindow = getVkWindow(window);
			VkDevice device = vkWindow->device;
			PipelineHandle* pipelineHandle = pipeline->vk.handle;
			uint8_t bufferCount = pipelineHandle->vk.bufferCount;

			VkDescriptorPool descriptorPool = createVkDescriptorPool(
				device,
				bufferCount);

			if (descriptorPool == NULL)
			{
				destroyMesh(mesh, true);
				destroyImage(texture);
				free(data);
				free(text);
				return NULL;
			}

			VkDescriptorSet* descriptorSets = createVkDescriptorSets(
				device,
				pipelineHandle->vk.descriptorSetLayout,
				descriptorPool,
				bufferCount,
				pipelineHandle->vk.sampler->vk.handle,
				texture->vk.imageView);

			if (descriptorSets == NULL)
			{
				vkDestroyDescriptorPool(
					device,
					descriptorPool,
					NULL);
				destroyMesh(mesh, true);
				destroyImage(texture);
				free(data);
				free(text);
				return NULL;
			}

			text->descriptorPool = descriptorPool;
			text->descriptorSets = descriptorSets;
		}
		else
		{
			text->descriptorPool = NULL;
			text->descriptorSets = NULL;
		}
#endif

		text->data = data;
		text->dataSize = 1;
		text->texture = texture;
		text->mesh = mesh;
		text->textSize = zeroVec2F;
		text->uniCharCount = 0;
	}
	else
	{
		uint32_t* uniChars = createTextUniChars(
			_data,
			dataLength,
			uniCharCount);

		if (uniChars == NULL)
		{
			free(text);
			return NULL;
		}

		Glyph* glyphs;
		size_t glyphCount;

		bool result = createTextGlyphs(
			uniChars,
			uniCharCount,
			&glyphs,
			&glyphCount);

		if (result == false)
		{
			free(uniChars);
			free(text);
			return NULL;
		}

		size_t dataSize = dataLength + 1;
		char* data = malloc(dataSize * sizeof(char));

		if (data == NULL)
		{
			free(glyphs);
			free(uniChars);
			free(text);
			return NULL;
		}

		memcpy(
			data,
			_data,
			dataSize * sizeof(char));

		float newLineAdvance;
		uint8_t* pixels;
		size_t pixelCount;
		uint32_t pixelLength;

		result = createTextPixels(
			font->face,
			fontSize,
			glyphs,
			glyphCount,
			0,
			&newLineAdvance,
			&pixels,
			&pixelCount,
			&pixelLength);

		if (result == false)
		{
			free(data);
			free(glyphs);
			free(uniChars);
			free(text);
			return NULL;
		}

		Image texture = createImage(
			window,
			IMAGE_2D_TYPE,
			R8_UNORM_IMAGE_FORMAT,
			(const void**)&pixels,
			vec3U(pixelLength, pixelLength, 1),
			1,
			isConstant,
			false);

		free(pixels);

		if (texture == NULL)
		{
			free(data);
			free(glyphs);
			free(uniChars);
			free(text);
			return NULL;
		}

		float* vertices;
		size_t vertexCount;
		Vec2F textSize;

		result = createTextVertices(
			uniChars,
			uniCharCount,
			glyphs,
			glyphCount,
			newLineAdvance,
			&vertices,
			&vertexCount,
			&textSize);

		free(glyphs);
		free(uniChars);

		if (result == false)
		{
			destroyImage(texture);
			free(data);
			free(text);
			return NULL;
		}

		Buffer vertexBuffer = createBuffer(
			window,
			VERTEX_BUFFER_TYPE,
			vertices,
			vertexCount * sizeof(float),
			isConstant);

		free(vertices);

		if (vertexBuffer == NULL)
		{
			destroyImage(texture);
			free(data);
			free(text);
			return NULL;
		}

		uint32_t* indices;
		size_t indexCount;

		result = createTextIndices(
			uniCharCount,
			&indices,
			&indexCount);

		if (result == false)
		{
			destroyBuffer(vertexBuffer);
			destroyImage(texture);
			free(data);
			free(text);
			return NULL;
		}

		Buffer indexBuffer = createBuffer(
			window,
			INDEX_BUFFER_TYPE,
			indices,
			indexCount * sizeof(uint32_t),
			isConstant);

		free(indices);

		if (indexBuffer == NULL)
		{
			destroyBuffer(vertexBuffer);
			destroyImage(texture);
			free(data);
			free(text);
			return NULL;
		}

		Mesh mesh = createMesh(
			window,
			UINT32_DRAW_INDEX,
			indexCount,
			0,
			vertexBuffer,
			indexBuffer);

		if (mesh == NULL)
		{
			destroyBuffer(indexBuffer);
			destroyBuffer(vertexBuffer);
			destroyImage(texture);
			free(data);
			free(text);
			return NULL;
		}

#if MPGX_SUPPORT_VULKAN
		if (api == VULKAN_GRAPHICS_API)
		{
			VkWindow vkWindow = getVkWindow(window);
			VkDevice device = vkWindow->device;

			PipelineHandle* pipelineHandle = pipeline->vk.handle;
			uint8_t bufferCount = pipelineHandle->vk.bufferCount;

			VkDescriptorPool descriptorPool = createVkDescriptorPool(
				device,
				bufferCount);

			if (descriptorPool == NULL)
			{
				destroyMesh(mesh, true);
				destroyImage(texture);
				free(data);
				free(text);
				return NULL;
			}

			VkDescriptorSet* descriptorSets = createVkDescriptorSets(
				device,
				pipelineHandle->vk.descriptorSetLayout,
				descriptorPool,
				bufferCount,
				pipelineHandle->vk.sampler->vk.handle,
				texture->vk.imageView);

			if (descriptorSets == NULL)
			{
				vkDestroyDescriptorPool(
					device,
					descriptorPool,
					NULL);
				destroyMesh(mesh, true);
				destroyImage(texture);
				free(data);
				free(text);
				return NULL;
			}

			text->descriptorPool = descriptorPool;
			text->descriptorSets = descriptorSets;
		}
		else
		{
			text->descriptorPool = NULL;
			text->descriptorSets = NULL;
		}
#endif

		text->data = data;
		text->dataSize = dataSize;
		text->texture = texture;
		text->mesh = mesh;
		text->textSize = textSize;
		text->uniCharCount = uniCharCount;
	}

	text->font = font;
	text->pipeline = pipeline;
	text->fontSize = fontSize;
	text->isConstant = isConstant;
	return text;
}
void destroyText(Text text)
{
	if (text == NULL)
		return;

#if MPGX_SUPPORT_VULKAN
	Pipeline pipeline = text->pipeline;

	GraphicsAPI api = getWindowGraphicsAPI(
		pipeline->base.framebuffer->base.window);

	if (api == VULKAN_GRAPHICS_API)
	{
		VkWindow vkWindow = getVkWindow(
			pipeline->vk.framebuffer->vk.window);
		VkDevice device = vkWindow->device;

		VkResult vkResult = vkQueueWaitIdle(
			vkWindow->graphicsQueue);

		if (vkResult != VK_SUCCESS)
			abort();

		free(text->descriptorSets);

		vkDestroyDescriptorPool(
			device,
			text->descriptorPool,
			NULL);
	}
#endif

	destroyMesh(
		text->mesh,
		true);
	destroyImage(text->texture);

	free(text->data);
	free(text);
}

Pipeline getTextPipeline(Text text)
{
	assert(text != NULL);
	return text->pipeline;
}
bool isTextConstant(Text text)
{
	assert(text != NULL);
	return text->isConstant;
}
Vec2F getTextSize(Text text)
{
	assert(text != NULL);
	return text->textSize;
}
size_t getTextIndexCount(Text text)
{
	assert(text != NULL);
	return getMeshIndexCount(text->mesh);
}

Vec2F getTextOffset(
	Text text,
	InterfaceAnchor anchor)
{
	assert(text != NULL);
	assert(anchor < INTERFACE_ANCHOR_COUNT);

	Vec2F offset = text->textSize;

	switch (anchor)
	{
	default:
		abort();
	case CENTER_INTERFACE_ANCHOR:
		return vec2F(
			-offset.x / 2.0f,
			offset.y / 2.0f);
	case LEFT_INTERFACE_ANCHOR:
		return vec2F(
			0.0f,
			offset.y / 2.0f);
	case RIGHT_INTERFACE_ANCHOR:
		return vec2F(
			-offset.x,
			offset.y / 2.0f);
	case BOTTOM_INTERFACE_ANCHOR:
		return vec2F(
			-offset.x / 2.0f,
			0.0f);
	case TOP_INTERFACE_ANCHOR:
		return vec2F(
			-offset.x / 2.0f,
			offset.y);
	case LEFT_BOTTOM_INTERFACE_ANCHOR:
		return vec2F(
			0.0f,
			0.0f);
	case LEFT_TOP_INTERFACE_ANCHOR:
		return vec2F(
			0.0f,
			offset.y);
	case RIGHT_BOTTOM_INTERFACE_ANCHOR:
		return vec2F(
			-offset.x,
			0.0f);
	case RIGHT_TOP_INTERFACE_ANCHOR:
		return vec2F(
			-offset.x,
			offset.y);
	}
}
size_t getTextUnicodeCharCount(
	Text text)
{
	assert(text != NULL);
	return text->uniCharCount;
}
bool getTextUnicodeCharAdvance(
	Text text,
	size_t index,
	Vec2F* _advance)
{
	assert(text != NULL);
	assert(index < text->uniCharCount);
	assert(_advance != NULL);

	const char* data = text->data;
	FT_Face face = text->font->face;
	uint32_t fontSize = text->fontSize;

	float newLineAdvance =
		((float)face->size->metrics.height / 64.0f) /
		(float)fontSize;

	Vec2F advance = zeroVec2F;

	for (size_t i = 0, j = 0; j <= index; j++)
	{
		uint32_t uniChar;

		if ((data[i] & 0b10000000) == 0)
		{
			uniChar = (uint32_t)data[i];
			i += 1;
		}
		else if ((data[i] & 0b11100000) == 0b11000000 &&
			(data[i + 1] & 0b11000000) == 0b10000000)
		{
			uniChar =
				(uint32_t)(data[i] & 0b00011111) << 6 |
				(uint32_t)(data[i + 1] & 0b00111111);
			i += 2;
		}
		else if ((data[i] & 0b11110000) == 0b11100000 &&
			(data[i + 1] & 0b11000000) == 0b10000000 &&
			(data[i + 2] & 0b11000000) == 0b10000000)
		{
			uniChar =
				(uint32_t)(data[i] & 0b00001111) << 12 |
				(uint32_t)(data[i + 1] & 0b00111111) << 6 |
				(uint32_t)(data[i + 2] & 0b00111111);
			i += 3;
		}
		else if ((data[i] & 0b11111000) == 0b11110000 &&
			(data[i + 1] & 0b11000000) == 0b10000000 &&
			(data[i + 2] & 0b11000000) == 0b10000000 &&
			(data[i + 3] & 0b11000000) == 0b10000000)
		{
			uniChar =
				(uint32_t)(data[i] & 0b00000111) << 18 |
				(uint32_t)(data[i + 1] & 0b00111111) << 12 |
				(uint32_t)(data[i + 2] & 0b00111111) << 6 |
				(uint32_t)(data[i + 3] & 0b00111111);
			i += 4;
		}
		else
		{
			return false;
		}

		if (uniChar == '\n')
		{
			advance.y -= newLineAdvance;
			advance.x = 0.0f;
			continue;
		}

		FT_UInt charIndex = FT_Get_Char_Index(
			face,
			uniChar);
		FT_Error result = FT_Load_Glyph(
			face,
			charIndex,
			FT_LOAD_BITMAP_METRICS_ONLY);

		if (result != 0)
			return false;

		advance.x += ((float)face->glyph->advance.x / 64.0f) /
			(float)fontSize;
	}

	*_advance = advance;
	return true;
}

Font getTextFont(
	Text text)
{
	assert(text != NULL);
	return text->font;
}
void setTextFont(
	Text text,
	Font font)
{
	assert(text != NULL);
	assert(font != NULL);
	assert(text->isConstant == false);
	text->font = font;
}

uint32_t getTextFontSize(
	Text text)
{
	assert(text != NULL);
	return text->fontSize;
}
void setTextFontSize(
	Text text,
	uint32_t fontSize)
{
	assert(text != NULL);
	assert(text->isConstant == false);
	text->fontSize = fontSize;
}

const char* getTextData(
	Text text)
{
	assert(text != NULL);
	return text->data;
}
bool setTextData(
	Text text,
	const char* _data)
{
	assert(text != NULL);
	assert(_data != NULL);
	assert(text->isConstant == false);

	size_t dataSize =
		strlen(_data) + 1;

	if (dataSize > text->dataSize)
	{
		char* data = realloc(
			text->data,
			dataSize);

		if (data == NULL)
			return false;

		memcpy(
			data,
			_data,
			dataSize);

		text->data = data;
		text->dataSize = dataSize;
		return true;
	}
	else
	{
		memcpy(
			text->data,
			_data,
			dataSize);
		return true;
	}
}

bool bakeText(
	Text text,
	bool reuse)
{
	assert(text != NULL);
	assert(text->isConstant == false);

	Pipeline pipeline = text->pipeline;
	Window window = pipeline->base.framebuffer->base.window;
	GraphicsAPI api = getWindowGraphicsAPI(window);

	const char* _data = text->data;
	size_t dataLength = strlen(_data);

	size_t uniCharCount = getTextUniCharCount(
		_data,
		dataLength);

	if (reuse == true)
	{
		if (uniCharCount == 0)
		{
			setMeshIndexCount(
				text->mesh,
				0);
			text->textSize = zeroVec2F;
			text->uniCharCount = 0;
		}
		else
		{
			uint32_t* uniChars = createTextUniChars(
				_data,
				dataLength,
				uniCharCount);

			if (uniChars == NULL)
				return false;

			Glyph* glyphs;
			size_t glyphCount;

			bool result = createTextGlyphs(
				uniChars,
				uniCharCount,
				&glyphs,
				&glyphCount);

			if (result == false)
			{
				free(uniChars);
				return false;
			}

			uint32_t textPixelLength =
				getImageSize(text->texture).x;

			float newLineAdvance;
			uint8_t* pixels;
			size_t pixelCount;
			uint32_t pixelLength;

			result = createTextPixels(
				text->font->face,
				text->fontSize,
				glyphs,
				glyphCount,
				textPixelLength,
				&newLineAdvance,
				&pixels,
				&pixelCount,
				&pixelLength);

			if (result == false)
			{
				free(glyphs);
				free(uniChars);
				return false;
			}

			Image texture;

#if MPGX_SUPPORT_VULKAN
			VkDescriptorSet* descriptorSets;
#endif

			if (pixelLength > textPixelLength)
			{
				texture = createImage(
					window,
					IMAGE_2D_TYPE,
					R8_UNORM_IMAGE_FORMAT,
					(const void**)&pixels,
					vec3U(pixelLength, pixelLength, 1),
					1,
					false,
					false);

				free(pixels);
				pixels = NULL;

				if (texture == NULL)
				{
					free(glyphs);
					free(uniChars);
					return false;
				}

#if MPGX_SUPPORT_VULKAN
				if (api == VULKAN_GRAPHICS_API)
				{
					VkWindow vkWindow = getVkWindow(window);
					VkDevice device = vkWindow->device;
					PipelineHandle* pipelineHandle = pipeline->vk.handle;

					descriptorSets = createVkDescriptorSets(
						device,
						pipelineHandle->vk.descriptorSetLayout,
						text->descriptorPool,
						pipelineHandle->vk.bufferCount,
						pipelineHandle->vk.sampler->vk.handle,
						texture->vk.imageView);

					if (descriptorSets == NULL)
					{
						destroyImage(texture);
						free(glyphs);
						free(uniChars);
						return false;
					}
				}
#endif
			}
			else
			{
				texture = NULL;
			}

			float* vertices;
			size_t vertexCount;
			Vec2F textSize;

			result = createTextVertices(
				uniChars,
				uniCharCount,
				glyphs,
				glyphCount,
				newLineAdvance,
				&vertices,
				&vertexCount,
				&textSize);

			free(glyphs);
			free(uniChars);

			if (result == false)
			{
				destroyImage(texture);
				free(pixels);
				return false;
			}

			Buffer vertexBuffer = NULL;
			Buffer indexBuffer = NULL;

			size_t textVertexBufferSize = getBufferSize(
				getMeshVertexBuffer(text->mesh));

			if (vertexCount * sizeof(float) > textVertexBufferSize)
			{
				vertexBuffer = createBuffer(
					window,
					VERTEX_BUFFER_TYPE,
					vertices,
					vertexCount * sizeof(float),
					false);

				free(vertices);

				if (vertexBuffer == NULL)
				{
					destroyImage(texture);
					free(pixels);
					return false;
				}

				uint32_t* indices;
				size_t indexCount;

				result = createTextIndices(
					uniCharCount,
					&indices,
					&indexCount);

				if (result == false)
				{
					destroyBuffer(vertexBuffer);
					destroyImage(texture);
					free(pixels);
					return false;
				}

				indexBuffer = createBuffer(
					window,
					INDEX_BUFFER_TYPE,
					indices,
					indexCount * sizeof(uint32_t),
					false);

				free(indices);

				if (indexBuffer == NULL)
				{
					destroyBuffer(vertexBuffer);
					destroyImage(texture);
					free(pixels);
					return false;
				}
			}

			if (texture == NULL)
			{
				setImageData(
					text->texture,
					pixels,
					vec3U(pixelLength, pixelLength, 1),
					zeroVec3U);
				free(pixels); // TODO: replace pixels with image data map
			}
			else
			{
#if MPGX_SUPPORT_VULKAN
				if (api == VULKAN_GRAPHICS_API)
				{
					VkWindow vkWindow = getVkWindow(window);
					VkDevice device = vkWindow->device;
					PipelineHandle* pipelineHandle = pipeline->vk.handle;

					vkFreeDescriptorSets(
						device,
						text->descriptorPool,
						pipelineHandle->vk.bufferCount,
						text->descriptorSets);
					free(text->descriptorSets);

					text->descriptorSets = descriptorSets;
				}
#endif

				destroyImage(text->texture);
				text->texture = texture;
			}

			if (vertexBuffer == NULL)
			{
				Mesh mesh = text->mesh;
				Buffer _vertexBuffer = getMeshVertexBuffer(mesh);

				setBufferData(
					_vertexBuffer,
					vertices,
					vertexCount * sizeof(float),
					0);
				setMeshIndexCount(
					mesh,
					uniCharCount * 6);

				free(vertices);
			}
			else
			{
				Mesh mesh = text->mesh;
				Buffer _vertexBuffer = getMeshVertexBuffer(mesh);
				Buffer _indexBuffer = getMeshIndexBuffer(mesh);

				destroyBuffer(_vertexBuffer);
				destroyBuffer(_indexBuffer);

				setMeshVertexBuffer(
					mesh,
					vertexBuffer);
				setMeshIndexBuffer(
					mesh,
					UINT32_DRAW_INDEX,
					uniCharCount * 6,
					0,
					indexBuffer);
			}

			text->textSize = textSize;
			text->uniCharCount = uniCharCount;
		}
	}
	else
	{
		if (uniCharCount == 0)
		{
			size_t dataSize = 1;

			char* data = malloc(
				dataSize * sizeof(char));

			if (data == NULL)
				return false;

			data[0] = '\0';

			void* pixels = NULL;

			Image texture = createImage(
				window,
				IMAGE_2D_TYPE,
				R8_UNORM_IMAGE_FORMAT,
				(const void**)&pixels,
				vec3U(text->fontSize, text->fontSize, 1),
				1,
				false,
				false);

			if (texture == NULL)
			{
				free(data);
				return false;
			}

			Buffer vertexBuffer = createBuffer(
				window,
				VERTEX_BUFFER_TYPE,
				NULL,
				16,
				false);

			if (vertexBuffer == NULL)
			{
				destroyImage(texture);
				free(data);
				return false;
			}

			Buffer indexBuffer = createBuffer(
				window,
				INDEX_BUFFER_TYPE,
				NULL,
				6,
				false);

			if (indexBuffer == NULL)
			{
				destroyBuffer(vertexBuffer);
				destroyImage(texture);
				free(data);
				return false;
			}

			Mesh mesh = createMesh(
				window,
				UINT32_DRAW_INDEX,
				0,
				0,
				vertexBuffer,
				indexBuffer);

			if (mesh == NULL)
			{
				destroyBuffer(indexBuffer);
				destroyBuffer(vertexBuffer);
				destroyImage(texture);
				free(data);
				return false;
			}

#if MPGX_SUPPORT_VULKAN
			if (api == VULKAN_GRAPHICS_API)
			{
				VkWindow vkWindow = getVkWindow(window);
				VkDevice device = vkWindow->device;
				PipelineHandle* pipelineHandle = pipeline->vk.handle;
				uint8_t bufferCount = pipelineHandle->vk.bufferCount;
				VkDescriptorPool descriptorPool = text->descriptorPool;

				VkDescriptorSet* descriptorSets = createVkDescriptorSets(
					device,
					pipelineHandle->vk.descriptorSetLayout,
					descriptorPool,
					bufferCount,
					pipelineHandle->vk.sampler->vk.handle,
					texture->vk.imageView);

				if (descriptorSets == NULL)
				{
					destroyMesh(mesh, true);
					destroyImage(texture);
					free(data);
					free(text);
					return false;
				}

				vkFreeDescriptorSets(
					device,
					descriptorPool,
					bufferCount,
					text->descriptorSets);
				free(text->descriptorSets);

				text->descriptorSets = descriptorSets;
			}
#endif

			destroyMesh(
				text->mesh,
				true);
			destroyImage(text->texture);
			free(text->data);

			text->data = data;
			text->dataSize = 1;
			text->texture = texture;
			text->mesh = mesh;
			text->textSize = zeroVec2F;
			text->uniCharCount = 0;
		}
		else
		{
			uint32_t* uniChars = createTextUniChars(
				_data,
				dataLength,
				uniCharCount);

			if (uniChars == NULL)
				return false;

			Glyph* glyphs;
			size_t glyphCount;

			bool result = createTextGlyphs(
				uniChars,
				uniCharCount,
				&glyphs,
				&glyphCount);

			if (result == false)
			{
				free(uniChars);
				return false;
			}

			size_t dataSize =
				dataLength + 1;
			char* data = malloc(
				dataSize * sizeof(char));

			if (data == NULL)
			{
				free(glyphs);
				free(uniChars);
				return false;
			}

			memcpy(
				data,
				_data,
				dataSize * sizeof(char));

			float newLineAdvance;
			uint8_t* pixels;
			size_t pixelCount;
			uint32_t pixelLength;

			result = createTextPixels(
				text->font->face,
				text->fontSize,
				glyphs,
				glyphCount,
				0,
				&newLineAdvance,
				&pixels,
				&pixelCount,
				&pixelLength);

			if (result == false)
			{
				free(data);
				free(glyphs);
				free(uniChars);
				return false;
			}

			Image texture = createImage(
				window,
				IMAGE_2D_TYPE,
				R8_UNORM_IMAGE_FORMAT,
				(const void**)&pixels,
				vec3U(pixelLength, pixelLength, 1),
				1,
				false,
				false);

			free(pixels);

			if (texture == NULL)
			{
				free(data);
				free(glyphs);
				free(uniChars);
				return false;
			}

			float* vertices;
			size_t vertexCount;
			Vec2F textSize;

			result = createTextVertices(
				uniChars,
				uniCharCount,
				glyphs,
				glyphCount,
				newLineAdvance,
				&vertices,
				&vertexCount,
				&textSize);

			free(glyphs);
			free(uniChars);

			if (result == false)
			{
				destroyImage(texture);
				free(data);
				return false;
			}

			Buffer vertexBuffer = createBuffer(
				window,
				VERTEX_BUFFER_TYPE,
				vertices,
				vertexCount * sizeof(float),
				false);

			free(vertices);

			if (vertexBuffer == NULL)
			{
				destroyImage(texture);
				free(data);
				return false;
			}

			uint32_t* indices;
			size_t indexCount;

			result = createTextIndices(
				uniCharCount,
				&indices,
				&indexCount);

			if (result == false)
			{
				destroyBuffer(vertexBuffer);
				destroyImage(texture);
				free(data);
				return false;
			}

			Buffer indexBuffer = createBuffer(
				window,
				INDEX_BUFFER_TYPE,
				indices,
				indexCount * sizeof(uint32_t),
				false);

			free(indices);

			if (indexBuffer == NULL)
			{
				destroyBuffer(vertexBuffer);
				destroyImage(texture);
				free(data);
				return false;
			}

			Mesh mesh = createMesh(
				window,
				UINT32_DRAW_INDEX,
				indexCount,
				0,
				vertexBuffer,
				indexBuffer);

			if (mesh == NULL)
			{
				destroyBuffer(indexBuffer);
				destroyBuffer(vertexBuffer);
				destroyImage(texture);
				free(data);
				return false;
			}

#if MPGX_SUPPORT_VULKAN
			if (api == VULKAN_GRAPHICS_API)
			{
				VkWindow vkWindow = getVkWindow(window);
				VkDevice device = vkWindow->device;
				PipelineHandle* pipelineHandle = pipeline->vk.handle;
				uint8_t bufferCount = pipelineHandle->vk.bufferCount;
				VkDescriptorPool descriptorPool = text->descriptorPool;

				VkDescriptorSet* descriptorSets = createVkDescriptorSets(
					device,
					pipelineHandle->vk.descriptorSetLayout,
					descriptorPool,
					bufferCount,
					pipelineHandle->vk.sampler->vk.handle,
					texture->vk.imageView);

				if (descriptorSets == NULL)
				{
					destroyMesh(mesh, true);
					destroyImage(texture);
					free(data);
					free(text);
					return false;
				}

				vkFreeDescriptorSets(
					device,
					descriptorPool,
					bufferCount,
					text->descriptorSets);
				free(text->descriptorSets);

				text->descriptorPool = descriptorPool;
				text->descriptorSets = descriptorSets;
			}
#endif

			destroyMesh(
				text->mesh,
				true);
			destroyImage(text->texture);
			free(text->data);

			text->data = data;
			text->dataSize = dataSize;
			text->texture = texture;
			text->mesh = mesh;
			text->textSize = textSize;
			text->uniCharCount = uniCharCount;
		}
	}

	return true;
}
size_t drawText(Text text)
{
	assert(text != NULL);

	Pipeline pipeline = text->pipeline;
	PipelineHandle* textPipeline = pipeline->gl.handle;
	textPipeline->base.texture = text->texture;

#if MPGX_SUPPORT_VULKAN
	Window window = pipeline->base.framebuffer->base.window;
	GraphicsAPI api = getWindowGraphicsAPI(window);

	if (api == VULKAN_GRAPHICS_API)
	{
		VkWindow vkWindow = getVkWindow(window);
		VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;

		// TODO: get scissors from bounding box
		VkRect2D scissor = {
			{ 0, 0 },
			{ 10000, 10000 },
		};
		vkCmdSetScissor(
			commandBuffer,
			0,
			1,
			&scissor);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->vk.layout,
			0,
			1,
			&text->descriptorSets[vkWindow->bufferIndex],
			0,
			NULL);

	}
#endif

	return drawMesh(
		text->mesh,
		pipeline);
}

Sampler createTextSampler(Window window)
{
	assert(window != NULL);

	return createSampler(
		window,
		LINEAR_IMAGE_FILTER,
		LINEAR_IMAGE_FILTER,
		NEAREST_IMAGE_FILTER,
		false,
		REPEAT_IMAGE_WRAP,
		REPEAT_IMAGE_WRAP,
		REPEAT_IMAGE_WRAP,
		NEVER_COMPARE_OPERATOR,
		false,
		defaultMipmapLodRange,
		DEFAULT_MIPMAP_LOD_BIAS);
}

#if MPGX_SUPPORT_VULKAN
static const VkVertexInputBindingDescription vertexInputBindingDescriptions[1] = {
	{
		0,
		sizeof(Vec2F) * 2,
		VK_VERTEX_INPUT_RATE_VERTEX,
	},
};
static const VkVertexInputAttributeDescription vertexInputAttributeDescriptions[2] = {
	{
		0,
		0,
		VK_FORMAT_R32G32_SFLOAT,
		0,
	},
	{
		1,
		0,
		VK_FORMAT_R32G32_SFLOAT,
		sizeof(Vec2F),
	},
};
static const VkPushConstantRange pushConstantRanges[2] = {
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
	},
	{
		VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(VertexPushConstants),
		sizeof(FragmentPushConstants),
	},
};

inline static VkDescriptorSetLayout createVkDescriptorSetLayout(
	VkDevice device)
{
	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[1] = {
		{
			0,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			NULL,
		},
	};
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		NULL,
		0,
		1,
		descriptorSetLayoutBindings,
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

static void onVkUniformsSet(Pipeline pipeline)
{
	PipelineHandle* pipelineHandle = pipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(pipelineHandle->vk.window);
	VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;
	VkPipelineLayout layout = pipeline->vk.layout;

	vkCmdPushConstants(
		commandBuffer,
		layout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
		&pipelineHandle->vk.vpc);
	vkCmdPushConstants(
		commandBuffer,
		layout,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(VertexPushConstants),
		sizeof(FragmentPushConstants),
		&pipelineHandle->vk.fpc);
}
static bool onVkHandleResize(
	Pipeline pipeline,
	Vec2U newSize,
	void* createInfo)
{
	PipelineHandle* pipelineHandle = pipeline->vk.handle;
	Window window = pipelineHandle->vk.window;
	VkWindow vkWindow = getVkWindow(window);
	VkSwapchain swapchain = vkWindow->swapchain;
	uint32_t bufferCount = swapchain->bufferCount;

	// TODO: resize each text
	/*if (bufferCount != handle->vk.bufferCount)
	{
		VkDevice device = vkWindow->device;

		free(handle->vk.descriptorSets);

		vkDestroyDescriptorPool(
			device,
			handle->vk.descriptorPool,
			NULL);

		VkDescriptorPool descriptorPool = createVkDescriptorPool(
			device,
			bufferCount);

		if (descriptorPool == NULL)
			abort();

		VkDescriptorSet* descriptorSets = createVkDescriptorSets(
			device,
			handle->vk.descriptorSetLayout,
			descriptorPool,
			bufferCount,
			handle->vk.sampler->vk.handle,
			handle->vk.imageView);

		if (descriptorSets == NULL)
			abort();

		handle->vk.descriptorPool = descriptorPool;
		handle->vk.descriptorSets = descriptorSets;
		handle->vk.bufferCount = bufferCount;
	}*/

	Vec4I size = vec4I(0, 0,
		(int32_t)newSize.x,
		(int32_t)newSize.y);
	pipeline->vk.state.viewport = size;

	VkPipelineCreateInfo _createInfo = {
		1,
		vertexInputBindingDescriptions,
		2,
		vertexInputAttributeDescriptions,
		1,
		&pipelineHandle->vk.descriptorSetLayout,
		2,
		pushConstantRanges,
	};

	*(VkPipelineCreateInfo*)createInfo = _createInfo;
	return true;
}
static void onVkHandleDestroy(void* handle)
{
	PipelineHandle* pipelineHandle = handle;
	VkWindow vkWindow = getVkWindow(pipelineHandle->vk.window);
	VkDevice device = vkWindow->device;

	vkDestroyDescriptorSetLayout(
		device,
		pipelineHandle->vk.descriptorSetLayout,
		NULL);
	free(pipelineHandle);
}
inline static Pipeline createVkHandle(
	Framebuffer framebuffer,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	PipelineHandle* pipelineHandle)
{
	VkWindow vkWindow = getVkWindow(framebuffer->vk.window);
	VkDevice device = vkWindow->device;

	VkDescriptorSetLayout descriptorSetLayout =
		createVkDescriptorSetLayout(device);

	if (descriptorSetLayout == NULL)
	{
		free(pipelineHandle);
		return NULL;
	}

	VkPipelineCreateInfo createInfo = {
		1,
		vertexInputBindingDescriptions,
		2,
		vertexInputAttributeDescriptions,
		1,
		&descriptorSetLayout,
		2,
		pushConstantRanges,
	};

	pipelineHandle->vk.descriptorSetLayout = descriptorSetLayout;
	pipelineHandle->vk.bufferCount = vkWindow->swapchain->bufferCount;

	return createPipeline(
		framebuffer,
		TEXT_PIPELINE_NAME,
		shaders,
		shaderCount,
		state,
		NULL,
		onVkUniformsSet,
		onVkHandleResize,
		onVkHandleDestroy,
		pipelineHandle,
		&createInfo);
}
#endif

static void onGlUniformsSet(Pipeline pipeline)
{
	PipelineHandle* pipelineHandle = pipeline->gl.handle;

	glUniformMatrix4fv(
		pipelineHandle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const float*)&pipelineHandle->gl.vpc.mvp);
	glUniform4fv(
		pipelineHandle->gl.colorLocation,
		1,
		(const float*)&pipelineHandle->gl.fpc.color);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec2F) * 2,
		0);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec2F) * 2,
		(const void*)sizeof(Vec2F));

	glUniform1i(
		pipelineHandle->gl.textureLocation,
		0);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(
		GL_TEXTURE_2D,
		pipelineHandle->gl.texture->gl.handle);
	glBindSampler(
		0,
		pipelineHandle->gl.sampler->gl.handle);

	assertOpenGL();
}
static bool onGlHandleResize(
	Pipeline pipeline,
	Vec2U newSize,
	void* createInfo)
{
	pipeline->vk.state.viewport = vec4I(
		0, 0,
		(int32_t)newSize.x,
		(int32_t)newSize.y);
	return true;
}
static void onGlHandleDestroy(void* handle)
{
	PipelineHandle* pipelineHandle = handle;
	free(pipelineHandle);
}
inline static Pipeline createGlHandle(
	Framebuffer framebuffer,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	PipelineHandle* pipelineHandle)
{
	Pipeline pipeline = createPipeline(
		framebuffer,
		TEXT_PIPELINE_NAME,
		shaders,
		shaderCount,
		state,
		NULL,
		onGlUniformsSet,
		onGlHandleResize,
		onGlHandleDestroy,
		pipelineHandle,
		NULL);

	if (pipeline == NULL)
		return NULL;

	GLuint glHandle = pipeline->gl.glHandle;

	GLint mvpLocation, colorLocation,
		textureLocation;

	bool result = getGlUniformLocation(
		glHandle,
		"u_MVP",
		&mvpLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Color",
		&colorLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Texture",
		&textureLocation);

	if (result == false)
	{
		destroyPipeline(pipeline, false);
		return NULL;
	}

	assertOpenGL();

	pipelineHandle->gl.mvpLocation = mvpLocation;
	pipelineHandle->gl.colorLocation = colorLocation;
	pipelineHandle->gl.textureLocation = textureLocation;
	return pipeline;
}

Pipeline createExtTextPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Sampler sampler,
	const PipelineState* state)
{
	assert(framebuffer != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(sampler != NULL);
	assert(vertexShader->base.type == VERTEX_SHADER_TYPE);
	assert(fragmentShader->base.type == FRAGMENT_SHADER_TYPE);
	assert(vertexShader->base.window == framebuffer->base.window);
	assert(fragmentShader->base.window == framebuffer->base.window);
	assert(sampler->base.window == framebuffer->base.window);

	PipelineHandle* pipelineHandle = malloc(
		sizeof(PipelineHandle));

	if (pipelineHandle == NULL)
		return NULL;

	Window window = framebuffer->vk.window;
	pipelineHandle->base.window = window;
	pipelineHandle->base.texture = NULL;
	pipelineHandle->base.sampler = sampler;
	pipelineHandle->base.vpc.mvp = identMat4F;
	pipelineHandle->base.fpc.color = whiteLinearColor;

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	GraphicsAPI api = getWindowGraphicsAPI(window);

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		return createVkHandle(
			framebuffer,
			shaders,
			2,
			state,
			pipelineHandle);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		return createGlHandle(
			framebuffer,
			shaders,
			2,
			state,
			pipelineHandle);
	}
	else
	{
		abort();
	}
}
Pipeline createTextPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Sampler sampler)
{
	assert(framebuffer != NULL);

	Vec2U framebufferSize =
		framebuffer->base.size;

	PipelineState state = {
		TRIANGLE_LIST_DRAW_MODE,
		FILL_POLYGON_MODE,
		BACK_CULL_MODE,
		LESS_COMPARE_OPERATOR,
		ALL_COLOR_COMPONENT,
		SRC_ALPHA_BLEND_FACTOR,
		ONE_MINUS_SRC_ALPHA_BLEND_FACTOR,
		ONE_BLEND_FACTOR,
		ZERO_BLEND_FACTOR,
		ADD_BLEND_OPERATOR,
		ADD_BLEND_OPERATOR,
		true,
		true,
		true,
		true,
		false,
		false,
		true,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		vec4I(0, 0,
			(int32_t)framebufferSize.x,
			(int32_t)framebufferSize.y),
		zeroVec4I,
		defaultDepthRange,
		defaultDepthBias,
		defaultBlendColor,
	};

	return createExtTextPipeline(
		framebuffer,
		vertexShader,
		fragmentShader,
		sampler,
		&state);
}

Sampler getTextPipelineSampler(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.sampler;
}

Mat4F getTextPipelineMVP(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.vpc.mvp;
}
void setTextPipelineMVP(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.vpc.mvp = mvp;
}

LinearColor getTextPipelineColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.fpc.color;
}
void setTextPipelineColor(
	Pipeline pipeline,
	LinearColor color)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.fpc.color = color;
}
