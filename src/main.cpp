#include <app.hpp>
#include <build_version.hpp>
#include <klib/args/parse.hpp>
#include <klib/version_str.hpp>
#include <print>

auto main(int argc, char** argv) -> int {
	try {
		auto const version = klib::to_string(riff::build_version_v);
		auto const app_info = klib::args::ParseInfo{
			.help_text = "riff: audio player demo using capo-lite",
			.version = version,
		};
		auto const parse_result = klib::args::parse_main(app_info, {}, argc, argv);
		if (parse_result.early_return()) { return parse_result.get_return_code(); }

		riff::App{}.run();
	} catch (std::exception const& e) {
		std::println("PANIC: {}", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		std::println("PANIC!");
		return EXIT_FAILURE;
	}
}
