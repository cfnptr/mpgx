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

layout(location = 0) in vec3 f_FragDir;
layout(location = 1) in float f_TexCoord;

layout(location = 0) out vec4 o_Color;

layout(push_constant) uniform FragmentPushConstants
{
    layout(offset = 64) vec4 sunDir;
    layout(offset = 80) vec4 sunColor;
} fpc;

layout(binding = 0) uniform sampler2D u_Texture;

vec4 getSkyColor()
{
    vec2 texCoords = vec2(max(fpc.sunDir.y, 0.0), f_TexCoord);
    return texture(u_Texture, texCoords);
}
float getSunLight()
{
    vec3 fragDir = normalize(f_FragDir);
    return pow(max(dot(fragDir, fpc.sunDir.xyz), 0.0), 4096.0); // TODO: sun size
}
void main()
{
    o_Color = (fpc.sunColor * getSunLight()) + getSkyColor();
    gl_FragDepth = 0.9999999;
}
