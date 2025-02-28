#include <src/application.h>
#include <src/vulkan_engine_lib.h>
#include <stdexcept>

int main()
{
	eApplication app;
	try {
		app.Run(800, 600);
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return 0;
}