#include "mpgx/text.h"
#include "mpgx/opengl.h"

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
	struct Pipeline* pipeline;
	// TODO: \/
	/*size_t textCapacity;
	size_t imageCapacity;*/
	char* data;
	struct Image* image;
	struct Mesh* mesh;
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

inline static bool createTextData(
	struct Window* window,
	FT_Face face,
	size_t fontSize,
	const char* text,
	bool constant,
	bool mipmap,
	char** _data,
	struct Mesh** _mesh,
	struct Image** _image)
{
	size_t textLength =
		strlen(text);
	char* data = malloc(
		textLength * sizeof(char));

	if (data == NULL)
		return NULL;

	if (textLength == 0)
	{
		// TODO: handle zero size string
		/*struct Buffer* vertexBuffer = createBuffer(
			window,
			VERTEX_BUFFER_TYPE,
			NULL,
			charQuadSize * sizeof(float),
			constant);
		struct Buffer* indexBuffer
		struct Mesh* mesh = createMesh()*/
	}

	size_t charCount = 0;
	size_t charCapacity = 1;

	uint32_t* chars = malloc(
		charCapacity * sizeof(uint32_t));

	if (chars == NULL)
	{
		free(data);
		return false;
	}

	size_t vertexCount = 0;
	size_t vertexCapacity = textLength * 16;

	float* vertices = malloc(
		vertexCapacity * sizeof(float));

	if (vertices == NULL)
	{
		free(chars);
		free(data);
		return false;
	}

	size_t index = 0;

	float posX = 0.0f;
	float posY = 0.0f;

	while (true)
	{
		char charValue = text[index];

		if (charValue == '\0')
			break;

		uint32_t newChar = 0b00000000;
		char* newCharPtr = (char*)&newChar;

		newCharPtr[0] = charValue;

		if ((charValue >> 7) == 0b00000000)
		{
			index += 1;
		}
		else if ((charValue >> 5) == 0b00000110 &&
			(text[index + 1] >> 6) == 0b00000010)
		{
			newCharPtr[1] = text[index + 1];
			index += 2;
		}
		else if ((charValue >> 4) == 0b00001110 &&
			(text[index + 1] >> 6) == 0b00000010 &&
			(text[index + 2] >> 6) == 0b00000010)
		{
			newCharPtr[1] = text[index + 1];
			newCharPtr[2] = text[index + 2];
			index += 3;
		}
		else if ((charValue >> 3) == 0b00011110 &&
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
			free(vertices);
			free(chars);
			free(data);
			return false;
		}

		FT_UInt charIndex = FT_Get_Char_Index(
			face,
			newChar);

		FT_Error result = FT_Load_Glyph(
			face,
			charIndex,
			FT_LOAD_BITMAP_METRICS_ONLY);

		if (result != 0)
		{
			free(vertices);
			free(chars);
			free(data);
			return false;
		}

		bool charExists = false;

		for (size_t i = 0; i < charCount; ++i)
		{
			if (newChar == chars[i])
			{
				charExists = true;
				break;
			}
		}

		if (charExists == false)
		{
			if (charCount == charCapacity)
			{
				charCapacity = charCapacity * 2;

				uint32_t* newChars = realloc(
					chars,
					charCapacity * sizeof(uint32_t));

				if (newChars == NULL)
				{
					free(vertices);
					free(chars);
					free(data);
					return false;
				}

				chars = newChars;
			}

			chars[charCount] = newChar;
			charCount++;
		}

		if (vertexCount == vertexCapacity)
		{
			vertexCapacity = vertexCapacity * 2;

			float* newVertices = realloc(
				vertices,
				vertexCapacity * sizeof(float));

			if (newVertices == NULL)
			{
				free(vertices);
				free(chars);
				free(data);
				return false;
			}

			vertices = newVertices;
		}

		FT_GlyphSlot glyph = face->glyph;

		float charPosX = posX + glyph->bitmap_left;
		float charPosY = posY - (float)(glyph->bitmap.rows - glyph->bitmap_top);
		float charWidth = (float)glyph->bitmap.width;
		float charHeight = (float)glyph->bitmap.rows;

		vertices[0] = charPosX;
		vertices[1] = charPosY;
		vertices[2] = 0.0f; // TODO: calc UV
		vertices[3] = 0.0f;
		vertices[4] = charPosX;
		vertices[5] = charPosY + charHeight;
		vertices[6] = 0.0f;
		vertices[7] = 1.0f;
		vertices[8] = charPosX + charWidth;
		vertices[9] = charPosY + charHeight;
		vertices[10] = 1.0f;
		vertices[11] = 1.0f;
		vertices[12] = charPosX + charWidth;
		vertices[13] = charPosY;
		vertices[14] = 1.0f;
		vertices[15] = 0.0f;
		vertexCount += 16;
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
		free(chars);
		free(data);
		return false;
	}

	size_t indexCount =
		(vertexCount / 16) * 6;
	uint32_t* indices = malloc(
		indexCount * sizeof(uint32_t));

	if (indices == NULL)
	{
		destroyBuffer(vertexBuffer);
		free(chars);
		free(data);
		return false;
	}

	for (size_t i = 0, j = 0; i < indexCount; i += 6, j += 4)
	{
		indices[0] = j + 0;
		indices[1] = j + 1;
		indices[2] = j + 2;
		indices[3] = j + 0;
		indices[4] = j + 2;
		indices[5] = j + 3;
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
		free(chars);
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
		free(chars);
		free(data);
		return false;
	}

	size_t rowLength =
		((charCount / 2) + 1);
	size_t imageLength =
		rowLength * fontSize;
	size_t imageSize =
		imageLength * imageLength;
	uint8_t* pixels = malloc(
		imageSize * sizeof(uint8_t) * 4);

	if (pixels == NULL)
	{
		destroyMesh(mesh);
		destroyBuffer(indexBuffer);
		destroyBuffer(vertexBuffer);
		free(chars);
		free(data);
		return false;
	}

	for (size_t i = 0; i < charCount; i++)
	{
		FT_UInt charIndex = FT_Get_Char_Index(
			face,
			chars[i]);

		FT_Error result = FT_Load_Glyph(
			face,
			charIndex,
			FT_LOAD_RENDER);

		if (result != 0)
		{
			free(pixels);
			destroyMesh(mesh);
			destroyBuffer(indexBuffer);
			destroyBuffer(vertexBuffer);
			free(chars);
			free(data);
			return false;
		}

		FT_GlyphSlot glyph = face->glyph;
		size_t glyphWidth = glyph->bitmap.width;
		size_t glyphHeight = glyph->bitmap.rows;
		uint8_t* bitmap = glyph->bitmap.buffer;

		size_t pixelPosY = (charCount / rowLength) * fontSize * 4;
		size_t pixelPosX = (charCount - pixelPosY * rowLength) * fontSize * 4;

		for (size_t y = 0; y < glyphHeight; y++)
		{
			for (size_t x = 0; x < glyphWidth; x++)
			{
				size_t pixelPos = (y + pixelPosY) * imageLength + (x + pixelPosX);
				uint8_t value = bitmap[y * glyphHeight + x];

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
		mipmap);

	free(pixels);
	free(chars);

	if (image == NULL)
	{
		destroyMesh(mesh);
		destroyBuffer(indexBuffer);
		destroyBuffer(vertexBuffer);
		free(data);
		return false;
	}

	*_data = data;
	*_mesh = mesh;
	*_image = image;
	return true;
}
inline static void destroyTextData(
	struct Image* image,
	struct Mesh* mesh,
	char* data)
{
	destroyImage(image);

	struct Buffer* vertexBuffer;
	struct Buffer* indexBuffer;

	getMeshBuffers(
		mesh,
		&vertexBuffer,
		&indexBuffer);

	destroyMesh(mesh);
	destroyBuffer(indexBuffer);
	destroyBuffer(vertexBuffer);

	free(data);
}
struct Text* createText(
	struct Window* window,
	struct Font* font,
	size_t fontSize,
	struct Pipeline* pipeline,
	const char* _text,
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

	char* data;
	struct Mesh* mesh;
	struct Image* image;

	bool dataResult = createTextData(
		window,
		face,
		fontSize,
		_text,
		constant,
		mipmap,
		&data,
		&mesh,
		&image);

	if (dataResult == false)
	{
		free(text);
		return NULL;
	}

	text->font = font;
	text->fontSize = fontSize;
	text->pipeline = pipeline;
	text->data = data;
	text->image = image;
	text->mesh = mesh;
	return text;
}
void destroyText(
	struct Text* text)
{
	if (text == NULL)
		return;

	destroyTextData(
		text->image,
		text->mesh,
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

	// TODO: set image filter
	// Get them from gl pipeline instance

	glUniformMatrix4fv(
		glTextPipeline->mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&textPipeline->mvp);
	glUniform4fv(
		glTextPipeline->colorLocation,
		1,
		(const GLfloat*)&textPipeline->color);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vector3F),
		0);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vector2F),
		(const void*)sizeof(struct Vector3F));

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
