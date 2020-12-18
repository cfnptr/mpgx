#include "mpgx/text.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include <assert.h>

struct Font
{
	FT_Face face;
};

struct Text
{
	struct Font* font;
	size_t fontSize;
	size_t textCapacity;
	size_t imageCapacity;
	struct Mesh* mesh;
	struct Image* image;
};

struct Font* createFont(
	const void* fileData,
	size_t fileSize)
{
	assert(fileData != NULL);
	assert(fileSize != 0);

	struct Font* font =
		malloc(sizeof(struct Font));

	if (font == NULL)
		return NULL;

	FT_Face face;

	FT_Error result = FT_New_Memory_Face(
		getFtLibrary(),
		fileData,
		fileSize,
		0,
		&face);

	if (result != 0)
	{
		free(face);
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

struct Glyph
{
	uint32_t charValue;
	size_t sizeX;
	size_t sizeY;
	size_t bearingX;
	size_t bearingY;
	size_t advance;
};

int compareGlyphs(
	const void * a,
	const void * b)
{
	if (((struct Glyph*)a)->charValue <
		((struct Glyph*)b)->charValue)
	{
		return -1;
	}
	if (((struct Glyph*)a)->charValue ==
		((struct Glyph*)b)->charValue)
	{
		return 0;
	}
	if (((struct Glyph*)a)->charValue >
		((struct Glyph*)b)->charValue)
	{
		return 1;
	}

	abort();
}
inline static struct Glyph* createTextGlyphs(
	const char* text,
	size_t* _count)
{
	size_t count = 0;
	size_t capacity = 1;

	struct Glyph* glyphs = malloc(
		capacity * sizeof(struct Glyph));

	if (glyphs == NULL)
		return NULL;

	size_t index = 0;

	while (true)
	{
		char value = text[index];

		if (value == '\0')
			break;

		uint32_t newChar = 0;
		char* newCharPtr = (char*)&newChar;

		newCharPtr[0] = value;

		if ((value >> 7) == 0b00000000)
		{
			index += 1;
		}
		else if ((value >> 5) == 0b00000110 &&
			(text[index + 1] >> 6) == 0b00000010)
		{
			newCharPtr[1] = text[index + 1];
			index += 2;
		}
		else if ((value >> 4) == 0b00001110 &&
			(text[index + 1] >> 6) == 0b00000010 &&
			(text[index + 2] >> 6) == 0b00000010)
		{
			newCharPtr[1] = text[index + 1];
			newCharPtr[2] = text[index + 2];
			index += 3;
		}
		else if ((value >> 3) == 0b00011110 &&
			(text[index + 1] >> 6) == 0b00000010 &&
			(text[index + 2] >> 6) == 0b00000010 &&
			(text[index + 3] >> 6) == 0b00000010)
		{
			newCharPtr[1] = text[index + 1];
			newCharPtr[2] = text[index + 2];
			newCharPtr[3] = text[index + 3];
			index += 4;
		}
		else
		{
			free(glyphs);
			return NULL;
		}

		// TODO: Possibly could be optimized
		//  with the binary search algorithm

		bool charExists = false;

		for (size_t i = 0; i < count; ++i)
		{
			if (newChar == glyphs[i].charValue)
			{
				charExists = true;
				break;
			}
		}

		if (charExists == false)
		{
			if (count == capacity)
			{
				capacity = capacity * 2;

				struct Glyph* newGlyphs = realloc(
					glyphs,
					capacity * sizeof(struct Glyph));

				if (newGlyphs == NULL)
				{
					free(glyphs);
					return NULL;
				}
			}

			glyphs[count++].charValue = newChar;
		}
	}

	qsort(
		glyphs,
		count,
		sizeof(struct Glyph),
		compareGlyphs);

	*_count = count;
	return glyphs;
}
inline static struct Image* createTextImage(
	struct Window* window,
	struct Glyph* glyphs,
	size_t glyphCount,
	FT_Face face,
	size_t fontSize)
{
	size_t glyphLength =
		((glyphCount / 2) + 1);
	size_t imageLength =
		glyphLength * fontSize;
	size_t imageSize =
		imageLength * imageLength;
	uint8_t* pixels = malloc(
		imageSize * sizeof(uint8_t) * 4);

	if (pixels == NULL)
		return NULL;

	for (size_t i = 0; i < glyphCount; i++)
	{
		FT_UInt charIndex = FT_Get_Char_Index(
			face,
			glyphs[i].charValue);

		FT_Error result = FT_Load_Glyph(
			face,
			charIndex,
			FT_LOAD_RENDER);

		if (result != 0)
		{
			free(pixels);
			return NULL;
		}

		FT_GlyphSlot faceGlyph = face->glyph;

		struct Glyph glyph = glyphs[i];
		glyph.sizeX = faceGlyph->bitmap.width;
		glyph.sizeY = faceGlyph->bitmap.rows;
		glyph.bearingX = faceGlyph->bitmap_left;
		glyph.bearingY = faceGlyph->bitmap_top;
		glyph.advance = faceGlyph->advance.x;
		glyphs[i] = glyph;

		size_t pixelPosY =
			(glyphCount / glyphLength) * fontSize * 4;
		size_t pixelPosX =
			(glyphCount - pixelPosY * glyphLength) * fontSize * 4;

		uint8_t* bitmap = faceGlyph->bitmap.buffer;

		for (size_t y = 0; y < glyph.sizeY; y++)
		{
			for (size_t x = 0; x < glyph.sizeX; x++)
			{
				size_t pixelPos =
					(y + pixelPosY) * imageLength + (x + pixelPosX);

				uint8_t value = bitmap[y * glyph.sizeY + x];

				pixels[pixelPos] = value;
				pixels[pixelPos + 1] = value;
				pixels[pixelPos + 2] = value;
				pixels[pixelPos + 3] = value;
			}
		}
	}

	struct Image* image = createImage2D(
		window,
		R8G8B8A8_UNORM_IMAGE_FORMAT,
		imageLength,
		imageLength,
		pixels,
		false);

	if (image == NULL)
	{
		free(pixels);
		return NULL;
	}

	return image;
}
struct Text* createText(
	struct Window* window,
	struct Font* font,
	size_t fontSize,
	const char* _text)
{
	assert(window != NULL);
	assert(font != NULL);

	struct Text* text =
		malloc(sizeof(struct Text));

	if (text == NULL)
		return NULL;

	FT_Error result = FT_Set_Pixel_Sizes(
		font->face,
		0,
		fontSize);

	if (result != 0)
	{
		free(text);
		return NULL;
	}

	if (_text == NULL ||
		_text[0] == '\0')
	{
		text->font = font;
		text->fontSize = fontSize;
		text->imageCapacity = 0;
		text->textCapacity = 0;
		text->mesh = NULL;
		text->image = NULL;
		return text;
	}

	size_t glyphCount;

	struct Glyph* glyphs = createTextGlyphs(
		_text,
		&glyphCount);

	if (glyphs == NULL)
	{
		free(text);
		return NULL;
	}

	struct Image* image = createTextImage(
		window,
		glyphs,
		glyphCount,
		font->face,
		fontSize);

	if (image == NULL)
	{
		free(glyphs);
		free(text);
		return NULL;
	}

	// TODO:

	/*struct Buffer* vertexBuffer = createBuffer(
		window,
		VERTEX_BUFFER_TYPE,
		NULL,
		)*/

	text->font = font;
	text->fontSize = fontSize;
	text->image = image;
	// TODO: mesh
	return text;
}
void destroyText(
	struct Text* text)
{
	if (text == NULL)
		return;

	free(text);
}

size_t getTextFontSize(
	struct Text* text)
{
	assert(text != NULL);
	return text->fontSize;
}
bool setTextFontSize(
	struct Text* text,
	size_t size)
{
	assert(text != NULL);
	assert(size != 0);

	FT_Error result = FT_Set_Pixel_Sizes(
		text->font->face,
		0,
		size);

	if (result != 0)
		return false;



	// TODO: recreate mesh

	text->fontSize = size;
	return true;
}
