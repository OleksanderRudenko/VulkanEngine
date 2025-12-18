#include <src/application.h>
#include <src/vulkan_engine_lib.h>
#include <stdexcept>
#include <sstream>

int main()
{
	xengine::Application app(800, 600);
	try {
		if(!app.Init())
		{
			return EXIT_FAILURE;
		}
		std::shared_ptr<xengine::Sprite> sprite = app.CreateSprite("../src/textures/test.png");
		std::shared_ptr<xengine::Sprite> sprite2 = app.CreateSprite("../src/textures/bubble.png");
		std::shared_ptr<xengine::Sprite> sprite3 = app.CreateSprite("../src/textures/pine.png");
		std::shared_ptr<xengine::Sprite> sprite4 = app.CreateSprite("../src/textures/enemy_ship_small_1.png");
		sprite2->SetPosition(glm::vec3(-1.5f, 0.25f, 0.0f));
		sprite3->SetPosition(glm::vec3(1.7f, 0.8f, 0.5f));
		sprite4->SetPosition(glm::vec3(1.8f, -1.0f, 0.5f));

		xengine::InputHandler* input = app.GetInputHandler();
		glm::vec3 position(0,0,0);
		bool showDemoWindow = true;
		while(!app.ShouldClose())
		{
			app.GLFWPollEvents();
			input->Update();

			// Start ImGui frame
			app.BeginImGuiFrame();

			// ImGui Demo Window
			if (showDemoWindow)
			{
				app.ImGuiShowDemoWindow(&showDemoWindow);
			}

			// Custom ImGui window
			app.ImGuiBeginWindow("Debug Info");

			// FPS text
			std::ostringstream fpsText;
			fpsText << "FPS: " << app.ImGuiGetFramerate();
			app.ImGuiText(fpsText.str().c_str());

			// Position text
			std::ostringstream posText;
			posText << "Sprite Position: " << position.x << ", 0.25";
			app.ImGuiText(posText.str().c_str());

			if (app.ImGuiButton("Reset Position"))
			{
				position.x = 0.0f;
				sprite2->SetPosition(glm::vec3(position.x, 0.25f, 0.0f));
			}
			app.ImGuiEndWindow();

			if(!app.DrawFrame())
			{
				std::cout << "Rendering failed\n";
				//return false;
			}
			if (input->IsKeyPressed(GLFW_KEY_W)) {
				position.x -= 0.05f;
				sprite2->SetPosition(glm::vec3(position.x, 0.25f, 0.0f));
			}
			//app.DeviceWaitIdle();
		}
		//app.Cleanup();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return 0;
}