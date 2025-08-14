#include <build_version.hpp>
#include <klib/args/parse.hpp>
#include <widgets/app.hpp>
#include <array>
#include <print>

auto main(int argc, char** argv) -> int {
	try {
		auto params = riff::Params{};
		auto const app_info = klib::args::ParseInfo{
			.help_text = "riff: audio player demo using capo-lite",
			.version = riff::build_version_str,
		};
		auto const args = std::array{
			klib::args::named_option(params.config_path, "config", "path to riff config file"),
		};
		auto const parse_result = klib::args::parse_main(app_info, args, argc, argv);
		if (parse_result.early_return()) { return parse_result.get_return_code(); }

		auto app = riff::App{params};
		app.run();
	} catch (std::exception const& e) {
		std::println("PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println("PANIC!");
		return EXIT_FAILURE;
	}
}
