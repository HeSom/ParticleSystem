#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include "app.h"

#include <iostream>

int main(int argc, char** argv)
{	
	App* app = App::create(argc, argv);
	if (app == nullptr) {
		std::cout << "Could not create App!";
		return -1;
	}
	if (!app->exec()) {
		App::free(app);
		return -1;
	}
	App::free(app);
	return 0;
}

