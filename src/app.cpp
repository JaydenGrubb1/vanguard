#include <print>
#include <ranges>

#include "app.hpp"

namespace vg {

App::App(std::span<const std::string_view> args) {
	for (auto [idx, arg] : std::views::enumerate(args)) {
		std::println("arg[{}] = {}", idx, arg);
	}
}

App::~App() {
	// TODO
}

void App::run() {
	m_running = true;
	// TODO
}

void App::quit() {
	m_running = false;
}

} // namespace vg
