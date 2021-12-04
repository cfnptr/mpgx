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

#include "cmmt/common.h"
#include <assert.h>

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
	AlignmentType alignment;
	bool isConstant;
	uint32_t* data;
	size_t dataCapacity;
	size_t dataLength;
	Image texture;
	Mesh mesh;
	Vec2F textSize;
#if MPGX_SUPPORT_VULKAN
	VkDescriptorPool descriptorPool;
	VkDescriptorSet* descriptorSets;
#endif
};

typedef struct Glyph
{
	bool isVisible;
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
	Text* texts;
	size_t textCapacity;
	size_t textCount;
} BasePipelineHandle;
typedef struct VkPipelineHandle
{
	Window window;
	Image texture;
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
	Text* texts;
	size_t textCapacity;
	size_t textCount;
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
	Text* texts;
	size_t textCapacity;
	size_t textCount;
	GLint mvpLocation;
	GLint colorLocation;
	GLint textureLocation;
} GlPipelineHandle;
union PipelineHandle
{
	BasePipelineHandle base;
	VkPipelineHandle vk;
	GlPipelineHandle gl;
};

typedef union PipelineHandle* PipelineHandle;

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

	FT_Library ftLibrary = getFtLibrary();

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
	Glyph* glyphs = malloc(
		uniCharCount * sizeof(Glyph));

	if (glyphs == NULL)
		return false;

	size_t glyphCount = 0;

	for (size_t i = 0; i < uniCharCount; i++)
	{
		uint32_t uniChar = uniChars[i];

		if (uniChar == '\n')
			continue;
		else if (uniChar == '\t')
			uniChar = ' ';

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

	if (glyphCount == 0)
	{
		free(glyphs);
		return false;
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
	uint8_t** _pixels,
	size_t* _pixelCount,
	uint32_t* _pixelLength)
{
	uint32_t glyphLength = (uint32_t)ceilf(sqrtf((float)glyphCount));
	uint32_t pixelLength = glyphLength * fontSize;
	size_t pixelCount = (size_t)pixelLength * pixelLength;

	uint8_t* pixels = calloc(
		pixelCount,
		sizeof(uint8_t));

	if (pixels == NULL)
		return false;

	if (textPixelLength < pixelLength)
		textPixelLength = pixelLength;

	for (size_t i = 0; i < glyphCount; i++)
	{
		Glyph glyph;
		glyph.uniChar = glyphs[i].uniChar;

		FT_UInt charIndex = FT_Get_Char_Index(
			face,
			glyph.uniChar);
		FT_Error result = FT_Load_Glyph(
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

		uint32_t pixelPosY = (uint32_t)(i / glyphLength);
		uint32_t pixelPosX = (uint32_t)(i - pixelPosY * glyphLength);

		pixelPosX *= fontSize;
		pixelPosY *= fontSize;

		uint32_t glyphWidth = glyphSlot->bitmap.width;
		uint32_t glyphHeight = glyphSlot->bitmap.rows;

		if (glyphWidth * glyphHeight == 0)
		{
			glyph.isVisible = false;
			glyph.position = zeroVec4F;
			glyph.texCoords = zeroVec4F;
		}
		else
		{
			glyph.isVisible = true;
			glyph.position.x = (float)glyphSlot->bitmap_left / (float)fontSize;
			glyph.position.y = ((float)glyphSlot->bitmap_top - (float)glyphHeight) / (float)fontSize;
			glyph.position.z = glyph.position.x + (float)glyphWidth / (float)fontSize;
			glyph.position.w = glyph.position.y + (float)glyphHeight /(float)fontSize;
			glyph.texCoords.x = (float)pixelPosX / (float)textPixelLength;
			glyph.texCoords.y = (float)pixelPosY / (float)textPixelLength;
			glyph.texCoords.z = glyph.texCoords.x + (float)glyphWidth / (float)textPixelLength;
			glyph.texCoords.w = glyph.texCoords.y + (float)glyphHeight / (float)textPixelLength;

			for (size_t y = 0; y < glyphHeight; y++)
			{
				for (size_t x = 0; x < glyphWidth; x++)
				{
					pixels[(y + pixelPosY) * pixelLength + (x + pixelPosX)] =
						bitmap[y * glyphWidth + x];
				}
			}
		}

		glyph.advance = ((float)glyphSlot->advance.x / 64.0f) / (float)fontSize;
		glyphs[i] = glyph;
	}

	*_pixels = pixels;
	*_pixelCount = pixelCount;
	*_pixelLength = pixelLength;
	return true;
}
inline static bool createTextVertices(
	const uint32_t* uniChars,
	size_t uniCharCount,
	const Glyph* glyphs,
	size_t glyphCount,
	float newLineAdvance,
	AlignmentType alignment,
	float** _vertices,
	size_t* _vertexCount,
	Vec2F* _textSize)
{
	// TODO: use mapBuffer here
	size_t vertexCount = uniCharCount * 16;
	float* vertices = malloc(vertexCount * sizeof(float));

	if (vertices == NULL)
		return false;

	Vec2F vertexOffset = vec2F(
		0.0f, -newLineAdvance * 0.5f);
	Vec2F textSize = zeroVec2F;

	size_t vertexIndex = 0;

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
		else if (uniChar == '\t')
		{
			Glyph searchGlyph;
			searchGlyph.uniChar = ' ';

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

			vertexOffset.x += glyph->advance * 4;
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

		if (glyph->isVisible == true)
		{
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
		}

		vertexOffset.x += glyph->advance;
	}

	if (vertexIndex == 0)
	{
		free(vertices);
		return false;
	}

	if (textSize.x < vertexOffset.x)
		textSize.x = vertexOffset.x;

	textSize.y = -vertexOffset.y;

	// TODO: make also alignment inside text lines

	switch (alignment)
	{
	default:
		abort();
	case CENTER_ALIGNMENT_TYPE:
		vertexOffset.x = textSize.x * -0.5f;
		vertexOffset.y = textSize.y * 0.5f;

		for (size_t i = 0; i < vertexCount; i += 16)
		{
			vertices[i + 0] += vertexOffset.x;
			vertices[i + 1] += vertexOffset.y;
			vertices[i + 4] += vertexOffset.x;
			vertices[i + 5] += vertexOffset.y;
			vertices[i + 8] += vertexOffset.x;
			vertices[i + 9] += vertexOffset.y;
			vertices[i + 12] += vertexOffset.x;
			vertices[i + 13] += vertexOffset.y;
		}
		break;
	case LEFT_ALIGNMENT_TYPE:
		vertexOffset.y = textSize.y * 0.5f;

		for (size_t i = 0; i < vertexCount; i += 16)
		{
			vertices[i + 1] += vertexOffset.y;
			vertices[i + 5] += vertexOffset.y;
			vertices[i + 9] += vertexOffset.y;
			vertices[i + 13] += vertexOffset.y;
		}
		break;
	case RIGHT_ALIGNMENT_TYPE:
		vertexOffset.x = -textSize.x;
		vertexOffset.y = textSize.y * 0.5f;

		for (size_t i = 0; i < vertexCount; i += 16)
		{
			vertices[i + 0] += vertexOffset.x;
			vertices[i + 1] += vertexOffset.y;
			vertices[i + 4] += vertexOffset.x;
			vertices[i + 5] += vertexOffset.y;
			vertices[i + 8] += vertexOffset.x;
			vertices[i + 9] += vertexOffset.y;
			vertices[i + 12] += vertexOffset.x;
			vertices[i + 13] += vertexOffset.y;
		}
		break;
	case BOTTOM_ALIGNMENT_TYPE:
		vertexOffset.x = textSize.x * -0.5f;
		vertexOffset.y = textSize.y;

		for (size_t i = 0; i < vertexCount; i += 16)
		{
			vertices[i + 0] += vertexOffset.x;
			vertices[i + 1] += vertexOffset.y;
			vertices[i + 4] += vertexOffset.x;
			vertices[i + 5] += vertexOffset.y;
			vertices[i + 8] += vertexOffset.x;
			vertices[i + 9] += vertexOffset.y;
			vertices[i + 12] += vertexOffset.x;
			vertices[i + 13] += vertexOffset.y;
		}
		break;
	case TOP_ALIGNMENT_TYPE:
		vertexOffset.x = textSize.x * -0.5f;

		for (size_t i = 0; i < vertexCount; i += 16)
		{
			vertices[i + 0] += vertexOffset.x;
			vertices[i + 4] += vertexOffset.x;
			vertices[i + 8] += vertexOffset.x;
			vertices[i + 12] += vertexOffset.x;
		}
		break;
	case LEFT_BOTTOM_ALIGNMENT_TYPE:
		vertexOffset.y = textSize.y;

		for (size_t i = 0; i < vertexCount; i += 16)
		{
			vertices[i + 1] += vertexOffset.y;
			vertices[i + 5] += vertexOffset.y;
			vertices[i + 9] += vertexOffset.y;
			vertices[i + 13] += vertexOffset.y;
		}
		break;
	case LEFT_TOP_ALIGNMENT_TYPE:
		break;
	case RIGHT_BOTTOM_ALIGNMENT_TYPE:
		vertexOffset.x = -textSize.x;
		vertexOffset.y = textSize.y;

		for (size_t i = 0; i < vertexCount; i += 16)
		{
			vertices[i + 0] += vertexOffset.x;
			vertices[i + 1] += vertexOffset.y;
			vertices[i + 4] += vertexOffset.x;
			vertices[i + 5] += vertexOffset.y;
			vertices[i + 8] += vertexOffset.x;
			vertices[i + 9] += vertexOffset.y;
			vertices[i + 12] += vertexOffset.x;
			vertices[i + 13] += vertexOffset.y;
		}
		break;
	case RIGHT_TOP_ALIGNMENT_TYPE:
		vertexOffset.x = -textSize.x;

		for (size_t i = 0; i < vertexCount; i += 16)
		{
			vertices[i + 0] += vertexOffset.x;
			vertices[i + 4] += vertexOffset.x;
			vertices[i + 8] += vertexOffset.x;
			vertices[i + 12] += vertexOffset.x;
		}
		break;
	}

	*_vertices = vertices;
	*_vertexCount = vertexIndex;
	*_textSize = textSize;
	return true;
}
inline static bool createTextIndices(
	size_t vertexCount,
	uint32_t** _indices,
	size_t* _indexCount)
{
	size_t indexCount = (vertexCount / 16) * 6;

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

Text createText32(
	Pipeline pipeline,
	Font font,
	uint32_t fontSize,
	AlignmentType alignment,
	const uint32_t* _data,
	size_t dataLength,
	bool isConstant)
{
	assert(pipeline != NULL);
	assert(font != NULL);
	assert(fontSize != 0);
	assert(alignment >= CENTER_ALIGNMENT_TYPE);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	assert(_data != NULL);
	assert(dataLength != 0);

	assert(strcmp(
		pipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);

	Text text = malloc(
		sizeof(struct Text));

	if (text == NULL)
		return NULL;

	uint32_t* data = malloc(
		dataLength * sizeof(uint32_t));

	if (data == NULL)
	{
		free(text);
		return NULL;
	}

	memcpy(
		data,
		_data,
		dataLength * sizeof(uint32_t));

	Glyph* glyphs;
	size_t glyphCount;

	bool result = createTextGlyphs(
		_data,
		dataLength,
		&glyphs,
		&glyphCount);

	if (result == false)
	{
		free(data);
		free(text);
		return NULL;
	}

	FT_Face face = font->face;

	FT_Error ftResult = FT_Set_Pixel_Sizes(
		face,
		0,
		(FT_UInt)fontSize);

	if (ftResult != 0)
	{
		free(glyphs);
		free(data);
		free(text);
		return false;
	}

	float newLineAdvance =
		((float)face->size->metrics.height / 64.0f) /
		(float)fontSize;

	uint8_t* pixels;
	size_t pixelCount;
	uint32_t pixelLength;

	result = createTextPixels(
		face,
		fontSize,
		glyphs,
		glyphCount,
		0,
		&pixels,
		&pixelCount,
		&pixelLength);

	if (result == false)
	{
		free(glyphs);
		free(data);
		free(text);
		return NULL;
	}

	Window window = pipeline->base.framebuffer->base.window;

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
		free(glyphs);
		free(data);
		free(text);
		return NULL;
	}

	float* vertices;
	size_t vertexCount;
	Vec2F textSize;

	result = createTextVertices(
		data,
		dataLength,
		glyphs,
		glyphCount,
		newLineAdvance,
		alignment,
		&vertices,
		&vertexCount,
		&textSize);

	free(glyphs);

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
		vertexCount,
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

	GraphicsAPI api = getWindowGraphicsAPI(window);

#if MPGX_SUPPORT_VULKAN
	if (api == VULKAN_GRAPHICS_API)
	{
		VkWindow vkWindow = getVkWindow(window);
		VkDevice device = vkWindow->device;

		PipelineHandle pipelineHandle = pipeline->vk.handle;
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

	text->font = font;
	text->pipeline = pipeline;
	text->fontSize = fontSize;
	text->alignment = alignment;
	text->isConstant = isConstant;
	text->data = data;
	text->dataCapacity = dataLength;
	text->dataLength = dataLength;
	text->texture = texture;
	text->mesh = mesh;
	text->textSize = textSize;

	PipelineHandle pipelineHandle = pipeline->base.handle;
	size_t count = pipelineHandle->base.textCount;

	if (count == pipelineHandle->base.textCapacity)
	{
		size_t capacity = pipelineHandle->base.textCapacity * 2;

		Text* texts = realloc(
			pipelineHandle->base.texts,
			capacity * sizeof(Text));

		if (texts == NULL)
		{
#if MPGX_SUPPORT_VULKAN
			if (api == VULKAN_GRAPHICS_API)
			{
				free(text->descriptorSets);

				VkWindow vkWindow = getVkWindow(window);

				vkDestroyDescriptorPool(
					vkWindow->device,
					text->descriptorPool,
					NULL);
			}
#endif
			destroyMesh(text->mesh, true);
			destroyImage(text->texture);
			free(text->data);
			free(text);
			return NULL;
		}

		pipelineHandle->base.texts = texts;
		pipelineHandle->base.textCapacity = capacity;
	}

	pipelineHandle->base.texts[count] = text;
	pipelineHandle->base.textCount = count + 1;
	return text;
}
Text createText8(
	Pipeline pipeline,
	Font font,
	uint32_t fontSize,
	AlignmentType alignment,
	const char* data,
	size_t dataLength,
	bool isConstant)
{
	assert(data != NULL);
	assert(dataLength != 0);

	size_t uniCharCount = getTextUniCharCount(
		data,
		dataLength);

	if (uniCharCount == 0)
		return NULL;

	uint32_t* uniChars = createTextUniChars(
		data,
		dataLength,
		uniCharCount);

	if (uniChars == NULL)
		return NULL;

	Text text = createText32(
		pipeline,
		font,
		fontSize,
		alignment,
		uniChars,
		dataLength,
		isConstant);

	free(uniChars);

	if (text == NULL)
		return NULL;

	return text;
}
void destroyText(Text text)
{
	if (text == NULL)
		return;

	PipelineHandle pipelineHandle = text->pipeline->base.handle;
	Text* texts = pipelineHandle->base.texts;
	size_t textCount = pipelineHandle->base.textCount;

	for (size_t i = 0; i < textCount; i++)
	{
		if (texts[i] != text)
			continue;

		for (size_t j = i + 1; j < textCount; j++)
			texts[j - 1] = texts[j];

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
		destroyMesh(text->mesh, true);
		destroyImage(text->texture);

		free(text->data);
		free(text);

		pipelineHandle->base.textCount--;
		return;
	}

	abort();
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
Vec2F getTextOffset(Text text)
{
	assert(text != NULL);

	AlignmentType alignment = text->alignment;
	Vec2F offset = text->textSize;

	switch (alignment)
	{
	default:
		abort();
	case CENTER_ALIGNMENT_TYPE:
		return vec2F(
			-offset.x * 0.5f,
			offset.y * 0.5f);
	case LEFT_ALIGNMENT_TYPE:
		return vec2F(
			0.0f,
			offset.y * 0.5f);
	case RIGHT_ALIGNMENT_TYPE:
		return vec2F(
			-offset.x,
			offset.y * 0.5f);
	case BOTTOM_ALIGNMENT_TYPE:
		return vec2F(
			-offset.x * 0.5f,
			0.0f);
	case TOP_ALIGNMENT_TYPE:
		return vec2F(
			-offset.x * 0.5f,
			offset.y);
	case LEFT_BOTTOM_ALIGNMENT_TYPE:
		return vec2F(
			0.0f,
			0.0f);
	case LEFT_TOP_ALIGNMENT_TYPE:
		return vec2F(
			0.0f,
			offset.y);
	case RIGHT_BOTTOM_ALIGNMENT_TYPE:
		return vec2F(
			-offset.x,
			0.0f);
	case RIGHT_TOP_ALIGNMENT_TYPE:
		return vec2F(
			-offset.x,
			offset.y);
	}
}

/*bool getTextUnicodeCharAdvance(
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
}*/ // TODO: remove

bool getTextCaretAdvance(
	Text text,
	size_t index,
	Vec2F* _advance)
{
	assert(text != NULL);
	assert(_advance != NULL);

	if (index == 0)
	{
		*_advance = zeroVec2F;
		return true;
	}

	FT_Face face = text->font->face;
	uint32_t fontSize = text->fontSize;

	float newLineAdvance = ((float)face->size->metrics.height /
		64.0f) / (float)fontSize;

	const uint32_t* data = text->data;
	size_t dataLength = text->dataLength;

	if (index > dataLength)
		index = dataLength;

	Vec2F advance = zeroVec2F;

	for (size_t i = 0; i < index; i++)
	{
		uint32_t uniChar = data[i];

		if (uniChar == '\n')
		{
			advance.y += newLineAdvance;
			advance.x = 0.0f;
			continue;
		}
		else if (uniChar == '\t')
		{
			uniChar = ' ';

			FT_UInt charIndex = FT_Get_Char_Index(
				face,
				uniChar);
			FT_Error result = FT_Load_Glyph(
				face,
				charIndex,
				FT_LOAD_BITMAP_METRICS_ONLY);

			if (result != 0)
				return false;

			advance.x += ((float)face->glyph->advance.x /
				64.0f) / (float)fontSize * 4;
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

		advance.x += ((float)face->glyph->advance.x /
			64.0f) / (float)fontSize;
	}

	*_advance = advance;
	return true;
}
bool getTextCaretPosition(
	Text text,
	Vec2F* advance,
	size_t* index)
{
	assert(text != NULL);
	assert(advance != NULL);
	assert(index != NULL);

	// TODO:
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

AlignmentType getTextAlignment(
	Text text)
{
	assert(text != NULL);
	return text->alignment;
}
void setTextAlignment(
	Text text,
	AlignmentType alignment)
{
	assert(text != NULL);
	assert(text->isConstant == false);
	assert(alignment >= CENTER_ALIGNMENT_TYPE);
	assert(alignment < ALIGNMENT_TYPE_COUNT);
	text->alignment = alignment;
}

size_t getTextDataLength(Text text)
{
	assert(text != NULL);
	return text->dataLength;
}
const uint32_t* getTextData(
	Text text)
{
	assert(text != NULL);
	return text->data;
}

bool setTextData32(
	Text text,
	const uint32_t* _data,
	size_t dataLength,
	bool reuseBuffers)
{
	assert(text != NULL);
	assert(_data != NULL);
	assert(dataLength != 0);
	assert(text->isConstant == false);

	if (reuseBuffers == true)
	{
		if (dataLength > text->dataCapacity)
		{
			uint32_t* data = realloc(
				text->data,
				dataLength * sizeof(uint32_t));

			if (data == NULL)
				return false;

			memcpy(
				data,
				_data,
				dataLength * sizeof(uint32_t));

			text->data = data;
			text->dataCapacity = dataLength;
			text->dataLength = dataLength;
			return true;
		}
		else
		{
			memcpy(
				text->data,
				_data,
				dataLength * sizeof(uint32_t));
			text->dataLength = dataLength;
			return true;
		}
	}
	else
	{
		uint32_t* data = malloc(
			dataLength * sizeof(uint32_t));

		if (data == NULL)
			return false;

		free(text->data);

		text->data = data;
		text->dataCapacity = dataLength;
		text->dataLength = dataLength;
		return true;
	}
}
bool setTextData8(
	Text text,
	const char* data,
	size_t dataLength,
	bool reuseBuffers)
{
	assert(data != NULL);
	assert(dataLength != 0);
	assert(text->isConstant == false);

	size_t uniCharCount = getTextUniCharCount(
		data,
		dataLength);

	if (uniCharCount == 0)
		return NULL;

	uint32_t* uniChars = createTextUniChars(
		data,
		dataLength,
		uniCharCount);

	if (uniChars == NULL)
		return NULL;

	bool result = setTextData32(
		text,
		uniChars,
		uniCharCount,
		reuseBuffers);

	free(uniChars);
	return result;
}

bool bakeText(
	Text text,
	bool reuseBuffers)
{
	assert(text != NULL);
	assert(text->isConstant == false);

	Pipeline pipeline = text->pipeline;
	Window window = pipeline->base.framebuffer->base.window;
	GraphicsAPI api = getWindowGraphicsAPI(window);

	uint32_t* data = text->data;
	size_t dataLength = text->dataLength;

	if (reuseBuffers == true)
	{
		Glyph* glyphs;
		size_t glyphCount;

		bool result = createTextGlyphs(
			data,
			dataLength,
			&glyphs,
			&glyphCount);

		if (result == false)
			return false;

		FT_Face face = text->font->face;
		uint32_t fontSize = text->fontSize;

		FT_Error ftResult = FT_Set_Pixel_Sizes(
			face,
			0,
			(FT_UInt)fontSize);

		if (ftResult != 0)
		{
			free(glyphs);
			return false;
		}

		float newLineAdvance =
			((float)face->size->metrics.height / 64.0f) /
			(float)fontSize;

		uint32_t textPixelLength =
			getImageSize(text->texture).x;

		uint8_t* pixels;
		size_t pixelCount;
		uint32_t pixelLength;

		result = createTextPixels(
			face,
			fontSize,
			glyphs,
			glyphCount,
			textPixelLength,
			&pixels,
			&pixelCount,
			&pixelLength);

		if (result == false)
		{
			free(glyphs);
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
				return false;
			}

#if MPGX_SUPPORT_VULKAN
			if (api == VULKAN_GRAPHICS_API)
			{
				VkWindow vkWindow = getVkWindow(window);
				VkDevice device = vkWindow->device;
				PipelineHandle pipelineHandle = pipeline->vk.handle;

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
			data,
			dataLength,
			glyphs,
			glyphCount,
			newLineAdvance,
			text->alignment,
			&vertices,
			&vertexCount,
			&textSize);

		free(glyphs);

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
				vertexCount,
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
				PipelineHandle pipelineHandle = pipeline->vk.handle;

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
				(vertexCount / 16) * 6);

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
				(vertexCount / 16) * 6,
				0,
				indexBuffer);
		}

		text->textSize = textSize;
	}
	else
	{
		Glyph* glyphs;
		size_t glyphCount;

		bool result = createTextGlyphs(
			data,
			dataLength,
			&glyphs,
			&glyphCount);

		if (result == false)
			return false;

		FT_Face face = text->font->face;
		uint32_t fontSize = text->fontSize;

		FT_Error ftResult = FT_Set_Pixel_Sizes(
			face,
			0,
			(FT_UInt)fontSize);

		if (ftResult != 0)
		{
			free(glyphs);
			return false;
		}

		float newLineAdvance =
			((float)face->size->metrics.height / 64.0f) /
			(float)fontSize;

		uint8_t* pixels;
		size_t pixelCount;
		uint32_t pixelLength;

		result = createTextPixels(
			face,
			fontSize,
			glyphs,
			glyphCount,
			0,
			&pixels,
			&pixelCount,
			&pixelLength);

		if (result == false)
		{
			free(glyphs);
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
			free(glyphs);
			return false;
		}

		float* vertices;
		size_t vertexCount;
		Vec2F textSize;

		result = createTextVertices(
			data,
			dataLength,
			glyphs,
			glyphCount,
			newLineAdvance,
			text->alignment,
			&vertices,
			&vertexCount,
			&textSize);

		free(glyphs);

		if (result == false)
		{
			destroyImage(texture);
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
			return false;
		}

		uint32_t* indices;
		size_t indexCount;

		result = createTextIndices(
			vertexCount,
			&indices,
			&indexCount);

		if (result == false)
		{
			destroyBuffer(vertexBuffer);
			destroyImage(texture);
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
			return false;
		}

#if MPGX_SUPPORT_VULKAN
		if (api == VULKAN_GRAPHICS_API)
		{
			VkWindow vkWindow = getVkWindow(window);
			VkDevice device = vkWindow->device;
			PipelineHandle pipelineHandle = pipeline->vk.handle;
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

		text->texture = texture;
		text->mesh = mesh;
		text->textSize = textSize;
	}

	return true;
}
size_t drawText(
	Text text,
	Vec4U scissor)
{
	assert(text != NULL);

	Pipeline pipeline = text->pipeline;
	PipelineHandle textPipeline = pipeline->gl.handle;
	textPipeline->base.texture = text->texture;

	bool dynamicScissor = scissor.z + scissor.w != 0;

	Window window = pipeline->base.framebuffer->base.window;
	GraphicsAPI api = getWindowGraphicsAPI(window);

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		VkWindow vkWindow = getVkWindow(window);
		VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;

		if (dynamicScissor == true)
		{
			VkRect2D vkScissor = {
				(int32_t)scissor.x,
				(int32_t)scissor.y,
				scissor.z,
				scissor.w,
			};
			vkCmdSetScissor(
				commandBuffer,
				0,
				1,
				&vkScissor);
		}

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipeline->vk.layout,
			0,
			1,
			&text->descriptorSets[vkWindow->bufferIndex],
			0,
			NULL);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		if (dynamicScissor == true)
		{
			glScissor(
				(GLint)scissor.x,
				(GLint)scissor.y,
				(GLsizei)scissor.z,
				(GLsizei)scissor.w);
		}
	}
	else
	{
		abort();
	}

	return drawMesh(
		text->mesh,
		pipeline);
}

float getTextPlatformScale(Pipeline pipeline)
{
	assert(pipeline != NULL);

	assert(strcmp(
		pipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);

	Framebuffer framebuffer =
		getPipelineFramebuffer(pipeline);
	Vec2U framebufferSize =
		getFramebufferSize(framebuffer);
	Vec2U windowSize = getWindowSize(
		getFramebufferWindow(framebuffer));

	return max(
		(float)framebufferSize.x / (float)windowSize.x,
		(float)framebufferSize.y / (float)windowSize.y);
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
	PipelineHandle pipelineHandle = pipeline->vk.handle;
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
	PipelineHandle pipelineHandle = pipeline->vk.handle;
	Window window = pipelineHandle->vk.window;
	VkWindow vkWindow = getVkWindow(window);
	uint32_t bufferCount = vkWindow->swapchain->bufferCount;

	if (bufferCount != pipelineHandle->vk.bufferCount)
	{
		Text* texts = pipelineHandle->vk.texts;
		size_t textCount = pipelineHandle->vk.textCount;
		VkDevice device = vkWindow->device;

		for (size_t i = 0; i < textCount; i++)
		{
			Text text = texts[i];

			VkDescriptorPool descriptorPool = createVkDescriptorPool(
				device,
				bufferCount);

			if (descriptorPool == NULL)
				return false;

			VkDescriptorSet* descriptorSets = createVkDescriptorSets(
				device,
				pipelineHandle->vk.descriptorSetLayout,
				descriptorPool,
				bufferCount,
				pipelineHandle->vk.sampler->vk.handle,
				text->texture->vk.imageView);

			if (descriptorSets == NULL)
			{
				vkDestroyDescriptorPool(
					device,
					descriptorPool,
					NULL);
				return false;
			}

			free(text->descriptorSets);

			vkDestroyDescriptorPool(
				device,
				text->descriptorPool,
				NULL);

			text->descriptorPool = descriptorPool;
			text->descriptorSets = descriptorSets;

		}

		pipelineHandle->vk.bufferCount = bufferCount;
	}

	Vec4U size = vec4U(0, 0,
		newSize.x, newSize.y);

	bool dynamic = pipeline->vk.state.viewport.z +
		pipeline->vk.state.viewport.w == 0;
	if (dynamic == false)
		pipeline->vk.state.viewport = size;

	dynamic = pipeline->vk.state.scissor.z +
		pipeline->vk.state.scissor.w == 0;
	if (dynamic == false)
		pipeline->vk.state.scissor = size;

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
	PipelineHandle pipelineHandle = handle;
	VkWindow vkWindow = getVkWindow(pipelineHandle->vk.window);
	VkDevice device = vkWindow->device;

	vkDestroyDescriptorSetLayout(
		device,
		pipelineHandle->vk.descriptorSetLayout,
		NULL);

	assert(pipelineHandle->vk.textCount == 0);

	free(pipelineHandle->vk.texts);
	free(pipelineHandle);
}
inline static Pipeline createVkHandle(
	Framebuffer framebuffer,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	PipelineHandle pipelineHandle)
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
	PipelineHandle pipelineHandle = pipeline->gl.handle;

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
	Vec4U size = vec4U(0, 0,
		newSize.x, newSize.y);

	bool dynamic = pipeline->vk.state.viewport.z +
		pipeline->vk.state.viewport.w == 0;
	if (dynamic == false)
		pipeline->vk.state.viewport = size;

	dynamic = pipeline->vk.state.scissor.z +
		pipeline->vk.state.scissor.w == 0;
	if (dynamic == false)
		pipeline->vk.state.scissor = size;
	return true;
}
static void onGlHandleDestroy(void* handle)
{
	PipelineHandle pipelineHandle = handle;

	assert(pipelineHandle->gl.textCount == 0);

	free(pipelineHandle->gl.texts);
	free(pipelineHandle);
}
inline static Pipeline createGlHandle(
	Framebuffer framebuffer,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	PipelineHandle pipelineHandle)
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
	const PipelineState* state,
	size_t textCapacity)
{
	assert(framebuffer != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(sampler != NULL);
	assert(textCapacity != 0);
	assert(vertexShader->base.type == VERTEX_SHADER_TYPE);
	assert(fragmentShader->base.type == FRAGMENT_SHADER_TYPE);
	assert(vertexShader->base.window == framebuffer->base.window);
	assert(fragmentShader->base.window == framebuffer->base.window);
	assert(sampler->base.window == framebuffer->base.window);

	PipelineHandle pipelineHandle = malloc(
		sizeof(union PipelineHandle));

	if (pipelineHandle == NULL)
		return NULL;

	Text* texts = malloc(
		textCapacity * sizeof(Text));

	if (texts == NULL)
	{
		free(pipelineHandle);
		return NULL;
	}

	Window window = framebuffer->vk.window;
	pipelineHandle->base.window = window;
	pipelineHandle->base.texture = NULL;
	pipelineHandle->base.sampler = sampler;
	pipelineHandle->base.vpc.mvp = identMat4F;
	pipelineHandle->base.fpc.color = whiteLinearColor;
	pipelineHandle->base.texts = texts;
	pipelineHandle->base.textCapacity = textCapacity;
	pipelineHandle->base.textCount = 0;

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
	Sampler sampler,
	size_t textCapacity,
	bool useScissor)
{
	assert(framebuffer != NULL);

	Vec2U framebufferSize =
		framebuffer->base.size;
	Vec4U size = vec4U(0, 0,
		framebufferSize.x,
		framebufferSize.y);

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
		size,
		useScissor ? zeroVec4U : size,
		defaultDepthRange,
		defaultDepthBias,
		defaultBlendColor,
	};

	return createExtTextPipeline(
		framebuffer,
		vertexShader,
		fragmentShader,
		sampler,
		&state,
		textCapacity);
}

Sampler getTextPipelineSampler(Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXT_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
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
	PipelineHandle pipelineHandle =
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
	PipelineHandle pipelineHandle =
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
	PipelineHandle pipelineHandle =
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
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.fpc.color = color;
}
