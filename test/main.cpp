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
		std::unique_ptr<xengine::Sprite> sprite2 = app.CreateSprite("../src/textures/bubble.png");
		std::unique_ptr<xengine::Sprite> sprite3 = app.CreateSprite("../src/textures/pine.png");
		std::unique_ptr<xengine::Sprite> sprite4 = app.CreateSprite("../src/textures/enemy_ship_small_1.png");
		sprite2->SetPosition(glm::vec3(-1.5f, 0.25f, 0.0f));
		sprite3->SetPosition(glm::vec3(1.7f, 0.8f, 0.5f));
		sprite4->SetPosition(glm::vec3(1.8f, -1.0f, 0.5f));
		app.AddSprite(std::move(sprite));
		app.AddSprite(std::move(sprite2));
		app.AddSprite(std::move(sprite3));
		app.AddSprite(std::move(sprite4));
		app.Run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return 0;
}