#include <src/application.h>
#include <src/vulkan_engine_lib.h>
#include <stdexcept>

int main()
{
	xengine::Application app(800, 600);
	try {
		if(!app.Init())
		{
			return EXIT_FAILURE;
		}
		std::unique_ptr<xengine::Sprite> sprite = app.CreateSprite("../src/textures/test.png");
		app.AddSprite(std::move(sprite));
		app.Run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return 0;
}