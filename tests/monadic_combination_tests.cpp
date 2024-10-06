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

TEST(MonadTests, TransformOrElseCombinationOnReference) {
    TrackCopies::reset_counts();

    auto track_obj1 = std::make_optional<TrackCopies>(1);
    auto track_obj2 = TrackCopies(2);

    auto result = resolve(track_obj1,
                          or_else([&track_obj2]() -> auto& { return track_obj2; }), 
                          transform([](auto&& x) -> auto& { return x.get().value; }));

    EXPECT_EQ(1, result.value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);

    track_obj1.value().value = 5;
    EXPECT_EQ(5, result.value());

    track_obj1 = std::optional<TrackCopies>{};
    auto result_from_else = resolve(track_obj1,
                                    or_else([&track_obj2]() -> auto& { return track_obj2; }), 
                                    transform([](auto&& x) -> auto& { return x.get().value; }));

    EXPECT_EQ(2, result_from_else.value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);

    track_obj2.value = 12;;
    EXPECT_EQ(12, result_from_else.value());
}

TEST(MonadTests, AndThenOrElseCombinationOnReference) {
    TrackCopies::reset_counts();

    auto track_obj1 = std::make_optional<TrackCopies>(1);
    auto track_obj2 = TrackCopies(2);

    auto result = resolve(track_obj1,
                          or_else([&track_obj2]() -> auto& { return track_obj2; }), 
                          and_then([](auto&& x) { return std::make_optional<std::reference_wrapper<const int>>(x.get().value); }));

    EXPECT_EQ(1, result.value().get());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);

    track_obj1.value().value = 5;
    EXPECT_EQ(5, result.value().get());

    track_obj1 = std::optional<TrackCopies>{};
    auto result_from_else = resolve(track_obj1,
                                    or_else([&track_obj2]() -> auto& { return track_obj2; }), 
                                    and_then([](auto&& x) { return std::make_optional<std::reference_wrapper<int>>(x.get().value); }));

    EXPECT_EQ(2, result_from_else.value().get());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);

    result_from_else.value().get() = 12;;
    EXPECT_EQ(12, track_obj2.value);
}