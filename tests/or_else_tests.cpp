#include <gtest/gtest.h>
#include <monadic_operations.hpp>
#include "track_copies.hpp"

TEST(MonadTests, ReturnValueOrElseTest) {
    EXPECT_EQ(25, resolve(std::optional<int>{}, or_else([](){ return 25; })).value());
    EXPECT_EQ(5, resolve(std::make_optional<int>(5), or_else([](){ return 25; })).value());

    int five = 5;
    EXPECT_EQ(5, resolve(std::optional<std::reference_wrapper<int>>{}, or_else([&five]() -> int& { return five; })).value());

    int twenty_five = 25;
    EXPECT_EQ(25, resolve(std::optional<std::reference_wrapper<int>>{twenty_five}, or_else([&five]() -> int& { return five; })).value());
}

TEST(MonadTests, ReturnMaybeValueOrElseTest) {
    EXPECT_EQ(25, resolve(std::optional<int>{}, or_else([](){ return std::optional<int>(25); })).value());

    EXPECT_EQ(5, resolve(std::make_optional<int>(5), or_else([](){ return std::make_optional<int>(25); })).value());
    EXPECT_EQ(std::nullopt, resolve(std::optional<int>{}, or_else([](){ return std::optional<int>{}; })));

    auto maybe_five = std::make_optional<int>(5);
    auto resolved = resolve(maybe_five, or_else([&maybe_five]() -> auto& { return maybe_five; }));
    EXPECT_EQ(5, resolved.value());
    resolved.value().get() = 6;
    EXPECT_EQ(6, maybe_five.value());

    maybe_five = std::make_optional<int>(5);
    auto nothing = std::optional<int>{};
    auto resolved_from_nothing = resolve(nothing, or_else([&maybe_five]() -> auto& { return maybe_five; }));
    EXPECT_EQ(5, resolved_from_nothing.value());
    resolved_from_nothing.value().get() = 6;
    EXPECT_EQ(6, maybe_five.value());

    EXPECT_EQ(std::optional<std::reference_wrapper<int>>{}, resolve(nothing, or_else([&nothing]() -> auto& { return nothing; })));
}

TEST(MonadTests, OrElseWithComplexTypes) {
    std::vector<int> default_vector{1, 2, 3};
    EXPECT_EQ(default_vector, resolve(std::optional<std::vector<int>>{}, or_else([&default_vector]() { return default_vector; })).value());

    std::vector<int> input_vector{4, 5, 6};
    EXPECT_EQ(input_vector, resolve(std::make_optional<std::vector<int>>(input_vector), or_else([&default_vector]() { return default_vector; })).value());
}

TEST(MonadTests, OrElseLambdaWithState) {
    int fallback_value = 50;
    auto fallback_lambda = [&fallback_value]() { return fallback_value; };

    EXPECT_EQ(50, resolve(std::optional<int>{}, or_else(fallback_lambda)).value());

    fallback_value = 100;
    EXPECT_EQ(100, resolve(std::optional<int>{}, or_else(fallback_lambda)).value());
}

TEST(MonadTests, TransformAndOrElseCombination) {
    auto combined = resolve(std::optional<int>{}, 
                            or_else([]() { return 10; }), 
                            transform([](int x) { return x * 2; }));

    EXPECT_EQ(20, combined.value());
}


TEST(MonadTests, NoCopyOnOrElseTest) {
    TrackCopies::reset_counts();

    auto result = resolve(std::optional<TrackCopies>{}, or_else([]() { return TrackCopies{}; }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 1);
}

TEST(MonadTests, NoCopyMoveOnOptionalOrElseTest) {
    TrackCopies::reset_counts();

    auto result = resolve(std::optional<TrackCopies>{}, or_else([]() { return std::make_optional<TrackCopies>(); }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, MoveRValOnOrElseReturnsOptionalTest) {
    TrackCopies::reset_counts();

    auto result = resolve(std::make_optional<TrackCopies>(), 
        or_else([]() { return std::make_optional<TrackCopies>(); }),
        transform([](auto&&){ return 1; }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 1);
}

TEST(MonadTests, MoveRValOnOrElseReturnsValueTest) {
    TrackCopies::reset_counts();

    auto result = resolve(std::make_optional<TrackCopies>(),
        or_else([]() { return TrackCopies(); }),
        transform([](auto&&){ return 1; }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 1);
}

TEST(MonadTests, CopyLValOnOrElseReturnsOptionalTest) {
    TrackCopies::reset_counts();

    auto track_obj = std::make_optional<TrackCopies>();
    auto result = resolve(track_obj,
        or_else([]() { return std::make_optional<TrackCopies>(); }),
        transform([](auto&&){ return 1; }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 1);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, CopyLValOnOrElseReturnsValueTest) {
    TrackCopies::reset_counts();

    auto track_obj = std::make_optional<TrackCopies>();
    auto result = resolve(track_obj,
        or_else([]() { return TrackCopies(); }),
        transform([](auto&&){ return 1; }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 1);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, MoveMovedLValOnOrElseReturnsOptionalTest) {
    TrackCopies::reset_counts();

    auto track_obj = std::make_optional<TrackCopies>();
    auto result = resolve(std::move(track_obj),
        or_else([]() { return std::make_optional<TrackCopies>(); }),
        transform([](auto&&){ return 1; }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 1);
}

TEST(MonadTests, MoveMovedLValOnOrElseReturnsValueTest) {
    TrackCopies::reset_counts();

    auto track_obj = std::make_optional<TrackCopies>();
    auto result = resolve(std::move(track_obj),
        or_else([]() { return TrackCopies(); }),
        transform([](auto&&){ return 1; }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 1);
}

TEST(MonadTests, NoCopyMoveOnOptionalOrElseAndTransformTest) {
    TrackCopies::reset_counts();

    auto result = resolve(std::optional<TrackCopies>{}, 
                          or_else([]() { return std::make_optional<TrackCopies>(); }),
                          transform([](auto&&){ return 1; }));
    
    EXPECT_EQ(1, result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}
