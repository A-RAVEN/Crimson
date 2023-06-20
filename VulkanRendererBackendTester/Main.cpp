#include <cstdlib>
#define NOMINMAX
#include <windows.h>
#define DEFINE_RENDERBACKEND_LOAD_FUNCTIONS
#include <CRenderBackend.h>


int main(int argc, char *argv[])
{
	graphics_backend::LoadRenderBackend();

	graphics_backend::CRenderBackend* pBackend = graphics_backend::NewRenderBackend();
	pBackend->Initialize("Test Vulkan Backend", "CRIMSON Engine");
	pBackend->NewWindow(512, 512, "Test Window");
	pBackend->NewWindow(512, 512, "Test Window 1");
	while (pBackend->AnyWindowRunning())
	{
		pBackend->TickWindows();
	}
	pBackend->Release();
	graphics_backend::DeleteRenderBackend(pBackend);

	graphics_backend::UnloadRenderBackend();
	return EXIT_SUCCESS;
}