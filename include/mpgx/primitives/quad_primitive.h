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
static const uint16_t quadTriInd[] = {
	0, 1, 2, 0, 2, 3,
};
static const uint16_t quadLineInd[] = {
	0, 1, 1, 2, 2, 3, 3, 0,
};
