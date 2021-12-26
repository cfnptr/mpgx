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

#pragma once
#include <stdint.h>

static const float cubeTriangleVertices[] = {
	-1.0f, -1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,
};
static const float cubeLineVertices[] = {
	-1.0f, -1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
};
static const float cubeTriangleVerticesNormals[] = {
	-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
	1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
	-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
	-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
	1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f,
	-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
	-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
	1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
	1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
	1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
};
static const float cubeTriangleVerticesCoords[] = {
	-1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
	-1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
	-1.0f, -1.0f, -1.0f, 0.0f, 1.0f,
	1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
	-1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
};
static const float cubeTriangleVerticesNormalsCoords[] = {
	-1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	-1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	-1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	-1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
	1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
};
static const uint16_t cubeTriangleIndices[] = {
	0, 1, 2, 0, 2, 3,
	4, 5, 6, 4, 6, 7,
	8, 9, 10, 8, 10, 11,
	12, 13, 14, 12, 14, 15,
	16, 17, 18, 16, 18, 19,
	20, 21, 22, 20, 22, 23,
};
static const uint16_t cubeLineIndices[] = {
	0, 1, 1, 2, 2, 3, 3, 0,
	4, 5, 5, 6, 6, 7, 7, 4,
	3, 4, 7, 0,
 	1, 6, 5, 2,
};
