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
#include <stdint.h>

static const float skyIcosphereVertices[] = {
	-0.276388f, 0.447220f, -0.850649f,
	-0.257937f, 0.550685f, -0.793860f,
	-0.377183f, 0.476988f, -0.793861f,
	0.723607f, 0.447220f, -0.525725f,
	0.675300f, 0.550685f, -0.490628f,
	0.638453f, 0.476987f, -0.604038f,
	0.723607f, 0.447220f, 0.525725f,
	0.675300f, 0.550685f, 0.490628f,
	0.771771f, 0.476987f, 0.420539f,
	-0.276388f, 0.447220f, 0.850649f,
	-0.257937f, 0.550685f, 0.793860f,
	-0.161465f, 0.476988f, 0.863951f,
	-0.894426f, 0.447216f, 0.000000f,
	-0.834716f, 0.550681f, 0.000000f,
	-0.871565f, 0.476984f, 0.113408f,
	-0.119245f, 0.992865f, 0.000000f,
	0.000000f, 1.000000f, 0.000000f,
	-0.036848f, 0.992865f, 0.113408f,
	-0.251147f, 0.967949f, 0.000000f,
	-0.166198f, 0.978672f, 0.120749f,
	-0.389673f, 0.920953f, 0.000000f,
	-0.307167f, 0.943208f, 0.126518f,
	-0.525730f, 0.850652f, 0.000000f,
	-0.451375f, 0.882854f, 0.129731f,
	-0.649456f, 0.760399f, 0.000000f,
	-0.587783f, 0.798549f, 0.129731f,
	-0.753442f, 0.657515f, 0.000000f,
	-0.706258f, 0.696558f, 0.126519f,
	-0.801022f, 0.586331f, 0.120750f,
	-0.077607f, 0.967950f, 0.238853f,
	-0.215245f, 0.943209f, 0.253036f,
	-0.120413f, 0.920955f, 0.370598f,
	-0.361800f, 0.894429f, 0.262863f,
	-0.262862f, 0.882855f, 0.389192f,
	-0.162456f, 0.850654f, 0.499995f,
	-0.506729f, 0.819912f, 0.266403f,
	-0.409951f, 0.819913f, 0.399604f,
	-0.305014f, 0.798552f, 0.518924f,
	-0.200688f, 0.760403f, 0.617666f,
	-0.638194f, 0.723610f, 0.262864f,
	-0.550008f, 0.733353f, 0.399605f,
	-0.447209f, 0.723612f, 0.525728f,
	-0.338569f, 0.696561f, 0.632593f,
	-0.232822f, 0.657519f, 0.716563f,
	-0.747366f, 0.614342f, 0.253038f,
	-0.672087f, 0.629942f, 0.389194f,
	-0.577830f, 0.629943f, 0.518926f,
	-0.471599f, 0.614344f, 0.632594f,
	-0.362366f, 0.586334f, 0.724502f,
	-0.831051f, 0.502299f, 0.238853f,
	-0.769872f, 0.519570f, 0.370600f,
	-0.688189f, 0.525736f, 0.499997f,
	-0.590366f, 0.519572f, 0.617668f,
	-0.483971f, 0.502302f, 0.716565f,
	-0.377183f, 0.476988f, 0.793861f,
	0.096471f, 0.992865f, 0.070089f,
	0.063483f, 0.978672f, 0.195376f,
	0.025408f, 0.943209f, 0.331227f,
	-0.016098f, 0.882855f, 0.469369f,
	-0.058250f, 0.798552f, 0.599101f,
	-0.097915f, 0.696561f, 0.710785f,
	-0.132684f, 0.586334f, 0.799129f,
	0.203181f, 0.967950f, 0.147618f,
	0.174138f, 0.943209f, 0.282901f,
	0.315251f, 0.920955f, 0.229040f,
	0.138197f, 0.894430f, 0.425319f,
	0.288916f, 0.882855f, 0.370262f,
	0.425323f, 0.850654f, 0.309011f,
	0.096779f, 0.819914f, 0.564248f,
	0.253366f, 0.819914f, 0.513369f,
	0.399272f, 0.798552f, 0.450440f,
	0.525420f, 0.760403f, 0.381735f,
	0.052790f, 0.723612f, 0.688185f,
	0.210088f, 0.733355f, 0.646571f,
	0.361804f, 0.723612f, 0.587778f,
	0.497009f, 0.696561f, 0.517479f,
	0.609547f, 0.657519f, 0.442856f,
	0.009708f, 0.614345f, 0.788978f,
	0.162463f, 0.629944f, 0.759458f,
	0.314971f, 0.629944f, 0.709904f,
	0.455902f, 0.614344f, 0.643998f,
	0.577066f, 0.586334f, 0.568513f,
	-0.029639f, 0.502302f, 0.864184f,
	0.114564f, 0.519572f, 0.846711f,
	0.262869f, 0.525738f, 0.809012f,
	0.405008f, 0.519572f, 0.752338f,
	0.531941f, 0.502302f, 0.681712f,
	0.638452f, 0.476987f, 0.604038f,
	0.096471f, 0.992865f, -0.070089f,
	0.205432f, 0.978671f, 0.000000f,
	0.322868f, 0.943208f, 0.078192f,
	0.441423f, 0.882855f, 0.160354f,
	0.551779f, 0.798551f, 0.240532f,
	0.645740f, 0.696561f, 0.312768f,
	0.719015f, 0.586334f, 0.373135f,
	0.203181f, 0.967950f, -0.147618f,
	0.322868f, 0.943208f, -0.078192f,
	0.315251f, 0.920955f, -0.229040f,
	0.447210f, 0.894429f, -0.000000f,
	0.441423f, 0.882855f, -0.160354f,
	0.425323f, 0.850654f, -0.309011f,
	0.566539f, 0.819912f, 0.082322f,
	0.566539f, 0.819912f, -0.082322f,
	0.551779f, 0.798551f, -0.240532f,
	0.525420f, 0.760403f, -0.381735f,
	0.670817f, 0.723611f, 0.162457f,
	0.679848f, 0.733353f, -0.000000f,
	0.670817f, 0.723611f, -0.162457f,
	0.645740f, 0.696560f, -0.312768f,
	0.609547f, 0.657519f, -0.442856f,
	0.753363f, 0.614343f, 0.234576f,
	0.772492f, 0.629942f, 0.080177f,
	0.772492f, 0.629942f, -0.080177f,
	0.753363f, 0.614343f, -0.234576f,
	0.719015f, 0.586334f, -0.373135f,
	0.812729f, 0.502301f, 0.295238f,
	0.840673f, 0.519571f, 0.152694f,
	0.850648f, 0.525736f, 0.000000f,
	0.840673f, 0.519571f, -0.152694f,
	0.812729f, 0.502301f, -0.295238f,
	0.771771f, 0.476987f, -0.420539f,
	-0.036848f, 0.992865f, -0.113408f,
	0.063483f, 0.978672f, -0.195376f,
	0.174138f, 0.943209f, -0.282901f,
	0.288916f, 0.882855f, -0.370262f,
	0.399272f, 0.798552f, -0.450440f,
	0.497009f, 0.696561f, -0.517479f,
	0.577066f, 0.586334f, -0.568513f,
	-0.077607f, 0.967950f, -0.238853f,
	0.025408f, 0.943209f, -0.331227f,
	-0.120413f, 0.920955f, -0.370598f,
	0.138197f, 0.894430f, -0.425319f,
	-0.016098f, 0.882855f, -0.469369f,
	-0.162456f, 0.850654f, -0.499995f,
	0.253366f, 0.819914f, -0.513369f,
	0.096779f, 0.819914f, -0.564248f,
	-0.058250f, 0.798552f, -0.599101f,
	-0.200688f, 0.760403f, -0.617666f,
	0.361804f, 0.723612f, -0.587778f,
	0.210088f, 0.733355f, -0.646571f,
	0.052790f, 0.723612f, -0.688185f,
	-0.097915f, 0.696561f, -0.710785f,
	-0.232822f, 0.657519f, -0.716563f,
	0.455902f, 0.614345f, -0.643998f,
	0.314971f, 0.629944f, -0.709904f,
	0.162463f, 0.629944f, -0.759458f,
	0.009708f, 0.614345f, -0.788978f,
	-0.132684f, 0.586335f, -0.799129f,
	0.531941f, 0.502302f, -0.681712f,
	0.405008f, 0.519572f, -0.752338f,
	0.262869f, 0.525738f, -0.809012f,
	0.114564f, 0.519573f, -0.846711f,
	-0.029639f, 0.502302f, -0.864184f,
	-0.161465f, 0.476988f, -0.863951f,
	-0.166198f, 0.978672f, -0.120749f,
	-0.215245f, 0.943209f, -0.253036f,
	-0.262862f, 0.882855f, -0.389192f,
	-0.305014f, 0.798552f, -0.518924f,
	-0.338569f, 0.696561f, -0.632593f,
	-0.362366f, 0.586334f, -0.724502f,
	-0.307167f, 0.943208f, -0.126518f,
	-0.361800f, 0.894429f, -0.262863f,
	-0.451375f, 0.882854f, -0.129731f,
	-0.409951f, 0.819913f, -0.399604f,
	-0.506729f, 0.819912f, -0.266403f,
	-0.587783f, 0.798549f, -0.129731f,
	-0.447209f, 0.723612f, -0.525728f,
	-0.550008f, 0.733353f, -0.399605f,
	-0.638194f, 0.723610f, -0.262864f,
	-0.706258f, 0.696558f, -0.126519f,
	-0.471599f, 0.614344f, -0.632594f,
	-0.577830f, 0.629943f, -0.518926f,
	-0.672087f, 0.629942f, -0.389194f,
	-0.747366f, 0.614342f, -0.253038f,
	-0.801022f, 0.586331f, -0.120750f,
	-0.483971f, 0.502302f, -0.716565f,
	-0.590366f, 0.519572f, -0.617668f,
	-0.688189f, 0.525736f, -0.499997f,
	-0.769872f, 0.519570f, -0.370600f,
	-0.831051f, 0.502299f, -0.238853f,
	-0.871565f, 0.476984f, -0.113408f,
	-0.931188f, 0.357738f, 0.070089f,
	-0.956626f, 0.251149f, 0.147618f,
	-0.903740f, 0.380897f, 0.195376f,
	-0.964711f, 0.129893f, 0.229041f,
	-0.921508f, 0.266063f, 0.282902f,
	-0.951058f, 0.000000f, 0.309013f,
	-0.918856f, 0.136410f, 0.370264f,
	-0.854992f, 0.399094f, 0.331229f,
	-0.861804f, 0.276396f, 0.425322f,
	-0.782446f, 0.409229f, 0.469371f,
	-0.892805f, -0.000000f, 0.450443f,
	-0.846660f, 0.140059f, 0.513372f,
	-0.776630f, 0.280118f, 0.564251f,
	-0.688190f, 0.409230f, 0.599104f,
	-0.809019f, 0.000000f, 0.587782f,
	-0.749882f, 0.140059f, 0.646576f,
	-0.670821f, 0.276397f, 0.688189f,
	-0.579226f, 0.399096f, 0.710788f,
	-0.704293f, 0.000000f, 0.709909f,
	-0.636088f, 0.136411f, 0.759463f,
	-0.553820f, 0.266065f, 0.788982f,
	-0.465085f, 0.380900f, 0.799132f,
	-0.587786f, -0.000000f, 0.809017f,
	-0.515946f, 0.129894f, 0.846716f,
	-0.436007f, 0.251152f, 0.864188f,
	-0.354409f, 0.357742f, 0.863953f,
	-0.221089f, 0.357741f, 0.907271f,
	-0.155215f, 0.251152f, 0.955422f,
	-0.093451f, 0.380900f, 0.919882f,
	-0.080276f, 0.129894f, 0.988273f,
	-0.015700f, 0.266064f, 0.963827f,
	-0.000000f, 0.000000f, 1.000000f,
	0.068205f, 0.136410f, 0.988302f,
	0.050816f, 0.399096f, 0.915500f,
	0.138198f, 0.276397f, 0.951055f,
	0.204615f, 0.409230f, 0.889193f,
	0.152509f, -0.000000f, 0.988302f,
	0.226618f, 0.140059f, 0.963861f,
	0.296648f, 0.280118f, 0.912981f,
	0.357124f, 0.409230f, 0.839639f,
	0.309016f, -0.000000f, 0.951057f,
	0.383207f, 0.140059f, 0.912982f,
	0.447215f, 0.276397f, 0.850649f,
	0.497012f, 0.399096f, 0.770520f,
	0.457527f, -0.000000f, 0.889196f,
	0.525732f, 0.136410f, 0.839642f,
	0.579229f, 0.266065f, 0.770522f,
	0.616302f, 0.380900f, 0.689266f,
	0.587786f, -0.000000f, 0.809017f,
	0.645839f, 0.129894f, 0.752343f,
	0.687159f, 0.251152f, 0.681715f,
	0.712150f, 0.357741f, 0.604039f,
	0.794547f, 0.357741f, 0.490629f,
	0.860698f, 0.251151f, 0.442858f,
	0.845982f, 0.380899f, 0.373136f,
	0.915098f, 0.129893f, 0.381737f,
	0.911804f, 0.266063f, 0.312769f,
	0.951058f, 0.000000f, 0.309013f,
	0.961008f, 0.136410f, 0.240533f,
	0.886396f, 0.399095f, 0.234577f,
	0.947213f, 0.276396f, 0.162458f,
	0.908902f, 0.409229f, 0.080178f,
	0.987059f, 0.000000f, 0.160355f,
	0.986715f, 0.140059f, 0.082322f,
	0.959966f, 0.280117f, -0.000000f,
	0.908902f, 0.409229f, -0.080178f,
	1.000000f, 0.000001f, -0.000000f,
	0.986715f, 0.140059f, -0.082323f,
	0.947213f, 0.276397f, -0.162458f,
	0.886395f, 0.399095f, -0.234577f,
	0.987059f, 0.000001f, -0.160356f,
	0.961008f, 0.136411f, -0.240534f,
	0.911803f, 0.266065f, -0.312769f,
	0.845982f, 0.380900f, -0.373136f,
	0.951058f, -0.000000f, -0.309013f,
	0.915098f, 0.129893f, -0.381737f,
	0.860698f, 0.251151f, -0.442858f,
	0.794547f, 0.357741f, -0.490629f,
	0.712150f, 0.357741f, -0.604039f,
	0.687159f, 0.251152f, -0.681715f,
	0.616302f, 0.380900f, -0.689266f,
	0.645839f, 0.129894f, -0.752342f,
	0.579229f, 0.266064f, -0.770521f,
	0.587786f, 0.000000f, -0.809017f,
	0.525732f, 0.136410f, -0.839641f,
	0.497012f, 0.399096f, -0.770520f,
	0.447216f, 0.276397f, -0.850649f,
	0.357124f, 0.409230f, -0.839639f,
	0.457527f, -0.000001f, -0.889196f,
	0.383208f, 0.140059f, -0.912982f,
	0.296649f, 0.280118f, -0.912981f,
	0.204616f, 0.409230f, -0.889193f,
	0.309017f, -0.000001f, -0.951056f,
	0.226619f, 0.140059f, -0.963861f,
	0.138199f, 0.276397f, -0.951055f,
	0.050817f, 0.399096f, -0.915500f,
	0.152509f, -0.000000f, -0.988302f,
	0.068206f, 0.136410f, -0.988302f,
	-0.015699f, 0.266064f, -0.963827f,
	-0.093451f, 0.380900f, -0.919882f,
	0.000000f, -0.000000f, -1.000000f,
	-0.080276f, 0.129894f, -0.988273f,
	-0.155215f, 0.251152f, -0.955422f,
	-0.221089f, 0.357741f, -0.907271f,
	-0.354409f, 0.357742f, -0.863953f,
	-0.436007f, 0.251152f, -0.864188f,
	-0.465085f, 0.380900f, -0.799132f,
	-0.515946f, 0.129894f, -0.846716f,
	-0.553820f, 0.266064f, -0.788983f,
	-0.587786f, 0.000000f, -0.809017f,
	-0.636088f, 0.136410f, -0.759463f,
	-0.579226f, 0.399096f, -0.710788f,
	-0.670820f, 0.276396f, -0.688190f,
	-0.688190f, 0.409229f, -0.599104f,
	-0.704293f, -0.000001f, -0.709909f,
	-0.749882f, 0.140058f, -0.646576f,
	-0.776630f, 0.280116f, -0.564252f,
	-0.782446f, 0.409228f, -0.469372f,
	-0.809019f, -0.000002f, -0.587783f,
	-0.846659f, 0.140057f, -0.513373f,
	-0.861804f, 0.276394f, -0.425323f,
	-0.854992f, 0.399093f, -0.331230f,
	-0.892805f, -0.000002f, -0.450444f,
	-0.918856f, 0.136407f, -0.370266f,
	-0.921508f, 0.266061f, -0.282904f,
	-0.903740f, 0.380896f, -0.195377f,
	-0.951058f, -0.000000f, -0.309013f,
	-0.964711f, 0.129893f, -0.229041f,
	-0.956626f, 0.251149f, -0.147618f,
	-0.931188f, 0.357738f, -0.070089f,
	-0.298886f, 0.253934f, 0.919883f,
	-0.379681f, 0.133033f, 0.915502f,
	-0.457527f, -0.000000f, 0.889196f,
	-0.230948f, 0.133033f, 0.963828f,
	-0.309017f, -0.000000f, 0.951056f,
	-0.152509f, 0.000000f, 0.988302f,
	0.782501f, 0.253934f, 0.568515f,
	0.753368f, 0.133032f, 0.644002f,
	0.704293f, -0.000000f, 0.709910f,
	0.845290f, 0.133032f, 0.517482f,
	0.809018f, -0.000000f, 0.587783f,
	0.892805f, 0.000000f, 0.450444f,
	0.782501f, 0.253934f, -0.568515f,
	0.845290f, 0.133032f, -0.517482f,
	0.892805f, -0.000000f, -0.450444f,
	0.753368f, 0.133032f, -0.644002f,
	0.809018f, 0.000000f, -0.587783f,
	0.704293f, 0.000000f, -0.709910f,
	-0.298886f, 0.253934f, -0.919883f,
	-0.230948f, 0.133032f, -0.963828f,
	-0.152509f, -0.000000f, -0.988302f,
	-0.379681f, 0.133033f, -0.915502f,
	-0.309017f, 0.000000f, -0.951056f,
	-0.457527f, 0.000000f, -0.889196f,
	-0.967222f, 0.253931f, -0.000000f,
	-0.988023f, 0.133031f, -0.078192f,
	-0.987059f, -0.000000f, -0.160355f,
	-0.988023f, 0.133031f, 0.078192f,
	-1.000000f, 0.000000f, -0.000000f,
	-0.987059f, 0.000000f, 0.160355f,
	0.000000f, -1.000000f, -0.000000f,
};
static const uint16_t skyIcosphereIndices[] = {
	0, 1, 2,
	3, 4, 5,
	6, 7, 8,
	9, 10, 11,
	12, 13, 14,
	15, 16, 17,
	18, 15, 19,
	20, 18, 21,
	22, 20, 23,
	24, 22, 25,
	26, 24, 27,
	13, 26, 28,
	15, 17, 19,
	19, 17, 29,
	18, 19, 21,
	21, 19, 30,
	19, 29, 30,
	30, 29, 31,
	20, 21, 23,
	23, 21, 32,
	21, 30, 32,
	32, 30, 33,
	30, 31, 33,
	33, 31, 34,
	22, 23, 25,
	25, 23, 35,
	23, 32, 35,
	35, 32, 36,
	32, 33, 36,
	36, 33, 37,
	33, 34, 37,
	37, 34, 38,
	24, 25, 27,
	27, 25, 39,
	25, 35, 39,
	39, 35, 40,
	35, 36, 40,
	40, 36, 41,
	36, 37, 41,
	41, 37, 42,
	37, 38, 42,
	42, 38, 43,
	26, 27, 28,
	28, 27, 44,
	27, 39, 44,
	44, 39, 45,
	39, 40, 45,
	45, 40, 46,
	40, 41, 46,
	46, 41, 47,
	41, 42, 47,
	47, 42, 48,
	42, 43, 48,
	48, 43, 10,
	13, 28, 14,
	14, 28, 49,
	28, 44, 49,
	49, 44, 50,
	44, 45, 50,
	50, 45, 51,
	45, 46, 51,
	51, 46, 52,
	46, 47, 52,
	52, 47, 53,
	47, 48, 53,
	53, 48, 54,
	48, 10, 54,
	54, 10, 9,
	17, 16, 55,
	29, 17, 56,
	31, 29, 57,
	34, 31, 58,
	38, 34, 59,
	43, 38, 60,
	10, 43, 61,
	17, 55, 56,
	56, 55, 62,
	29, 56, 57,
	57, 56, 63,
	56, 62, 63,
	63, 62, 64,
	31, 57, 58,
	58, 57, 65,
	57, 63, 65,
	65, 63, 66,
	63, 64, 66,
	66, 64, 67,
	34, 58, 59,
	59, 58, 68,
	58, 65, 68,
	68, 65, 69,
	65, 66, 69,
	69, 66, 70,
	66, 67, 70,
	70, 67, 71,
	38, 59, 60,
	60, 59, 72,
	59, 68, 72,
	72, 68, 73,
	68, 69, 73,
	73, 69, 74,
	69, 70, 74,
	74, 70, 75,
	70, 71, 75,
	75, 71, 76,
	43, 60, 61,
	61, 60, 77,
	60, 72, 77,
	77, 72, 78,
	72, 73, 78,
	78, 73, 79,
	73, 74, 79,
	79, 74, 80,
	74, 75, 80,
	80, 75, 81,
	75, 76, 81,
	81, 76, 7,
	10, 61, 11,
	11, 61, 82,
	61, 77, 82,
	82, 77, 83,
	77, 78, 83,
	83, 78, 84,
	78, 79, 84,
	84, 79, 85,
	79, 80, 85,
	85, 80, 86,
	80, 81, 86,
	86, 81, 87,
	81, 7, 87,
	87, 7, 6,
	55, 16, 88,
	62, 55, 89,
	64, 62, 90,
	67, 64, 91,
	71, 67, 92,
	76, 71, 93,
	7, 76, 94,
	55, 88, 89,
	89, 88, 95,
	62, 89, 90,
	90, 89, 96,
	89, 95, 96,
	96, 95, 97,
	64, 90, 91,
	91, 90, 98,
	90, 96, 98,
	98, 96, 99,
	96, 97, 99,
	99, 97, 100,
	67, 91, 92,
	92, 91, 101,
	91, 98, 101,
	101, 98, 102,
	98, 99, 102,
	102, 99, 103,
	99, 100, 103,
	103, 100, 104,
	71, 92, 93,
	93, 92, 105,
	92, 101, 105,
	105, 101, 106,
	101, 102, 106,
	106, 102, 107,
	102, 103, 107,
	107, 103, 108,
	103, 104, 108,
	108, 104, 109,
	76, 93, 94,
	94, 93, 110,
	93, 105, 110,
	110, 105, 111,
	105, 106, 111,
	111, 106, 112,
	106, 107, 112,
	112, 107, 113,
	107, 108, 113,
	113, 108, 114,
	108, 109, 114,
	114, 109, 4,
	7, 94, 8,
	8, 94, 115,
	94, 110, 115,
	115, 110, 116,
	110, 111, 116,
	116, 111, 117,
	111, 112, 117,
	117, 112, 118,
	112, 113, 118,
	118, 113, 119,
	113, 114, 119,
	119, 114, 120,
	114, 4, 120,
	120, 4, 3,
	88, 16, 121,
	95, 88, 122,
	97, 95, 123,
	100, 97, 124,
	104, 100, 125,
	109, 104, 126,
	4, 109, 127,
	88, 121, 122,
	122, 121, 128,
	95, 122, 123,
	123, 122, 129,
	122, 128, 129,
	129, 128, 130,
	97, 123, 124,
	124, 123, 131,
	123, 129, 131,
	131, 129, 132,
	129, 130, 132,
	132, 130, 133,
	100, 124, 125,
	125, 124, 134,
	124, 131, 134,
	134, 131, 135,
	131, 132, 135,
	135, 132, 136,
	132, 133, 136,
	136, 133, 137,
	104, 125, 126,
	126, 125, 138,
	125, 134, 138,
	138, 134, 139,
	134, 135, 139,
	139, 135, 140,
	135, 136, 140,
	140, 136, 141,
	136, 137, 141,
	141, 137, 142,
	109, 126, 127,
	127, 126, 143,
	126, 138, 143,
	143, 138, 144,
	138, 139, 144,
	144, 139, 145,
	139, 140, 145,
	145, 140, 146,
	140, 141, 146,
	146, 141, 147,
	141, 142, 147,
	147, 142, 1,
	4, 127, 5,
	5, 127, 148,
	127, 143, 148,
	148, 143, 149,
	143, 144, 149,
	149, 144, 150,
	144, 145, 150,
	150, 145, 151,
	145, 146, 151,
	151, 146, 152,
	146, 147, 152,
	152, 147, 153,
	147, 1, 153,
	153, 1, 0,
	121, 16, 15,
	128, 121, 154,
	130, 128, 155,
	133, 130, 156,
	137, 133, 157,
	142, 137, 158,
	1, 142, 159,
	121, 15, 154,
	154, 15, 18,
	128, 154, 155,
	155, 154, 160,
	154, 18, 160,
	160, 18, 20,
	130, 155, 156,
	156, 155, 161,
	155, 160, 161,
	161, 160, 162,
	160, 20, 162,
	162, 20, 22,
	133, 156, 157,
	157, 156, 163,
	156, 161, 163,
	163, 161, 164,
	161, 162, 164,
	164, 162, 165,
	162, 22, 165,
	165, 22, 24,
	137, 157, 158,
	158, 157, 166,
	157, 163, 166,
	166, 163, 167,
	163, 164, 167,
	167, 164, 168,
	164, 165, 168,
	168, 165, 169,
	165, 24, 169,
	169, 24, 26,
	142, 158, 159,
	159, 158, 170,
	158, 166, 170,
	170, 166, 171,
	166, 167, 171,
	171, 167, 172,
	167, 168, 172,
	172, 168, 173,
	168, 169, 173,
	173, 169, 174,
	169, 26, 174,
	174, 26, 13,
	1, 159, 2,
	2, 159, 175,
	159, 170, 175,
	175, 170, 176,
	170, 171, 176,
	176, 171, 177,
	171, 172, 177,
	177, 172, 178,
	172, 173, 178,
	178, 173, 179,
	173, 174, 179,
	179, 174, 180,
	174, 13, 180,
	180, 13, 12,
	181, 12, 14,
	182, 181, 183,
	184, 182, 185,
	186, 184, 187,
	181, 14, 183,
	183, 14, 49,
	182, 183, 185,
	185, 183, 188,
	183, 49, 188,
	188, 49, 50,
	184, 185, 187,
	187, 185, 189,
	185, 188, 189,
	189, 188, 190,
	188, 50, 190,
	190, 50, 51,
	186, 187, 191,
	191, 187, 192,
	187, 189, 192,
	192, 189, 193,
	189, 190, 193,
	193, 190, 194,
	190, 51, 194,
	194, 51, 52,
	191, 192, 195,
	195, 192, 196,
	192, 193, 196,
	196, 193, 197,
	193, 194, 197,
	197, 194, 198,
	194, 52, 198,
	198, 52, 53,
	195, 196, 199,
	199, 196, 200,
	196, 197, 200,
	200, 197, 201,
	197, 198, 201,
	201, 198, 202,
	198, 53, 202,
	202, 53, 54,
	199, 200, 203,
	203, 200, 204,
	200, 201, 204,
	204, 201, 205,
	201, 202, 205,
	205, 202, 206,
	202, 54, 206,
	206, 54, 9,
	207, 9, 11,
	208, 207, 209,
	210, 208, 211,
	212, 210, 213,
	207, 11, 209,
	209, 11, 82,
	208, 209, 211,
	211, 209, 214,
	209, 82, 214,
	214, 82, 83,
	210, 211, 213,
	213, 211, 215,
	211, 214, 215,
	215, 214, 216,
	214, 83, 216,
	216, 83, 84,
	212, 213, 217,
	217, 213, 218,
	213, 215, 218,
	218, 215, 219,
	215, 216, 219,
	219, 216, 220,
	216, 84, 220,
	220, 84, 85,
	217, 218, 221,
	221, 218, 222,
	218, 219, 222,
	222, 219, 223,
	219, 220, 223,
	223, 220, 224,
	220, 85, 224,
	224, 85, 86,
	221, 222, 225,
	225, 222, 226,
	222, 223, 226,
	226, 223, 227,
	223, 224, 227,
	227, 224, 228,
	224, 86, 228,
	228, 86, 87,
	225, 226, 229,
	229, 226, 230,
	226, 227, 230,
	230, 227, 231,
	227, 228, 231,
	231, 228, 232,
	228, 87, 232,
	232, 87, 6,
	233, 6, 8,
	234, 233, 235,
	236, 234, 237,
	238, 236, 239,
	233, 8, 235,
	235, 8, 115,
	234, 235, 237,
	237, 235, 240,
	235, 115, 240,
	240, 115, 116,
	236, 237, 239,
	239, 237, 241,
	237, 240, 241,
	241, 240, 242,
	240, 116, 242,
	242, 116, 117,
	238, 239, 243,
	243, 239, 244,
	239, 241, 244,
	244, 241, 245,
	241, 242, 245,
	245, 242, 246,
	242, 117, 246,
	246, 117, 118,
	243, 244, 247,
	247, 244, 248,
	244, 245, 248,
	248, 245, 249,
	245, 246, 249,
	249, 246, 250,
	246, 118, 250,
	250, 118, 119,
	247, 248, 251,
	251, 248, 252,
	248, 249, 252,
	252, 249, 253,
	249, 250, 253,
	253, 250, 254,
	250, 119, 254,
	254, 119, 120,
	251, 252, 255,
	255, 252, 256,
	252, 253, 256,
	256, 253, 257,
	253, 254, 257,
	257, 254, 258,
	254, 120, 258,
	258, 120, 3,
	259, 3, 5,
	260, 259, 261,
	262, 260, 263,
	264, 262, 265,
	259, 5, 261,
	261, 5, 148,
	260, 261, 263,
	263, 261, 266,
	261, 148, 266,
	266, 148, 149,
	262, 263, 265,
	265, 263, 267,
	263, 266, 267,
	267, 266, 268,
	266, 149, 268,
	268, 149, 150,
	264, 265, 269,
	269, 265, 270,
	265, 267, 270,
	270, 267, 271,
	267, 268, 271,
	271, 268, 272,
	268, 150, 272,
	272, 150, 151,
	269, 270, 273,
	273, 270, 274,
	270, 271, 274,
	274, 271, 275,
	271, 272, 275,
	275, 272, 276,
	272, 151, 276,
	276, 151, 152,
	273, 274, 277,
	277, 274, 278,
	274, 275, 278,
	278, 275, 279,
	275, 276, 279,
	279, 276, 280,
	276, 152, 280,
	280, 152, 153,
	277, 278, 281,
	281, 278, 282,
	278, 279, 282,
	282, 279, 283,
	279, 280, 283,
	283, 280, 284,
	280, 153, 284,
	284, 153, 0,
	285, 0, 2,
	286, 285, 287,
	288, 286, 289,
	290, 288, 291,
	285, 2, 287,
	287, 2, 175,
	286, 287, 289,
	289, 287, 292,
	287, 175, 292,
	292, 175, 176,
	288, 289, 291,
	291, 289, 293,
	289, 292, 293,
	293, 292, 294,
	292, 176, 294,
	294, 176, 177,
	290, 291, 295,
	295, 291, 296,
	291, 293, 296,
	296, 293, 297,
	293, 294, 297,
	297, 294, 298,
	294, 177, 298,
	298, 177, 178,
	295, 296, 299,
	299, 296, 300,
	296, 297, 300,
	300, 297, 301,
	297, 298, 301,
	301, 298, 302,
	298, 178, 302,
	302, 178, 179,
	299, 300, 303,
	303, 300, 304,
	300, 301, 304,
	304, 301, 305,
	301, 302, 305,
	305, 302, 306,
	302, 179, 306,
	306, 179, 180,
	303, 304, 307,
	307, 304, 308,
	304, 305, 308,
	308, 305, 309,
	305, 306, 309,
	309, 306, 310,
	306, 180, 310,
	310, 180, 12,
	206, 9, 207,
	205, 206, 311,
	204, 205, 312,
	203, 204, 313,
	206, 207, 311,
	311, 207, 208,
	205, 311, 312,
	312, 311, 314,
	311, 208, 314,
	314, 208, 210,
	204, 312, 313,
	313, 312, 315,
	312, 314, 315,
	315, 314, 316,
	314, 210, 316,
	316, 210, 212,
	232, 6, 233,
	231, 232, 317,
	230, 231, 318,
	229, 230, 319,
	232, 233, 317,
	317, 233, 234,
	231, 317, 318,
	318, 317, 320,
	317, 234, 320,
	320, 234, 236,
	230, 318, 319,
	319, 318, 321,
	318, 320, 321,
	321, 320, 322,
	320, 236, 322,
	322, 236, 238,
	258, 3, 259,
	257, 258, 323,
	256, 257, 324,
	255, 256, 325,
	258, 259, 323,
	323, 259, 260,
	257, 323, 324,
	324, 323, 326,
	323, 260, 326,
	326, 260, 262,
	256, 324, 325,
	325, 324, 327,
	324, 326, 327,
	327, 326, 328,
	326, 262, 328,
	328, 262, 264,
	284, 0, 285,
	283, 284, 329,
	282, 283, 330,
	281, 282, 331,
	284, 285, 329,
	329, 285, 286,
	283, 329, 330,
	330, 329, 332,
	329, 286, 332,
	332, 286, 288,
	282, 330, 331,
	331, 330, 333,
	330, 332, 333,
	333, 332, 334,
	332, 288, 334,
	334, 288, 290,
	310, 12, 181,
	309, 310, 335,
	308, 309, 336,
	307, 308, 337,
	310, 181, 335,
	335, 181, 182,
	309, 335, 336,
	336, 335, 338,
	335, 182, 338,
	338, 182, 184,
	308, 336, 337,
	337, 336, 339,
	336, 338, 339,
	339, 338, 340,
	338, 184, 340,
	340, 184, 186,
	339, 341, 337,
	337, 341, 307,
	307, 341, 303,
	303, 341, 299,
	299, 341, 295,
	295, 341, 290,
	290, 341, 334,
	334, 341, 333,
	333, 341, 331,
	331, 341, 281,
	281, 341, 277,
	277, 341, 273,
	273, 341, 269,
	269, 341, 264,
	264, 341, 328,
	328, 341, 327,
	327, 341, 325,
	325, 341, 255,
	255, 341, 251,
	251, 341, 247,
	247, 341, 243,
	243, 341, 238,
	238, 341, 322,
	322, 341, 321,
	321, 341, 319,
	319, 341, 229,
	229, 341, 225,
	225, 341, 221,
	221, 341, 217,
	217, 341, 212,
	212, 341, 316,
	316, 341, 315,
	315, 341, 313,
	313, 341, 203,
	203, 341, 199,
	199, 341, 195,
	195, 341, 191,
	191, 341, 186,
	186, 341, 340,
	340, 341, 339,
};
