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

in vec2 f_TexCoords;
layout(location = 0) out vec4 o_Color;

uniform sampler2D u_Texture;
uniform vec4 u_Color;

void main()
{
    vec4 color = texture(u_Texture, f_TexCoords);
    o_Color = color * u_Color;
}
