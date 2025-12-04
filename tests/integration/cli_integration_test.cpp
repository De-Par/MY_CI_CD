#include <gtest/gtest.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

// Интеграционный тест: запускает готовый CLI и проверяет stdout.
static std::string run_cli_and_capture() {
    std::array<char, 256> buffer{};
    std::string result;

    const char *build_root_env = std::getenv("MESON_BUILD_ROOT");
    const std::filesystem::path build_root =
        build_root_env ? std::filesystem::path(build_root_env) : std::filesystem::path(".");

    std::filesystem::path cli_path = build_root / "awesome_calc_cli";
#ifdef _WIN32
    cli_path += ".exe";
#endif

    const std::string cmd = "\"" + cli_path.string() + "\"";
    struct PipeCloser {
        void operator()(FILE *f) const {
            if (f) {
                pclose(f);
            }
        }
    };

    std::unique_ptr<FILE, PipeCloser> pipe(popen(cmd.c_str(), "r"));
    if (!pipe) {
        throw std::runtime_error("failed to spawn awesome_calc_cli");
    }

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result.append(buffer.data());
    }
    return result;
}

TEST(CliIntegration, ProducesExpectedOutput) {
    const std::string out = run_cli_and_capture();
    EXPECT_NE(out.find("add(2, 3) = 5"), std::string::npos);
    EXPECT_NE(out.find("mean(1,2,3,4) = 2.5"), std::string::npos);
}
