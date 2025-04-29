#include <app.hpp>
#include <print>

auto main(int argc, char** argv) -> int {
	try {
		riff::App{}.run(argc, argv);
	} catch (std::exception const& e) {
		std::println("PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println("PANIC!");
		return EXIT_FAILURE;
	}
}
