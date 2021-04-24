#pragma once
#include "cmmt/vector.h"

inline static float colToVal(
	uint8_t r)
{
	return (float)r / 255.0f;
}
inline static Vec2F colToVec2F(
	uint8_t r,
	uint8_t g)
{
	return vec2F(
		(float)r / 255.0f,
		(float)g / 255.0f);
}
inline static Vec3F colToVec3F(
	uint8_t r,
	uint8_t g,
	uint8_t b)
{
	return vec3F(
		(float)r / 255.0f,
		(float)g / 255.0f,
		(float)b / 255.0f);
}
inline static Vec4F colToVec4F(
	uint8_t r,
	uint8_t g,
	uint8_t b,
	uint8_t a)
{
	return vec4F(
		(float)r / 255.0f,
		(float)g / 255.0f,
		(float)b / 255.0f,
		(float)a / 255.0f);
}