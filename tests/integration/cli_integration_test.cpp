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

    auto get_env = [](const char *name) -> std::string {
#ifdef _WIN32
        char *buf = nullptr;
        size_t sz = 0;
        if (_dupenv_s(&buf, &sz, name) != 0 || buf == nullptr) {
            return {};
        }
        std::string val(buf);
        free(buf);
        return val;
#else
        const char *v = std::getenv(name);
        return v ? std::string(v) : std::string();
#endif
    };

    const std::string build_root_env = get_env("MESON_BUILD_ROOT");
    const std::filesystem::path build_root = !build_root_env.empty()
                                                 ? std::filesystem::path(build_root_env)
                                                 : std::filesystem::path(".");

    // Бинарь может лежать в корне build или в поддиректории app (как генерирует Meson).
    std::filesystem::path cli_path = build_root / "awesome_calc_cli";
    if (!std::filesystem::exists(cli_path)) {
        cli_path = build_root / "app" / "awesome_calc_cli";
    }
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

TEST(CliIntegration, ProducesExpectedOutput) {
    const std::string out = run_cli_and_capture();
    EXPECT_NE(out.find("add(2, 3) = 5"), std::string::npos);
    EXPECT_NE(out.find("mean(1,2,3,4) = 2.5"), std::string::npos);
    EXPECT_NE(out.find("median(1,2,3,4) = 2.5"), std::string::npos);
    EXPECT_NE(out.find("weighted_mean(1,2,3,4 | 0.1,0.2,0.3,0.4) = 3"), std::string::npos);
    EXPECT_NE(out.find("clamp_add(100, 50, -100, 120) = 120"), std::string::npos);
    EXPECT_NE(out.find("running_stats count=3"), std::string::npos);
}
