#include <SDL3/SDL_main.h>
#include <windows.h>

#include <cstdlib>
#include <print>
#include <vector>

#include "app.hpp"

int main(int argc, char** argv) {
	std::vector<std::string_view> args(argv, argv + argc);

	if (AttachConsole(ATTACH_PARENT_PROCESS)) {
		std::freopen("CONOUT$", "w", stdout);
		std::freopen("CONOUT$", "w", stderr);
	}

	try {
		vg::App app(args);
		app.run();
	} catch (const std::exception& e) {
		std::println(stderr, "Unhandled exception: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println(stderr, "Unhandled unknown exception");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
