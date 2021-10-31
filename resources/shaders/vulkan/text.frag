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

layout(location = 0) in vec2 f_TexCoords;
layout(location = 0) out vec4 o_Color;

layout(push_constant) uniform FragmentPushConstants
{
    layout(offset = 64) vec4 color;
} fpc;

layout(binding = 0) uniform sampler2D u_Texture;

void main()
{
    float color = texture(u_Texture, f_TexCoords).r;
    o_Color = vec4(fpc.color.rgb, fpc.color.a * color);
}