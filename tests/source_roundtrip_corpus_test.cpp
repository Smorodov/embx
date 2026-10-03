#include <catch2/catch_test_macros.hpp>
#include "codegen/SourceGenerator.h"
#include "parser/ParserDriver.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

std::vector<std::string> collectCorpus() {
    namespace fs = std::filesystem;
    std::vector<std::string> files;
    const fs::path roots[] = {"../examples", "../course"};
    for (const auto& root : roots) {
        if (!fs::exists(root)) continue;
        for (const auto& entry : fs::recursive_directory_iterator(root)) {
            if (entry.is_regular_file() && entry.path().extension() == ".embx")
                files.push_back(entry.path().generic_string());
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

std::unique_ptr<embx::ast::Module> parse(const std::string& path) {
    std::string error;
    auto module = embx::parser::parseFile(path, error);
    REQUIRE(module);
    REQUIRE(error.empty());
    return module;
}

std::string generate(const embx::ast::Module& module, const std::string& path) {
    std::string source;
    std::string error;
    REQUIRE(embx::codegen::generateSource(module, source, error));
    REQUIRE(error.empty());
    REQUIRE_FALSE(source.empty());
    INFO("source corpus entry: " << path);
    return source;
}

} // namespace

TEST_CASE("source generator covers the complete source corpus") {
    const auto files = collectCorpus();
    REQUIRE_FALSE(files.empty());

    for (const auto& path : files) {
        DYNAMIC_SECTION(path) {
            auto first = parse(path);
            const auto canonical1 = generate(*first, path);

            const auto temp = std::filesystem::path(path).string() + ".corpus.generated.embx";
            {
                std::ofstream out(temp, std::ios::binary);
                REQUIRE(out);
                out << canonical1;
            }

            auto second = parse(temp);
            const auto canonical2 = generate(*second, temp);
            std::error_code ignored;
            std::filesystem::remove(temp, ignored);

            REQUIRE(canonical2 == canonical1);
        }
    }
}
