#include <gtest/gtest.h>
#include <monadic_operations.hpp>
#include "track_copies.hpp"

TEST(MonadTests, TransformAndThenCombination) {
    auto combined = resolve(std::make_optional<int>(3), 
                            transform([](int x) { return x + 1; }), 
                            and_then([](int x) { return std::make_optional<int>(x * 2); }));
    EXPECT_EQ(8, combined.value());

    combined = resolve(std::make_optional<int>(3), 
                            transform([](int x) { return x + 1; }), 
                            and_then([](int x) { return x > 10 ? std::make_optional<int>(x * 2) : std::nullopt; }));
    EXPECT_EQ(std::nullopt, combined);
}

TEST(MonadTests, MultipleResolveCalls) {
    auto combined = resolve(std::make_optional<int>(5), 
                            transform([](int x) { return x * 2; }), 
                            and_then([](int x) { return std::make_optional<int>(x + 3); }),
                            or_else([]() { return 0; }));

    EXPECT_EQ(13, combined.value());

    combined = resolve(std::make_optional<int>(5),
                       transform([](int x) { return x * 2; }), 
                       and_then([](int x) { return x < 2 ? std::make_optional<int>(x + 3) : std::nullopt; }),
                       or_else([]() { return 0; }));

    EXPECT_EQ(0, combined.value());
}