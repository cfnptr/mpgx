#include "mpgx/interface.h"
#include "mpgx/camera.h"

#include <assert.h>

struct Interface
{
	int TODO;
};

struct Interface* createInterface()
{
	struct Interface* interface =
		malloc(sizeof(struct Interface));

	if (interface == NULL)
		return NULL;

	return interface;
}
void destroyInterface(
	struct Interface* interface)
{
	if (interface == NULL)
		return;

	free(interface);
}

void updateInterface(
	struct Interface* interface,
	struct Window* window)
{
	assert(interface != NULL);
	assert(window != NULL);

	// TODO:

	/*interface->camera = createOrthographicCamera(
		0.0f,
		1.0f,
		0.0f,
		1.0f,
		0.0f,
		1.0f);*/
}
