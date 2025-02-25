#include "stdafx.h"
#include "application.h"
#include <GLFW/glfw3.h>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <iostream>

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