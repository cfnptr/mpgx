#include "mpgx/text.h"
#include "mpgx/opengl.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include <wchar.h>
#include <assert.h>

struct Font
{
	FT_Face face;
};

struct Text
{
	struct Window* window;
	struct Font* font;
	size_t fontSize;
	struct Pipeline* pipeline;
	char* data;
	size_t dataSize;
	bool constant;
	struct Image* image;
	struct Mesh* mesh;
};
struct Glyph
{
	uint32_t uniChar;
	float posX;
	float posY;
	float posZ;
	float posW;
	float advance;
	float texCoordU;
	float texCoordV;
	float texCoordS;
	float texCoordT;
};

struct GlTextPipeline
{
	GLenum handle;
	GLint mvpLocation;
	GLint colorLocation;
	GLint imageLocation;
};
struct TextPipeline
{
	struct Matrix4F mvp;
	struct Vector4F color;
	struct Image* image;
	void* handle;
};

struct Font* createFont(
	const void* filePath)
{
	assert(filePath != NULL);

	struct Font* font =
		malloc(sizeof(struct Font));

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
void destroyFont(
	struct Font* font)
{
	if (font == NULL)
		return;

	FT_Done_Face(font->face);
	free(font);
}

int compareGlyph(
	const void* a,
	const void* b)
{
	if (((struct Glyph*)a)->uniChar <
		((struct Glyph*)b)->uniChar)
	{
		return -1;
	}
	if (((struct Glyph*)a)->uniChar ==
		((struct Glyph*)b)->uniChar)
	{
		return 0;
	}
	if (((struct Glyph*)a)->uniChar >
		((struct Glyph*)b)->uniChar)
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
		return false;

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
			return false;
		}
	}

	return uniChars;
}
inline static bool createTextGlyphs(
	const uint32_t* uniChars,
	size_t uniCharCount,
	struct Glyph** _glyphs,
	size_t* _glyphCount)
{
	size_t glyphCount = 0;

	struct Glyph* glyphs = malloc(
		uniCharCount * sizeof(struct Glyph));

	if (glyphs == NULL)
		return false;

	for (size_t i = 0; i < uniCharCount; i++)
	{
		uint32_t uniChar = uniChars[i];
		bool glyphExists = false;

		// TODO: optimize with binary search
		for (size_t j = 0; j < glyphCount; j++)
		{
			if (uniChar == glyphs[j].uniChar)
			{
				glyphExists = true;
				break;
			}
		}

		if (glyphExists == false)
		{
			glyphs[glyphCount].uniChar = uniChar;
			glyphCount++;
		}
	}

	qsort(
		glyphs,
		glyphCount,
		sizeof(struct Glyph),
		compareGlyph);

	*_glyphs = glyphs;
	*_glyphCount = glyphCount;
	return true;
}
inline static bool createTextPixels(
	FT_Face face,
	size_t fontSize,
	struct Glyph* glyphs,
	size_t glyphCount,
	size_t textPixelLength,
	uint8_t** _pixels,
	size_t* _pixelCount,
	size_t* _pixelLength)
{
	size_t glyphLength =
		((size_t)sqrtf(glyphCount) + 1);
	size_t pixelLength =
		glyphLength * fontSize;
	size_t pixelCount =
		pixelLength * pixelLength;

	// TODO: get max image size
	if (pixelLength > 2048)
		return NULL;

	uint8_t* pixels = malloc(
		pixelCount * sizeof(uint8_t) * 4);

	if (pixels == NULL)
		return false;

	FT_Error result = FT_Set_Pixel_Sizes(
		face,
		0,
		fontSize);

	if (result != 0)
	{
		free(pixels);
		return NULL;
	}

	if (textPixelLength < pixelLength)
		textPixelLength = pixelLength;

	for (size_t i = 0; i < glyphCount; i++)
	{
		struct Glyph glyph;
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
		glyph.posX = (float)glyphSlot->bitmap_left / fontSize;
		glyph.posY = ((float)glyphSlot->bitmap_top - glyphHeight) / fontSize;
		glyph.posZ = glyph.posX + (float)glyphWidth / fontSize;
		glyph.posW = glyph.posY + (float)glyphHeight / fontSize;
		glyph.advance = (glyphSlot->advance.x / 64.0f) / fontSize;
		glyph.texCoordU = (float)pixelPosX / textPixelLength;
		glyph.texCoordV = (float)pixelPosY / textPixelLength;
		glyph.texCoordS = glyph.texCoordU + (float)glyphWidth / textPixelLength;
		glyph.texCoordT = glyph.texCoordV + (float)glyphHeight / textPixelLength;

		glyphs[i] = glyph;

		for (size_t y = 0; y < glyphHeight; y++)
		{
			for (size_t x = 0; x < glyphWidth; x++)
			{
				size_t pixelPos =
					(y + pixelPosY) * 4 * pixelLength +
					(x + pixelPosX) * 4;
				uint8_t value = bitmap[y * glyphWidth + x];

				// TODO: possibly optimize with texture packing
				pixels[pixelPos + 0] = 255;
				pixels[pixelPos + 1] = 0;
				pixels[pixelPos + 2] = 255;
				pixels[pixelPos + 3] = value;
			}
		}
	}

	*_pixels = pixels;
	*_pixelCount = pixelCount;
	*_pixelLength = pixelLength;
	return true;
}
inline static bool createTextVertices(
	const uint32_t* uniChars,
	size_t uniCharCount,
	const struct Glyph* glyphs,
	size_t glyphCount,
	float** _vertices,
	size_t* _vertexCount)
{
	size_t vertexCount =
		uniCharCount * 16;
	float* vertices = malloc(
		vertexCount * sizeof(float));

	if (vertices == NULL)
		return false;

	size_t vertexIndex = 0;
	float vertexGlyphPosX = 0.0f;
	float vertexGlyphPosY = 0.0f;

	for (size_t i = 0; i < uniCharCount; i++)
	{
		struct Glyph searchGlyph;
		searchGlyph.uniChar = uniChars[i];

		struct Glyph* glyph = bsearch(
			&searchGlyph,
			glyphs,
			glyphCount,
			sizeof(struct Glyph),
			compareGlyph);

		if (glyph == NULL)
		{
			free(vertices);
			return false;
		}

		float glyphPosX = vertexGlyphPosX + glyph->posX;
		float glyphPosY = vertexGlyphPosY + glyph->posY;
		float glyphPosZ = vertexGlyphPosX + glyph->posZ;
		float glyphPosW = vertexGlyphPosY + glyph->posW;
		float texCoordU = glyph->texCoordU;
		float texCoordV = glyph->texCoordV;
		float texCoordS = glyph->texCoordS;
		float texCoordT = glyph->texCoordT;

		vertices[vertexIndex + 0] = glyphPosX;
		vertices[vertexIndex + 1] = glyphPosY;
		vertices[vertexIndex + 2] = texCoordU;
		vertices[vertexIndex + 3] = texCoordT;
		vertices[vertexIndex + 4] = glyphPosX;
		vertices[vertexIndex + 5] = glyphPosW;
		vertices[vertexIndex + 6] = texCoordU;
		vertices[vertexIndex + 7] = texCoordV;
		vertices[vertexIndex + 8] = glyphPosZ;
		vertices[vertexIndex + 9] = glyphPosW;
		vertices[vertexIndex + 10] = texCoordS;
		vertices[vertexIndex + 11] = texCoordV;
		vertices[vertexIndex + 12] = glyphPosZ;
		vertices[vertexIndex + 13] = glyphPosY;
		vertices[vertexIndex + 14] = texCoordS;
		vertices[vertexIndex + 15] = texCoordT;

		vertexIndex += 16;
		vertexGlyphPosX += glyph->advance;
	}

	*_vertices = vertices;
	*_vertexCount = vertexCount;
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
		indices[i + 0] = j + 0;
		indices[i + 1] = j + 1;
		indices[i + 2] = j + 2;
		indices[i + 3] = j + 0;
		indices[i + 4] = j + 2;
		indices[i + 5] = j + 3;
	}

	*_indices = indices;
	*_indexCount = indexCount;
	return true;
}
struct Text* createText(
	struct Window* window,
	struct Font* font,
	size_t fontSize,
	struct Pipeline* pipeline,
	const char* _data,
	bool constant)
{
	// TODO:
	// Bind different descriptor sets
	// in Vulkan graphics API

	assert(window != NULL);
	assert(pipeline != NULL);
	assert(font != NULL);
	assert(_data != NULL);
	assert(window == getPipelineWindow(pipeline));

	struct Text* text =
		malloc(sizeof(struct Text));

	if (text == NULL)
		return NULL;

	size_t dataLength =
		strlen(_data);

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

		struct Image* image = createImage(
			window,
			IMAGE_2D_TYPE,
			R8G8B8A8_UNORM_IMAGE_FORMAT,
			fontSize,
			fontSize,
			1,
			NULL,
			false);

		if (image == NULL)
		{
			free(data);
			free(text);
			return NULL;
		}

		struct Buffer* vertexBuffer = createBuffer(
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

		struct Buffer* indexBuffer = createBuffer(
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

		struct Mesh* mesh = createMesh(
			window,
			UINT32_DRAW_INDEX,
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

		struct Glyph* glyphs;
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

		size_t dataSize =
			dataLength + 1;
		char* data = malloc(
			dataSize * sizeof(char));

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

		uint8_t* pixels;
		size_t pixelCount;
		size_t pixelLength;

		result = createTextPixels(
			font->face,
			fontSize,
			glyphs,
			glyphCount,
			0,
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

		struct Image* image = createImage(
			window,
			IMAGE_2D_TYPE,
			R8G8B8A8_UNORM_IMAGE_FORMAT,
			pixelLength,
			pixelLength,
			1,
			pixels,
			false);

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

		result = createTextVertices(
			uniChars,
			uniCharCount,
			glyphs,
			glyphCount,
			&vertices,
			&vertexCount);

		free(glyphs);
		free(uniChars);

		if (result == false)
		{
			destroyImage(image);
			free(data);
			free(text);
			return NULL;
		}

		struct Buffer* vertexBuffer = createBuffer(
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

		struct Buffer* indexBuffer = createBuffer(
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

		struct Mesh* mesh = createMesh(
			window,
			UINT32_DRAW_INDEX,
			indexCount,
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
	}

	text->window = window;
	text->font = font;
	text->fontSize = fontSize;
	text->pipeline = pipeline;
	text->constant = constant;
	return text;
}
void destroyText(
	struct Text* text)
{
	if (text == NULL)
		return;

	struct Buffer* vertexBuffer;
	struct Buffer* indexBuffer;

	getMeshBuffers(
		text->mesh,
		&vertexBuffer,
		&indexBuffer);

	destroyMesh(text->mesh);
	destroyBuffer(indexBuffer);
	destroyBuffer(vertexBuffer);

	destroyImage(text->image);
	free(text->data);

	free(text);
}

struct Window* getTextWindow(
	const struct Text* text)
{
	assert(text != NULL);
	return text->window;
}
bool getTextConstant(
	const struct Text* text)
{
	assert(text != NULL);
	return text->constant;
}

struct Font* getTextFont(
	const struct Text* text)
{
	assert(text != NULL);
	return text->font;
}
void setTextFont(
	struct Text* text,
	struct Font* font)
{
	assert(text != NULL);
	assert(font != NULL);
	assert(text->constant == false);
	text->font = font;
}

size_t getTextFontSize(
	const struct Text* text)
{
	assert(text != NULL);
	return text->fontSize;
}
void setTextFontSize(
	struct Text* text,
	size_t fontSize)
{
	assert(text != NULL);
	assert(text->constant == false);
	text->fontSize = fontSize;
}

struct Pipeline* getTextPipeline(
	const struct Text* text)
{
	assert(text != NULL);
	return text->pipeline;
}
void setTextPipeline(
	struct Text* text,
	struct Pipeline* pipeline)
{
	assert(text != NULL);
	assert(pipeline != NULL);
	assert(text->constant == false);

	assert(text->window ==
		getPipelineWindow(pipeline));

	text->pipeline = pipeline;
}

const char* getTextData(
	const struct Text* text)
{
	assert(text != NULL);
	return text->data;
}
bool setTextData(
	struct Text* text,
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

bool updateText(
	struct Text* text,
	bool reuse)
{
	assert(text != NULL);
	assert(text->constant == false);

	struct Window* window =
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
		}
		else
		{
			uint32_t* uniChars = createTextUniChars(
				_data,
				dataLength,
				uniCharCount);

			if (uniChars == NULL)
				return false;

			struct Glyph* glyphs;
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

			size_t textPixelLength =
				getImageWidth(text->image);

			uint8_t* pixels;
			size_t pixelCount;
			size_t pixelLength;

			result = createTextPixels(
				text->font->face,
				text->fontSize,
				glyphs,
				glyphCount,
				textPixelLength,
				&pixels,
				&pixelCount,
				&pixelLength);

			if (result == false)
			{
				free(glyphs);
				free(uniChars);
				return false;
			}

			struct Image* image = NULL;

			if (pixelLength > textPixelLength)
			{
				image = createImage(
					window,
					IMAGE_2D_TYPE,
					R8G8B8A8_UNORM_IMAGE_FORMAT,
					pixelLength,
					pixelLength,
					1,
					pixels,
					false);

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

			result = createTextVertices(
				uniChars,
				uniCharCount,
				glyphs,
				glyphCount,
				&vertices,
				&vertexCount);

			free(glyphs);
			free(uniChars);

			if (result == false)
			{
				destroyImage(image);
				free(pixels);
				return false;
			}

			struct Buffer* vertexBuffer = NULL;
			struct Buffer* indexBuffer = NULL;

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
					pixelLength,
					pixelLength,
					1,
					0,
					0,
					0,
					0);

				free(pixels);
			}
			else
			{
				destroyImage(text->image);
				text->image = image;
			}

			if (vertexBuffer == NULL)
			{
				struct Buffer* _vertexBuffer =
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
				struct Buffer* _vertexBuffer;
				struct Buffer* _indexBuffer;

				getMeshBuffers(
					text->mesh,
					&_vertexBuffer,
					&_indexBuffer);

				destroyBuffer(_vertexBuffer);
				destroyBuffer(_indexBuffer);

				setMeshBuffers(
					text->mesh,
					UINT32_DRAW_INDEX,
					uniCharCount * 6,
					vertexBuffer,
					indexBuffer);
			}
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

			struct Image* image = createImage(
				window,
				IMAGE_2D_TYPE,
				R8G8B8A8_UNORM_IMAGE_FORMAT,
				text->fontSize,
				text->fontSize,
				1,
				NULL,
				false);

			if (image == NULL)
			{
				free(data);
				return false;
			}

			struct Buffer* vertexBuffer = createBuffer(
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

			struct Buffer* indexBuffer = createBuffer(
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

			struct Mesh* mesh = createMesh(
				window,
				UINT32_DRAW_INDEX,
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

			getMeshBuffers(
				text->mesh,
				&vertexBuffer,
				&indexBuffer);

			destroyMesh(text->mesh);
			destroyBuffer(vertexBuffer);
			destroyBuffer(indexBuffer);

			destroyImage(text->image);
			free(text->data);

			text->data = data;
			text->dataSize = dataSize;
			text->image = image;
			text->mesh = mesh;
		}
		else
		{
			uint32_t* uniChars = createTextUniChars(
				_data,
				dataLength,
				uniCharCount);

			if (uniChars == NULL)
				return false;

			struct Glyph* glyphs;
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

			uint8_t* pixels;
			size_t pixelCount;
			size_t pixelLength;

			result = createTextPixels(
				text->font->face,
				text->fontSize,
				glyphs,
				glyphCount,
				0,
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

			struct Image* image = createImage(
				window,
				IMAGE_2D_TYPE,
				R8G8B8A8_UNORM_IMAGE_FORMAT,
				pixelLength,
				pixelLength,
				1,
				pixels,
				false);

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

			result = createTextVertices(
				uniChars,
				uniCharCount,
				glyphs,
				glyphCount,
				&vertices,
				&vertexCount);

			free(glyphs);
			free(uniChars);

			if (result == false)
			{
				destroyImage(image);
				free(data);
				return false;
			}

			struct Buffer* vertexBuffer = createBuffer(
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

			struct Buffer* indexBuffer = createBuffer(
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

			struct Mesh* mesh = createMesh(
				window,
				UINT32_DRAW_INDEX,
				indexCount,
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

			getMeshBuffers(
				text->mesh,
				&vertexBuffer,
				&indexBuffer);

			destroyMesh(text->mesh);
			destroyBuffer(vertexBuffer);
			destroyBuffer(indexBuffer);

			destroyImage(text->image);
			free(text->data);

			text->data = data;
			text->dataSize = dataSize;
			text->image = image;
			text->mesh = mesh;
		}
	}

	return true;
}
void drawTextCommand(
	struct Text* text)
{
	assert(text != NULL);

	struct Pipeline* pipeline =
		text->pipeline;
	struct TextPipeline* textPipeline =
		(struct TextPipeline*)pipeline->handle;

	textPipeline->image =
		text->image;

	drawMeshCommand(
		text->mesh,
		pipeline);
}

inline static struct GlTextPipeline* createGlTextPipeline(
	struct Window* window,
	const void* vertexShader,
	const void* fragmentShader,
	bool gles)
{
	struct GlTextPipeline* pipeline = malloc(
		sizeof(struct GlTextPipeline));

	if (pipeline == NULL)
		return NULL;

	GLenum stages[2] = {
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER,
	};
	const char* shaders[2] = {
		(const char*)vertexShader,
		(const char*)fragmentShader,
	};

	makeWindowContextCurrent(window);

	GLuint handle = createGlPipeline(
		stages,
		shaders,
		2,
		gles);

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

	pipeline->handle = handle;
	pipeline->mvpLocation = mvpLocation;
	pipeline->colorLocation = colorLocation;
	pipeline->imageLocation = imageLocation;
	return pipeline;
}
void destroyGlTextPipeline(
	struct Pipeline* pipeline)
{
	struct TextPipeline* textPipeline =
		(struct TextPipeline*)pipeline->handle;
	struct GlTextPipeline* glTextPipeline =
		(struct GlTextPipeline*)textPipeline->handle;

	makeWindowContextCurrent(
		pipeline->window);

	glDeleteProgram(
		glTextPipeline->handle);

	assertOpenGL();

	free(glTextPipeline);
	free(textPipeline);
}
void bindGlTextPipeline(
	struct Pipeline* pipeline)
{
	struct TextPipeline* textPipeline =
		(struct TextPipeline*)pipeline->handle;
	struct GlTextPipeline* glTextPipeline =
		(struct GlTextPipeline*)textPipeline->handle;

	glUseProgram(glTextPipeline->handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);

	glBlendFunc(
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA);

	// TODO: change blending to descending ordering

	assertOpenGL();
}
void setGlTextPipelineUniforms(
	struct Pipeline* pipeline)
{
	struct TextPipeline* textPipeline =
		(struct TextPipeline*)pipeline->handle;
	struct GlTextPipeline* glTextPipeline =
		(struct GlTextPipeline*)textPipeline->handle;

	struct Image* image =
		textPipeline->image;
	GLuint glImage = *(const GLuint*)
		getImageHandle(image);

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
		glTextPipeline->imageLocation,
		0);

	// TODO: TMP
	struct Matrix4F mvp = translateMatrix4F(
		scaleMatrix4F(
			textPipeline->mvp,
			createValueVector3F(0.1f)),
		createVector3F(-2, 0, 0));

	glUniformMatrix4fv(
		glTextPipeline->mvpLocation,
		1,
		GL_FALSE,
		(const float*)&mvp);
	glUniform4fv(
		glTextPipeline->colorLocation,
		1,
		(const float*)&textPipeline->color);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vector2F) * 2,
		0);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vector2F) * 2,
		(const void*)sizeof(struct Vector2F));

	assertOpenGL();
}
struct Pipeline* createTextPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	enum CullFace cullFace,
	enum FrontFace frontFace,
	const void* vertexShader,
	size_t vertexShaderSize,
	const void* fragmentShader,
	size_t fragmentShaderSize)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);

	struct TextPipeline* textPipeline =
		malloc(sizeof(struct TextPipeline));

	if (textPipeline == NULL)
		return NULL;

	enum GraphicsAPI api =
		getWindowGraphicsAPI(window);;

	void* handle;

	DestroyPipeline destroyFunction;
	BindPipelineCommand bindFunction;
	SetUniformsCommand setUniformsFunction;

	if (api == OPENGL_GRAPHICS_API)
	{
		handle = createGlTextPipeline(
			window,
			vertexShader,
			fragmentShader,
			false);

		destroyFunction = destroyGlTextPipeline;
		bindFunction = bindGlTextPipeline;
		setUniformsFunction = setGlTextPipelineUniforms;
	}
	else if (api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlTextPipeline(
			window,
			vertexShader,
			fragmentShader,
			true);

		destroyFunction = destroyGlTextPipeline;
		bindFunction = bindGlTextPipeline;
		setUniformsFunction = setGlTextPipelineUniforms;
	}
	else
	{
		free(textPipeline);
		return NULL;
	}

	if (handle == NULL)
	{
		free(textPipeline);
		return NULL;
	}

	textPipeline->mvp = createIdentityMatrix4F();
	textPipeline->color = createValueVector4F(1.0f);
	textPipeline->image = NULL;
	textPipeline->handle = handle;

	struct Pipeline* pipeline = createPipeline(
		window,
		drawMode,
		cullFace,
		frontFace,
		destroyFunction,
		bindFunction,
		setUniformsFunction,
		textPipeline);

	if (pipeline == NULL)
	{
		destroyGlTextPipeline(handle);
		free(textPipeline);
		return NULL;
	}

	return pipeline;
}
