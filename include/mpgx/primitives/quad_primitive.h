#pragma once
#include <stdint.h>

static const float quadVert2D[] = {
	-1.0f, -1.0f,
	-1.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, -1.0f,
};
static const float quadVertTex2D[] = {
	-1.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 0.0f,
};
static const float quadVert3D[] = {
	-1.0f, -1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
};
static const float quadVertNorm3D[] = {
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
	-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
	1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
	1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
};
static const float quadVertTex3D[] = {
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};
static const float quadVertNormTex3D[] = {
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
};
static const uint16_t quadInd[] = {
	0, 1, 2, 0, 2, 3,
};
