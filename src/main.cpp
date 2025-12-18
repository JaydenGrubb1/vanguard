#include <cstdlib>
#include <print>

int main(int argc, char **argv) {
	std::println("Hello, World!");

	for (int i = 0; i < argc; i++) {
		std::println("argv[{}] = {}", i, argv[i]);
	}

	return EXIT_SUCCESS;
}
