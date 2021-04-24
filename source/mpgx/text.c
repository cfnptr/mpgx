#include "mpgx/text.h"
#include "mpgx/pipeline.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include <wchar.h>
#include <assert.h>

// TODO: make better look

struct Font
{
	FT_Face face;
};

struct Text
{
	Window* window;
	Font* font;
	uint32_t fontSize;
	char* data;
	size_t dataSize;
	bool constant;
	Image* image;
	Mesh* mesh;
	Vec2F textSize;
	size_t uniCharCount;
};

typedef struct Glyph
{
	uint32_t uniChar;
	Vec4F position;
	Vec4F texCoords;
	float advance;
} Glyph;

typedef struct VkTextPipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
	Image* image;
	Mat4F mvp;
	Vec4F color;
	// TODO:
} VkTextPipeline;
typedef struct GlTextPipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
	Image* image;
	Mat4F mvp;
	Vec4F color;
	GLenum handle;
	GLint mvpLocation;
	GLint colorLocation;
	GLint imageLocation;
} GlTextPipeline;
typedef union TextPipeline
{
	VkTextPipeline vk;
	GlTextPipeline gl;
} TextPipeline;

Font* createFontFromFile(
	const void* filePath)
{
	assert(filePath != NULL);

	Font* font = malloc(
		sizeof(Font));

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
		free(font);
		return NULL;
	}

	font->face = face;
	return font;
}
void destroyFont(Font* font)
{
	if (font == NULL)
		return;

	FT_Done_Face(font->face);
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
	size_t pixelCount = pixelLength * pixelLength;

	uint8_t* pixels = malloc(
		pixelCount * 4 * sizeof(uint8_t));

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
				size_t pixelPos =
					(y + pixelPosY) * 4 * pixelLength +
					(x + pixelPosX) * 4;

				// TODO: possibly optimize with texture packing
				pixels[pixelPos + 0] = 255;
				pixels[pixelPos + 1] = 255;
				pixels[pixelPos + 2] = 255;
				pixels[pixelPos + 3] = bitmap[y * glyphWidth + x];
			}
		}
	}

	*_newLineAdvance = newLineAdvance;
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
	float** _vertices,
	size_t* _vertexCount,
	Vec2F* _textSize)
{
	size_t vertexCount =
		uniCharCount * 16;
	float* vertices = malloc(
		vertexCount * sizeof(float));

	if (vertices == NULL)
		return false;

	size_t vertexIndex = 0;
	Vec2F textSize = zeroVec2F();

	for (size_t i = 0; i < uniCharCount; i++)
	{
		uint32_t uniChar = uniChars[i];

		if (uniChar == '\n')
		{
			textSize.y -= newLineAdvance;
			textSize.x = 0.0f;
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
			textSize.x + glyph->position.x,
			textSize.y + glyph->position.y,
			textSize.x + glyph->position.z,
			textSize.y + glyph->position.w);
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
		textSize.x += glyph->advance;
	}

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
Text* createText(
	Window* window,
	Font* font,
	uint32_t fontSize,
	const char* _data,
	bool constant)
{
	// TODO:
	// Bind different descriptor sets
	// in Vulkan graphics API

	assert(window != NULL);
	assert(font != NULL);
	assert(fontSize > 0);
	assert(_data != NULL);

	Text* text = malloc(sizeof(Text));

	if (text == NULL)
		return NULL;

	size_t dataLength = strlen(_data);

	size_t uniCharCount = getTextUniCharCount(
		_data,
		dataLength);

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

		Image* image = createImage(
			window,
			IMAGE_2D_TYPE,
			R8G8B8A8_UNORM_IMAGE_FORMAT,
			vec3U(fontSize, fontSize, 1),
			(const void**)&pixels,
			1);

		if (image == NULL)
		{
			free(data);
			free(text);
			return NULL;
		}

		Buffer* vertexBuffer = createBuffer(
			window,
			VERTEX_BUFFER_TYPE,
			NULL,
			16,
			constant);

		if (vertexBuffer == NULL)
		{
			destroyImage(image);
			free(data);
			free(text);
			return NULL;
		}

		Buffer* indexBuffer = createBuffer(
			window,
			INDEX_BUFFER_TYPE,
			NULL,
			6,
			constant);

		if (indexBuffer == NULL)
		{
			destroyBuffer(vertexBuffer);
			destroyImage(image);
			free(data);
			free(text);
			return NULL;
		}

		Mesh* mesh = createMesh(
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
			destroyImage(image);
			free(data);
			free(text);
			return NULL;
		}

		text->data = data;
		text->dataSize = 1;
		text->image = image;
		text->mesh = mesh;
		text->textSize = zeroVec2F();
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

		Image* image = createImage(
			window,
			IMAGE_2D_TYPE,
			R8G8B8A8_UNORM_IMAGE_FORMAT,
			vec3U(pixelLength, pixelLength, 1),
			(const void**)&pixels,
			1);

		free(pixels);

		if (image == NULL)
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
			destroyImage(image);
			free(data);
			free(text);
			return NULL;
		}

		Buffer* vertexBuffer = createBuffer(
			window,
			VERTEX_BUFFER_TYPE,
			vertices,
			vertexCount * sizeof(float),
			constant);

		free(vertices);

		if (vertexBuffer == NULL)
		{
			destroyImage(image);
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
			destroyImage(image);
			free(data);
			free(text);
			return NULL;
		}

		Buffer* indexBuffer = createBuffer(
			window,
			INDEX_BUFFER_TYPE,
			indices,
			indexCount * sizeof(uint32_t),
			constant);

		free(indices);

		if (indexBuffer == NULL)
		{
			destroyBuffer(vertexBuffer);
			destroyImage(image);
			free(data);
			free(text);
			return NULL;
		}

		Mesh* mesh = createMesh(
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
			destroyImage(image);
			free(data);
			free(text);
			return NULL;
		}

		text->data = data;
		text->dataSize = dataSize;
		text->image = image;
		text->mesh = mesh;
		text->textSize = textSize;
		text->uniCharCount = uniCharCount;
	}

	text->window = window;
	text->font = font;
	text->fontSize = fontSize;
	text->constant = constant;
	return text;
}
void destroyText(Text* text)
{
	if (text == NULL)
		return;

	Mesh* mesh = text->mesh;
	Buffer* vertexBuffer = getMeshVertexBuffer(mesh);
	Buffer* indexBuffer = getMeshIndexBuffer(mesh);

	destroyMesh(mesh);
	destroyBuffer(indexBuffer);
	destroyBuffer(vertexBuffer);

	destroyImage(text->image);
	free(text->data);

	free(text);
}

Window* getTextWindow(const Text* text)
{
	assert(text != NULL);
	return text->window;
}
bool isTextConstant(const Text* text)
{
	assert(text != NULL);
	return text->constant;
}
Vec2F getTextSize(const Text* text)
{
	assert(text != NULL);
	return text->textSize;
}

size_t getTextUnicodeCharCount(
	const Text* text)
{
	assert(text != NULL);
	return text->uniCharCount;
}
bool getTextUnicodeCharAdvance(
	const Text* text,
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

	Vec2F advance = zeroVec2F();

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

Font* getTextFont(
	const Text* text)
{
	assert(text != NULL);
	return text->font;
}
void setTextFont(
	Text* text,
	Font* font)
{
	assert(text != NULL);
	assert(font != NULL);
	assert(text->constant == false);
	text->font = font;
}

uint32_t getTextFontSize(
	const Text* text)
{
	assert(text != NULL);
	return text->fontSize;
}
void setTextFontSize(
	Text* text,
	uint32_t fontSize)
{
	assert(text != NULL);
	assert(text->constant == false);
	text->fontSize = fontSize;
}

const char* getTextData(
	const Text* text)
{
	assert(text != NULL);
	return text->data;
}
bool setTextData(
	Text* text,
	const char* _data)
{
	assert(text != NULL);
	assert(_data != NULL);
	assert(text->constant == false);

	size_t dataSize =
		strlen(_data) + 1;

	if (dataSize > text->dataSize)
	{
		char* data = realloc(
			text->data,
			dataSize * sizeof(char));

		if (data == NULL)
			return false;

		memcpy(
			data,
			_data,
			dataSize * sizeof(char));

		text->data = data;
		text->dataSize = dataSize;
		return true;
	}
	else
	{
		memcpy(
			text->data,
			_data,
			dataSize * sizeof(char));
		return true;
	}
}

bool bakeText(
	Text* text,
	bool reuse)
{
	assert(text != NULL);
	assert(text->constant == false);

	Window* window =
		text->window;
	const char* _data =
		text->data;

	size_t dataLength =
		strlen(_data);
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
			text->textSize = zeroVec2F();
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
				getImageSize(text->image).x;

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

			Image* image = NULL;

			if (pixelLength > textPixelLength)
			{
				image = createImage(
					window,
					IMAGE_2D_TYPE,
					R8G8B8A8_UNORM_IMAGE_FORMAT,
					vec3U(pixelLength, pixelLength, 1),
					(const void**)&pixels,
					1);

				free(pixels);
				pixels = NULL;

				if (image == NULL)
				{
					free(glyphs);
					free(uniChars);
					return false;
				}
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
				destroyImage(image);
				free(pixels);
				return false;
			}

			Buffer* vertexBuffer = NULL;
			Buffer* indexBuffer = NULL;

			size_t textVertexBufferSize = getBufferSize(
				getMeshVertexBuffer(text->mesh));

			if (vertexCount * sizeof(float) > textVertexBufferSize)
			{
				vertexBuffer = createBuffer(
					window,
					VERTEX_BUFFER_TYPE,
					vertices,
					vertexCount * sizeof(float),
					text->constant);

				free(vertices);

				if (vertexBuffer == NULL)
				{
					destroyImage(image);
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
					destroyImage(image);
					free(pixels);
					return false;
				}

				indexBuffer = createBuffer(
					window,
					INDEX_BUFFER_TYPE,
					indices,
					indexCount * sizeof(uint32_t),
					text->constant);

				free(indices);

				if (indexBuffer == NULL)
				{
					destroyBuffer(vertexBuffer);
					destroyImage(image);
					free(pixels);
					return false;
				}
			}

			if (image == NULL)
			{
				setImageData(
					text->image,
					pixels,
					vec3U(pixelLength, pixelLength, 1),
					zeroVec3U());

				free(pixels);
			}
			else
			{
				destroyImage(text->image);
				text->image = image;
			}

			if (vertexBuffer == NULL)
			{
				Buffer* _vertexBuffer =
					getMeshVertexBuffer(text->mesh);

				setBufferData(
					_vertexBuffer,
					vertices,
					vertexCount * sizeof(float),
					0);
				setMeshIndexCount(
					text->mesh,
					uniCharCount * 6);

				free(vertices);
			}
			else
			{
				Mesh* mesh = text->mesh;
				Buffer* _vertexBuffer = getMeshVertexBuffer(mesh);
				Buffer* _indexBuffer = getMeshIndexBuffer(mesh);

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

			Image* image = createImage(
				window,
				IMAGE_2D_TYPE,
				R8G8B8A8_UNORM_IMAGE_FORMAT,
				vec3U(text->fontSize, text->fontSize, 1),
				(const void**)&pixels,
				1);

			if (image == NULL)
			{
				free(data);
				return false;
			}

			Buffer* vertexBuffer = createBuffer(
				window,
				VERTEX_BUFFER_TYPE,
				NULL,
				16,
				text->constant);

			if (vertexBuffer == NULL)
			{
				destroyImage(image);
				free(data);
				return false;
			}

			Buffer* indexBuffer = createBuffer(
				window,
				INDEX_BUFFER_TYPE,
				NULL,
				6,
				text->constant);

			if (indexBuffer == NULL)
			{
				destroyBuffer(vertexBuffer);
				destroyImage(image);
				free(data);
				return false;
			}

			Mesh* mesh = createMesh(
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
				destroyImage(image);
				free(data);
				return false;
			}

			Mesh* _mesh = text->mesh;
			vertexBuffer = getMeshVertexBuffer(_mesh);
			indexBuffer = getMeshIndexBuffer(_mesh);

			destroyMesh(_mesh);
			destroyBuffer(vertexBuffer);
			destroyBuffer(indexBuffer);

			destroyImage(text->image);
			free(text->data);

			text->data = data;
			text->dataSize = 1;
			text->image = image;
			text->mesh = mesh;
			text->textSize = zeroVec2F();
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

			Image* image = createImage(
				window,
				IMAGE_2D_TYPE,
				R8G8B8A8_UNORM_IMAGE_FORMAT,
				vec3U(pixelLength, pixelLength, 1),
				(const void**)&pixels,
				1);

			free(pixels);

			if (image == NULL)
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
				destroyImage(image);
				free(data);
				return false;
			}

			Buffer* vertexBuffer = createBuffer(
				window,
				VERTEX_BUFFER_TYPE,
				vertices,
				vertexCount * sizeof(float),
				text->constant);

			free(vertices);

			if (vertexBuffer == NULL)
			{
				destroyImage(image);
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
				destroyImage(image);
				free(data);
				return false;
			}

			Buffer* indexBuffer = createBuffer(
				window,
				INDEX_BUFFER_TYPE,
				indices,
				indexCount * sizeof(uint32_t),
				text->constant);

			free(indices);

			if (indexBuffer == NULL)
			{
				destroyBuffer(vertexBuffer);
				destroyImage(image);
				free(data);
				return false;
			}

			Mesh* mesh = createMesh(
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
				destroyImage(image);
				free(data);
				return false;
			}

			Mesh* _mesh = text->mesh;
			vertexBuffer = getMeshVertexBuffer(_mesh);
			indexBuffer = getMeshIndexBuffer(_mesh);

			destroyMesh(_mesh);
			destroyBuffer(vertexBuffer);
			destroyBuffer(indexBuffer);

			destroyImage(text->image);
			free(text->data);

			text->data = data;
			text->dataSize = dataSize;
			text->image = image;
			text->mesh = mesh;
			text->textSize = textSize;
			text->uniCharCount = uniCharCount;
		}
	}

	return true;
}
void drawText(
	Text* text,
	Pipeline* pipeline)
{
	assert(text != NULL);
	assert(pipeline != NULL);

	assert(text->window ==
		getPipelineWindow(pipeline));
	assert(strcmp(
		getPipelineName(pipeline),
		"Text") == 0);

	TextPipeline* textPipeline =
		getPipelineHandle(pipeline);

	textPipeline->vk.image =
		text->image;
	drawMesh(
		text->mesh,
		pipeline);
}

inline static TextPipeline* onGlTextPipelineCreate(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader)
{
	TextPipeline* pipeline = malloc(
		sizeof(TextPipeline));

	if (pipeline == NULL)
		return NULL;

	Shader* shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	makeWindowContextCurrent(window);

	GLuint handle = createGlPipeline(
		shaders,
		2);

	if (handle == GL_ZERO)
	{
		free(pipeline);
		return NULL;
	}

	GLint mvpLocation = glGetUniformLocation(
		handle,
		"u_MVP");

	if (mvpLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_MVP' location\n");
#endif
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	GLint colorLocation = glGetUniformLocation(
		handle,
		"u_Color");

	if (colorLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_Color' location\n");
#endif
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	GLint imageLocation = glGetUniformLocation(
		handle,
		"u_Image");

	if (imageLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_Image' location\n");
#endif
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	assertOpenGL();

	pipeline->gl.vertexShader = vertexShader;
	pipeline->gl.fragmentShader = fragmentShader;
	pipeline->gl.image = NULL;
	pipeline->gl.mvp = identMat4F();
	pipeline->gl.color = valVec4F(1.0f);
	pipeline->gl.handle = handle;
	pipeline->gl.mvpLocation = mvpLocation;
	pipeline->gl.colorLocation = colorLocation;
	pipeline->gl.imageLocation = imageLocation;
	return pipeline;
}
static void onGlTextPipelineDestroy(
	Window* window,
	void* pipeline)
{
	TextPipeline* textPipeline =
		(TextPipeline*)pipeline;

	makeWindowContextCurrent(window);

	glDeleteProgram(
		textPipeline->gl.handle);
	assertOpenGL();

	free(textPipeline);
}
static void onGlTextPipelineBind(
	Pipeline* pipeline)
{
	TextPipeline* textPipeline =
		getPipelineHandle(pipeline);

	glUseProgram(textPipeline->gl.handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	glBlendFunc(
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA);

	assertOpenGL();
}
static void onGlTextUniformsSet(
	Pipeline* pipeline)
{
	TextPipeline* textPipeline =
		getPipelineHandle(pipeline);

	glUniformMatrix4fv(
		textPipeline->gl.mvpLocation,
		1,
		GL_FALSE,
		(const float*)&textPipeline->gl.mvp);
	glUniform4fv(
		textPipeline->gl.colorLocation,
		1,
		(const float*)&textPipeline->gl.color);

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

	GLuint glImage = *(const GLuint*)
		getImageHandle(textPipeline->gl.image);

	// TODO: use sampler

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(
		GL_TEXTURE_2D,
		glImage);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_S,
		GL_REPEAT);
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_T,
		GL_REPEAT);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_NEAREST);
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_NEAREST);

	glUniform1i(
		textPipeline->gl.imageLocation,
		0);

	assertOpenGL();
}
Pipeline* createTextPipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	uint8_t drawMode)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(getShaderType(vertexShader) == VERTEX_SHADER_TYPE);
	assert(getShaderType(fragmentShader) == FRAGMENT_SHADER_TYPE);
	assert(getShaderWindow(vertexShader) == window);
	assert(getShaderWindow(fragmentShader) == window);

	uint8_t api = getWindowGraphicsAPI(window);

	TextPipeline* handle;
	OnPipelineDestroy onDestroy;
	OnPipelineBind onBind;
	OnPipelineUniformsSet onUniformsSet;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = onGlTextPipelineCreate(
			window,
			vertexShader,
			fragmentShader);

		onDestroy = onGlTextPipelineDestroy;
		onBind = onGlTextPipelineBind;
		onUniformsSet = onGlTextUniformsSet;
	}
	else
	{
		return NULL;
	}

	if (handle == NULL)
		return NULL;

	Pipeline* pipeline = createPipeline(
		window,
		"Text",
		drawMode,
		onDestroy,
		onBind,
		onUniformsSet,
		handle);

	if (pipeline == NULL)
	{
		onDestroy(
			window,
			handle);
		return NULL;
	}

	return pipeline;
}

Shader* getTextPipelineVertexShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Text") == 0);
	TextPipeline* textPipeline =
		getPipelineHandle(pipeline);
	return textPipeline->vk.vertexShader;
}
Shader* getTextPipelineFragmentShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Text") == 0);
	TextPipeline* textPipeline =
		getPipelineHandle(pipeline);
	return textPipeline->vk.fragmentShader;
}

Vec4F getTextPipelineColor(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Text") == 0);
	TextPipeline* textPipeline =
		getPipelineHandle(pipeline);
	return textPipeline->vk.color;
}
void setTextPipelineColor(
	Pipeline* pipeline,
	Vec4F color)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Text") == 0);
	TextPipeline* textPipeline =
		getPipelineHandle(pipeline);
	textPipeline->vk.color = color;
}

Mat4F getTextPipelineMVP(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Text") == 0);
	TextPipeline* textPipeline =
		getPipelineHandle(pipeline);
	return textPipeline->vk.mvp;
}
void setTextPipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Text") == 0);
	TextPipeline* textPipeline =
		getPipelineHandle(pipeline);
	textPipeline->vk.mvp = mvp;
}
