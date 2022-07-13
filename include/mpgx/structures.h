// Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
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

#pragma once
#include "cmmt/color.h"
#include "cmmt/vector.h"
#include "cmmt/matrix.h"

/*
 * GLSL 2D single floating vector structure.
 */
typedef struct vec2
{
	float x, y;
} vec2;
/*
 * GLSL 3D single floating vector structure.
 */
typedef struct vec3
{
	float x, y, z;
} vec3;
/*
 * GLSL 4D single floating vector structure.
 */
typedef struct vec4
{
	float x, y, z, w;
} vec4;

/*
 * GLSL 2D double floating vector structure.
 */
typedef struct dvec2
{
	double x, y;
} dvec2;
/*
 * GLSL 3D double floating vector structure.
 */
typedef struct dvec3
{
	double x, y, z;
} dvec3;
/*
 * GLSL 4D double floating vector structure.
 */
typedef struct dvec4
{
	double x, y, z, w;
} dvec4;

/*
 * GLSL 2D signed integer vector structure.
 */
typedef struct ivec2
{
	int32_t x, y;
} ivec2;
/*
 * GLSL 3D signed integer vector structure.
 */
typedef struct ivec3
{
	int32_t x, y, z;
} ivec3;
/*
 * GLSL 4D signed integer vector structure.
 */
typedef struct ivec4
{
	int32_t x, y, z, w;
} ivec4;

/*
 * GLSL 2D unsigned integer vector structure.
 */
typedef struct uvec2
{
	uint32_t x, y;
} uvec2;
/*
 * GLSL 3D unsigned integer vector structure.
 */
typedef struct uvec3
{
	uint32_t x, y, z;
} uvec3;
/*
 * GLSL 4D unsigned integer vector structure.
 */
typedef struct uvec4
{
	uint32_t x, y, z, w;
} uvec4;

/*
 * GLSL 2D boolean vector structure.
 */
typedef struct bvec2
{
	uint32_t x, y;
} bvec2;
/*
 * GLSL 3D boolean vector structure.
 */
typedef struct bvec3
{
	uint32_t x, y, z;
} bvec3;
/*
 * GLSL 4D boolean vector structure.
 */
typedef struct bvec4
{
	uint32_t x, y, z, w;
} bvec4;

/*
 * GLSL 2x2 single floating matrix structure.
 */
typedef struct mat2
{
	float m00, m01;
	float m10, m11;
} mat2;
/*
 * GLSL 3x3 single floating matrix structure.
 */
typedef struct mat3
{
	float m00, m01, m02;
	float m10, m11, m12;
	float m20, m21, m22;
} mat3;
/*
 * GLSL 4x4 single floating matrix structure.
 */
typedef struct mat4
{
	float m00, m01, m02, m03;
	float m10, m11, m12, m13;
	float m20, m21, m22, m23;
	float m30, m31, m32, m33;
} mat4;

/*
 * GLSL 2x2 double floating matrix structure.
 */
typedef struct dmat2
{
	double m00, m01;
	double m10, m11;
} dmat2;
/*
 * GLSL 3x3 double floating matrix structure.
 */
typedef struct dmat3
{
	double m00, m01, m02;
	double m10, m11, m12;
	double m20, m21, m22;
} dmat3;
/*
 * GLSL 4x4 double floating matrix structure.
 */
typedef struct dmat4
{
	double m00, m01, m02, m03;
	double m10, m11, m12, m13;
	double m20, m21, m22, m23;
	double m30, m31, m32, m33;
} dmat4;

// TODO: different sided matrices

/*
 * Convert CMMT to GLSL 2D single floating vector.
 * vector - CMMT vector value.
 */
inline static vec2 cmmtToVec2(Vec2F vector)
{
	vec2 result;
	result.x = (float)vector.x;
	result.y = (float)vector.y;
	return result;
}
/*
 * Convert CMMT to GLSL 3D single floating vector.
 * vector - CMMT vector value.
 */
inline static vec3 cmmtToVec3(Vec3F vector)
{
	vec3 result;
	result.x = (float)vector.x;
	result.y = (float)vector.y;
	result.z = (float)vector.z;
	return result;
}
/*
 * Convert CMMT to GLSL 4D single floating vector.
 * vector - CMMT vector value.
 */
inline static vec4 cmmtToVec4(Vec4F vector)
{
	vec4 result;
	result.x = (float)vector.x;
	result.y = (float)vector.y;
	result.z = (float)vector.z;
	result.w = (float)vector.w;
	return result;
}

/*
 * Convert CMMT to GLSL 2D signed integer vector.
 * vector - CMMT vector value.
 */
inline static ivec2 cmmtToIvec2(Vec2I vector)
{
	ivec2 result;
	result.x = (int32_t)vector.x;
	result.y = (int32_t)vector.y;
	return result;
}
/*
 * Convert CMMT to GLSL 3D signed integer vector.
 * vector - CMMT vector value.
 */
inline static ivec3 cmmtToIvec3(Vec3I vector)
{
	ivec3 result;
	result.x = (int32_t)vector.x;
	result.y = (int32_t)vector.y;
	result.z = (int32_t)vector.z;
	return result;
}
/*
 * Convert CMMT to GLSL 4D signed integer vector.
 * vector - CMMT vector value.
 */
inline static ivec4 cmmtToIvec4(Vec4I vector)
{
	ivec4 result;
	result.x = (int32_t)vector.x;
	result.y = (int32_t)vector.y;
	result.z = (int32_t)vector.z;
	result.w = (int32_t)vector.w;
	return result;
}

/*
 * Convert CMMT to GLSL 4x4 single floating matrix.
 * matrix - CMMT matrix value.
 */
inline static mat4 cmmtToMat4(Mat4F matrix)
{
	mat4 result;
	result.m00 = (float)matrix.m00;
	result.m01 = (float)matrix.m01;
	result.m02 = (float)matrix.m02;
	result.m03 = (float)matrix.m03;

	result.m10 = (float)matrix.m10;
	result.m11 = (float)matrix.m11;
	result.m12 = (float)matrix.m12;
	result.m13 = (float)matrix.m13;

	result.m20 = (float)matrix.m20;
	result.m21 = (float)matrix.m21;
	result.m22 = (float)matrix.m22;
	result.m23 = (float)matrix.m23;

	result.m30 = (float)matrix.m30;
	result.m31 = (float)matrix.m31;
	result.m32 = (float)matrix.m32;
	result.m33 = (float)matrix.m33;
	return result;
}

/*
 * Convert CMMT to GLSL 4D single floating vector.
 * linearColor - CMMT linear color value.
 */
inline static vec4 cmmtColorToVec4(LinearColor linearColor)
{
	vec4 result;
	result.x = (float)linearColor.r;
	result.y = (float)linearColor.g;
	result.z = (float)linearColor.b;
	result.w = (float)linearColor.a;
	return result;
}
