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

uniform int u_Radius;
uniform int u_Offset;
uniform sampler2D u_Buffer;

// sigma = radius / 2
const float c_Weights[] = float[]
(
    1.000000000,
    0.786986053, 0.106506981,
    0.402619958, 0.244201347, 0.054488685,
    0.270682156, 0.216745317, 0.111280762, 0.036632847,
    0.204163685, 0.180173829, 0.123831540, 0.066282243, 0.027630551,
    0.163967207, 0.151360810, 0.119064629, 0.079811409, 0.045589000, 0.022190548,
    0.137022823, 0.129618034, 0.109719291, 0.083108537, 0.056331765, 0.034166943, 0.018544022,
    0.117695794, 0.112988606, 0.099966787, 0.081512496, 0.061254792, 0.042423189, 0.027077837, 0.015928393,
    0.103152618, 0.099978946, 0.091031864, 0.077863678, 0.062565222, 0.047226712, 0.033488750, 0.022308318, 0.013960189,
    0.091811255, 0.089572065, 0.083176881, 0.073516704, 0.061847590, 0.049523681, 0.037744734, 0.027381247, 0.018906163, 0.012425303,
    0.082718462, 0.081080534, 0.076358765, 0.069092274, 0.060065933, 0.050171286, 0.040263399, 0.031045157, 0.022998819, 0.016369877, 0.011194727,
    0.075265467, 0.074031636, 0.070450172, 0.064861946, 0.057775140, 0.049789209, 0.041511897, 0.033485215, 0.026132250, 0.019730752, 0.014412975, 0.010186073,
    0.069045149, 0.068092816, 0.065313913, 0.060932133, 0.055287033, 0.048790637, 0.041878000, 0.034960020, 0.028385310, 0.022415679, 0.017216561, 0.012861072, 0.009344245,
    0.063774936, 0.063024648, 0.060826343, 0.057331566, 0.052773602, 0.047441728, 0.041650973, 0.035711706, 0.029903147, 0.024453670, 0.019529542, 0.015232138, 0.011602473, 0.008630998,
    0.059252501, 0.058650956, 0.056882720, 0.054053333, 0.050327048, 0.045911055, 0.041036464, 0.035938457, 0.030837970, 0.025926804, 0.021357432, 0.017237963, 0.013632003, 0.010562587, 0.008018954,
    0.055329189, 0.054839555, 0.053396493, 0.051075280, 0.047994114, 0.044304151, 0.040177237, 0.035792738, 0.031324852, 0.026931608, 0.022746511, 0.018873241, 0.015383579, 0.012318207, 0.009689845, 0.007487992,
    0.051893312, 0.051489476, 0.050296724, 0.048369884, 0.045795687, 0.042686272, 0.039171126, 0.035388164, 0.031474885, 0.027560329, 0.023758490, 0.020163568, 0.016847292, 0.013858204, 0.011222715, 0.008947529, 0.007022996
);

vec3 calcBlurColor(sampler2D buffer, vec2 texCoords, int radius, int offset)
{
    vec2 texelSize = 1.0 / textureSize(buffer, 0);
    vec3 result = texture(buffer, texCoords).rgb * c_Weights[offset];
    
    for(int i = 1; i <= radius; i++)
    {
        vec2 coords = vec2(0.0, texelSize.y * i);
        float weight = c_Weights[offset + i];
        result += texture(buffer, texCoords + coords).rgb * weight;
        result += texture(buffer, texCoords - coords).rgb * weight;
    }
    
    return result;
}
void main()
{
    vec3 blurColor = calcBlurColor(
        u_Buffer, f_TexCoords, u_Radius, u_Offset);
    o_Color = vec4(blurColor, 1.0);
}
