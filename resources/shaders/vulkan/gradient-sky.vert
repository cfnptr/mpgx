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

#version 420

layout(location = 0) in vec3 v_Position;

layout(location = 0) out vec3 f_FragDir;
layout(location = 1) out float f_TexCoord;

layout(push_constant) uniform PushConstant
{
    mat4 mvp;
} p;

void main()
{
    gl_Position = p.mvp * vec4(v_Position, 1.0);
    f_FragDir = v_Position;
    f_TexCoord = clamp(v_Position.y, 0.0, 1.0);
}
