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
	struct Font* font;
	size_t fontSize;
	struct Pipeline* pipeline;
	// TODO: \/
	/*size_t textCapacity;
	size_t imageCapacity;*/
	wchar_t* data;
	struct Image* image;
	struct Mesh* mesh;
};
struct Glyph
{
	wchar_t wideChar;
	uint32_t sizeX;
	uint32_t sizeY;
	int32_t bearingX;
	int32_t bearingY;
	float advance;
	float texCoordU;
	float texCoordV;
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

	FT_Face face;

	FT_Error result = FT_New_Face(
		getFtLibrary(),
		filePath,
		0,
		&face);

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

wchar_t* createTextString(
	const char* text)
{
	assert(text);

	size_t length = mbstowcs(
		NULL,
		text,
		0);

	if (length == (size_t)-1)
		return NULL;

	wchar_t* string = malloc(
		(length + 1) * sizeof(wchar_t));

	if (string == NULL)
		return NULL;

	size_t count = mbstowcs(
		string,
		text,
		length + 1);

	if (count == (size_t)-1)
	{
		free(string);
		return NULL;
	}

	return string;
}
void destroyTextString(
	wchar_t* text)
{
	free(text);
}

int compareGlyph(
	const void* a,
	const void* b)
{
	if (((struct Glyph*)a)->wideChar <
		((struct Glyph*)b)->wideChar)
	{
		return -1;
	}
	if (((struct Glyph*)a)->wideChar ==
		((struct Glyph*)b)->wideChar)
	{
		return 0;
	}
	if (((struct Glyph*)a)->wideChar >
		((struct Glyph*)b)->wideChar)
	{
		return 1;
	}

	abort();
}
inline static bool createTextData(
	struct Window* window,
	FT_Face face,
	size_t fontSize,
	const wchar_t* text,
	bool constant,
	bool mipmap,
	wchar_t** _data,
	struct Image** _image,
	struct Mesh** _mesh)
{
	size_t textLength =
		wcslen(text);

	// TODO: handle zero size string
	if (textLength == 0)
		return NULL;

	wchar_t* data = malloc(
		textLength * sizeof(wchar_t));

	if (data == NULL)
		return NULL;

	size_t glyphCount = 0;

	struct Glyph* glyphs = malloc(
		textLength * sizeof(struct Glyph));

	if (glyphs == NULL)
	{
		free(data);
		return false;
	}

	for (size_t i = 0; i < textLength; i++)
	{
		wchar_t wideChar = text[i];
		bool glyphExists = false;

		// TODO: optimize with binary search
		for (size_t j = 0; j < glyphCount; j++)
		{
			if (wideChar == glyphs[j].wideChar)
			{
				glyphExists = true;
				break;
			}
		}

		if (glyphExists == false)
		{
			glyphs[glyphCount].wideChar = wideChar;
			glyphCount++;
		}
	}

	qsort(
		glyphs,
		glyphCount,
		sizeof(struct Glyph),
		compareGlyph);

	size_t glyphLength =
		((glyphCount / 2) + 1);
	size_t pixelLength =
		glyphLength * fontSize;
	size_t pixelCount =
		pixelLength * pixelLength;
	uint8_t* pixels = malloc(
		pixelCount * sizeof(uint8_t) * 4);

	if (pixels == NULL)
	{
		free(glyphs);
		free(data);
		return false;
	}

	for (size_t i = 0; i < glyphCount; i++)
	{
		struct Glyph glyph;
		glyph.wideChar = glyphs[i].wideChar;

		FT_UInt charIndex = FT_Get_Char_Index(
			face,
			glyph.wideChar);
		FT_Error result = FT_Load_Glyph(
			face,
			charIndex,
			FT_LOAD_RENDER);

		if (result != 0)
		{
			free(pixels);
			free(glyphs);
			free(data);
			return false;
		}

		FT_GlyphSlot glyphSlot = face->glyph;
		uint8_t* bitmap = glyphSlot->bitmap.buffer;

		size_t pixelPosY = (i / glyphLength);
		size_t pixelPosX = (i - pixelPosY * glyphLength);

		pixelPosX *= fontSize;
		pixelPosY *= fontSize;

		glyph.sizeX = glyphSlot->bitmap.width;
		glyph.sizeY = glyphSlot->bitmap.rows;
		glyph.bearingX = glyphSlot->bitmap_left;
		glyph.bearingY = glyphSlot->bitmap_top;
		glyph.advance = glyphSlot->advance.x / 64.0f;
		glyph.texCoordU = (float)pixelPosX / pixelLength;
		glyph.texCoordV = (float)pixelPosY / pixelLength;
		glyphs[i] = glyph;

		for (size_t y = 0; y < glyph.sizeY; y++)
		{
			for (size_t x = 0; x < glyph.sizeX; x++)
			{
				size_t pixelPos =
					(y + pixelPosY) * 4 * pixelLength +
					(x + pixelPosX) * 4;
				uint8_t value = bitmap[y * glyph.sizeX + x];

				pixels[pixelPos + 0] = 255;
				pixels[pixelPos + 1] = 0;
				pixels[pixelPos + 2] = 255;
				pixels[pixelPos + 3] = value;
			}
		}
	}

	struct Image* image = createImage2D(
		window,
		R8G8B8A8_UNORM_IMAGE_FORMAT,
		pixelLength,
		pixelLength,
		pixels,
		mipmap);

	free(pixels);

	if (image == NULL)
	{
		free(glyphs);
		free(data);
		return false;
	}

	size_t vertexCount =
		textLength * 16;
	float* vertices = malloc(
		vertexCount * sizeof(float));

	if (vertices == NULL)
	{
		free(glyphs);
		free(data);
		return false;
	}

	size_t vertexIndex = 0;
	float vertexGlyphPosX = 0.0f;
	float vertexGlyphPosY = 0.0f;

	for (size_t i = 0; i < textLength; i++)
	{
		struct Glyph searchGlyph;
		searchGlyph.wideChar = text[i];

		struct Glyph* glyph = bsearch(
			&searchGlyph,
			glyphs,
			glyphCount,
			sizeof(struct Glyph),
			compareGlyph);

		if (glyph == NULL)
			abort();

		float glyphPosX = vertexGlyphPosX + glyph->bearingX;
		float glyphPosY = vertexGlyphPosY - (float)(glyph->sizeY - glyph->bearingY);
		float glyphWidth = (float)glyph->sizeX;
		float glyphHeight = (float)glyph->sizeY;
		float texCoordU = glyph->texCoordU;
		float texCoordV = glyph->texCoordV;

		vertices[vertexIndex + 0] = glyphPosX;
		vertices[vertexIndex + 1] = glyphPosY;
		vertices[vertexIndex + 2] = texCoordU;
		vertices[vertexIndex + 3] = texCoordV + (float)glyphHeight / pixelLength;
		vertices[vertexIndex + 4] = glyphPosX;
		vertices[vertexIndex + 5] = glyphPosY + glyphHeight;
		vertices[vertexIndex + 6] = texCoordU;
		vertices[vertexIndex + 7] = texCoordV;
		vertices[vertexIndex + 8] = glyphPosX + glyphWidth;
		vertices[vertexIndex + 9] = glyphPosY + glyphHeight;
		vertices[vertexIndex + 10] = texCoordU + (float)glyphWidth / pixelLength;
		vertices[vertexIndex + 11] = texCoordV;
		vertices[vertexIndex + 12] = glyphPosX + glyphWidth;
		vertices[vertexIndex + 13] = glyphPosY;
		vertices[vertexIndex + 14] = texCoordU + (float)glyphWidth / pixelLength;
		vertices[vertexIndex + 15] = texCoordV + (float)glyphHeight / pixelLength;

		vertexIndex += 16;
		vertexGlyphPosX += glyph->advance;
	}

	struct Buffer* vertexBuffer = createBuffer(
		window,
		VERTEX_BUFFER_TYPE,
		vertices,
		vertexCount * sizeof(float),
		constant);

	free(vertices);
	free(glyphs);

	if (vertexBuffer == NULL)
	{
		free(data);
		return false;
	}

	size_t indexCount =
		textLength * 6;
	uint32_t* indices = malloc(
		indexCount * sizeof(uint32_t));

	if (indices == NULL)
	{
		free(data);
		return false;
	}

	for (size_t i = 0, j = 0; i < indexCount; i += 6, j += 4)
	{
		indices[i + 0] = j + 0;
		indices[i + 1] = j + 1;
		indices[i + 2] = j + 2;
		indices[i + 3] = j + 0;
		indices[i + 4] = j + 2;
		indices[i + 5] = j + 3;
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
		free(data);
		return false;
	}

	*_data = data;
	*_image = image;
	*_mesh = mesh;
	return true;
}
inline static void destroyTextData(
	struct Mesh* mesh,
	struct Image* image,
	wchar_t* data)
{
	struct Buffer* vertexBuffer;
	struct Buffer* indexBuffer;

	getMeshBuffers(
		mesh,
		&vertexBuffer,
		&indexBuffer);

	destroyMesh(mesh);
	destroyBuffer(indexBuffer);
	destroyBuffer(vertexBuffer);

	destroyImage(image);
	free(data);
}
struct Text* createText(
	struct Window* window,
	struct Font* font,
	size_t fontSize,
	struct Pipeline* pipeline,
	const wchar_t* _text,
	bool mipmap,
	bool constant)
{
	// TODO:
	// Bind different descriptor sets
	// in Vulkan graphics API

	assert(window != NULL);
	assert(pipeline != NULL);
	assert(font != NULL);
	assert(_text != NULL);
	assert(window == getPipelineWindow(pipeline));

	struct Text* text =
		malloc(sizeof(struct Text));

	if (text == NULL)
		return NULL;

	FT_Face face = font->face;

	FT_Error result = FT_Set_Pixel_Sizes(
		face,
		0,
		fontSize);

	if (result != 0)
	{
		free(text);
		return NULL;
	}

	bool dataResult = createTextData(
		window,
		face,
		fontSize,
		_text,
		constant,
		mipmap,
		&text->data,
		&text->image,
		&text->mesh);

	if (dataResult == false)
	{
		free(text);
		return NULL;
	}

	text->font = font;
	text->fontSize = fontSize;
	text->pipeline = pipeline;
	return text;
}
void destroyText(
	struct Text* text)
{
	if (text == NULL)
		return;

	destroyTextData(
		text->mesh,
		text->image,
		text->data);

	free(text);
}

size_t getTextFontSize(
	const struct Text* text)
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

	glActiveTexture(GL_TEXTURE0);

	GLuint glImage = *(const GLuint*)
		getImageHandle(textPipeline->image);

	glBindTexture(
		GL_TEXTURE_2D,
		glImage);

	// TODO pass values in contructor
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glUniform1i(
		glTextPipeline->imageLocation,
		0);

	// TODO: set image filter
	// Get them from gl pipeline instance

	// TODO: TMP
	struct Matrix4F mvp = translateMatrix4F(
		scaleMatrix4F(
			textPipeline->mvp,
			createValueVector3F(0.01f)),
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
	/*textPipeline->minFilter = minFilter;
	textPipeline->magFilter = magFilter;
	textPipeline->mipmapFilter = mipmapFilter;*/
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
