#pragma once
#include "mpgx/window.h"

struct Interface;

struct Interface* createInterface();

void destroyInterface(
	struct Interface* interface);

void updateInterface(
	struct Interface* interface,
	struct Window* window);

// TODO: render system
// createRenderer(struct RenderSYstem*)
// destroyRenderer(struct RenderSYstem*)

