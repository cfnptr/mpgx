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

	// TODO:
	// create wide charr array and char array
	// create already known vertex array.

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

	size_t charIndex = 0;

	float posX = 0.0f;
	float posY = 0.0f;

	while (true)
	{
		char charValue = text[charIndex];

		if (charValue == '\0')
			break;

		uint32_t newChar = 0b00000000;
		char* newCharPtr = (char*)&newChar;

		newCharPtr[0] = charValue;

		if ((charValue >> 7) == 0b00000000)
		{
			charIndex += 1;
		}
		else if ((charValue >> 5) == 0b00000110 &&
			(text[charIndex + 1] >> 6) == 0b00000010)
		{
			newCharPtr[1] = text[charIndex + 1];
			charIndex += 2;
		}
		else if ((charValue >> 4) == 0b00001110 &&
			(text[charIndex + 1] >> 6) == 0b00000010 &&
			(text[charIndex + 2] >> 6) == 0b00000010)
		{
			newCharPtr[1] = text[charIndex + 1];
			newCharPtr[2] = text[charIndex + 2];
			charIndex += 3;
		}
		else if ((charValue >> 3) == 0b00011110 &&
			(text[charIndex + 1] >> 6) == 0b00000010 &&
			(text[charIndex + 2] >> 6) == 0b00000010 &&
			(text[charIndex + 3] >> 6) == 0b00000010)
		{
			newCharPtr[1] = text[charIndex + 1];
			newCharPtr[2] = text[charIndex + 2];
			newCharPtr[3] = text[charIndex + 3];
			charIndex += 4;
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

		vertices[vertexCount + 0] = charPosX;
		vertices[vertexCount + 1] = charPosY;
		vertices[vertexCount + 2] = 0.0f; // TODO: calc UV
		vertices[vertexCount + 3] = 0.0f;
		vertices[vertexCount + 4] = charPosX;
		vertices[vertexCount + 5] = charPosY + charHeight;
		vertices[vertexCount + 6] = 0.0f;
		vertices[vertexCount + 7] = 1.0f;
		vertices[vertexCount + 8] = charPosX + charWidth;
		vertices[vertexCount + 9] = charPosY + charHeight;
		vertices[vertexCount + 10] = 1.0f;
		vertices[vertexCount + 11] = 1.0f;
		vertices[vertexCount + 12] = charPosX + charWidth;
		vertices[vertexCount + 13] = charPosY;
		vertices[vertexCount + 14] = 1.0f;
		vertices[vertexCount + 15] = 0.0f;

		vertexCount += 16;
		posX += glyph->advance.x / 64.0f;
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

	size_t charLength =
		((charCount / 2) + 1);
	size_t imageLength =
		charLength * fontSize;
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

		size_t pixelPosY = (i / charLength);
		size_t pixelPosX = (i - pixelPosY * charLength);

		pixelPosX *= fontSize;
		pixelPosY *= fontSize;

		for (size_t y = 0; y < glyphHeight; y++)
		{
			for (size_t x = 0; x < glyphWidth; x++)
			{
				size_t pixelPos = 
					(y + pixelPosY) * 4 * imageLength + 
					(x + pixelPosX) * 4;
				uint8_t value = bitmap[y * glyphWidth + x];

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

	// TODO pass values in contructor
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glUniform1i(
		glTextPipeline->imageLocation,
		0);

	// TODO: set image filter
	// Get them from gl pipeline instance

	// TODO: TMP
	struct Matrix4F mvp = translateMatrix4F(
		scaleMatrix4F(
			textPipeline->mvp,
			createValueVector3F(0.01)),
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
