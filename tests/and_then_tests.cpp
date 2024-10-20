#include <gtest/gtest.h>
#include <monadic_operations.hpp>
#include "track_copies.hpp"

TEST(MonadTests, SimpleAndThenTest) {
    EXPECT_EQ(25, resolve(std::make_optional<int>(5), and_then([](auto x){ return std::make_optional<int>(x * x); })).value());
    EXPECT_EQ(std::nullopt, resolve(std::optional<int>{}, and_then([](auto x){ return std::make_optional<int>(x * x); })));

    auto maybe_five = std::make_optional<int>(5);
    auto ref_to_maybe_five = resolve(std::make_optional<int>(25), and_then([&maybe_five](auto) -> auto& { return maybe_five; })).value();
    EXPECT_EQ(5, ref_to_maybe_five);
    ref_to_maybe_five.get() = 6;
    EXPECT_EQ(6, maybe_five.value());

    EXPECT_EQ(std::nullopt, resolve(std::optional<int>{}, and_then([&maybe_five](auto) -> auto& { return maybe_five; })));

    const auto const_maybe_five = std::make_optional<int>(5);
    EXPECT_EQ(5, resolve(std::make_optional<int>(25), and_then([&const_maybe_five](auto) -> const auto& { return const_maybe_five; })).value());
    EXPECT_EQ(std::nullopt, resolve(std::optional<int>{}, and_then([&const_maybe_five](auto) -> const auto& { return const_maybe_five; })));
}

TEST(MonadTests, AndThenWithNestedOptionals) {
    auto nested_opt = std::make_optional<int>(5);
    EXPECT_EQ(5, resolve(std::make_optional<int>(2), and_then([&nested_opt](int) { return nested_opt; })).value());
    EXPECT_EQ(std::nullopt, resolve(std::optional<int>{}, and_then([&nested_opt](int) { return nested_opt; })));
}

TEST(MonadTests, AndThenWithNonTrivialType) {
    struct Foo {
        int x;
        explicit Foo(int val) : x(val) {}
    };

    EXPECT_EQ(10, resolve(std::make_optional<Foo>(Foo{5}), and_then([](const Foo& foo) { return std::make_optional<>(foo.x * 2); })).value());
    EXPECT_EQ(std::nullopt, resolve(std::optional<Foo>{}, and_then([](const Foo& foo) { return std::make_optional<>(foo.x * 2); })));

    auto foo = resolve(std::make_optional<int>(30), and_then([](auto x) { return std::make_optional<Foo>(x + 5); }));
    EXPECT_EQ(35, foo.value().x);
    EXPECT_EQ(std::nullopt, resolve(std::optional<int>{}, and_then([](auto x) { return std::make_optional<Foo>(x + 5); })));

    EXPECT_EQ(std::nullopt, resolve(std::optional<int>{1}, and_then([](auto x) { return std::optional<Foo>{}; })));
}

TEST(MonadTests, AndThenWithOptionalToOptional) {
    auto even_or_nothing = [](int x) { return x % 2 == 0 ? std::make_optional<int>(x) : std::nullopt; };

    auto opt_result = resolve(std::make_optional<int>(3), and_then(even_or_nothing));

    EXPECT_EQ(std::nullopt, opt_result);

    auto valid_result = resolve(std::make_optional<int>(4), and_then(even_or_nothing));

    EXPECT_EQ(4, valid_result.value());
}

TEST(MonadTests, NoCopyMoveOnAndThenTest) {
    TrackCopies::reset_counts();
    
    auto result = resolve(std::make_optional<int>(2), and_then([](auto x) { return std::make_optional<TrackCopies>(x); }));
    
    EXPECT_EQ(2, result.value().value);
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, NoCopyMoveOnAndThenTest2) {
    TrackCopies::reset_counts();
    
    auto result = resolve(std::make_optional<TrackCopies>(5), and_then([](auto&& x) { return std::make_optional<>(x.value); }));
    
    EXPECT_EQ(5, result.value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, NoCopyMoveOnLValueAndThenTest) {
    TrackCopies::reset_counts();
    
    auto track_obj = std::make_optional<TrackCopies>(5);
    auto result = resolve(track_obj, and_then([](auto&& x) { return std::make_optional<>(x.value); }));
    
    EXPECT_EQ(5, result.value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, NoCopyMoveOnLValueMovedAndThenTest) {
    TrackCopies::reset_counts();
    
    auto track_obj = std::make_optional<TrackCopies>(5);
    auto result = resolve(std::move(track_obj), and_then([](auto&& x) { return std::make_optional<>(x.value); }));
    
    EXPECT_EQ(5, result.value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, AndThenReturnRefTest) {
    TrackCopies::reset_counts();
    
    auto track_obj1 = std::make_optional<TrackCopies>(5);
    auto track_obj2 = std::make_optional<TrackCopies>(10);

    auto result = resolve(track_obj1, and_then([&track_obj2](auto&& x) -> auto& { return track_obj2; }));
    
    EXPECT_EQ(10, result.value().get().value);
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);

    track_obj2.value().value = 3;
    EXPECT_EQ(3, result.value().get().value);
}

TEST(MonadTests, AndThenReturnCopyTest) {
    TrackCopies::reset_counts();
    
    auto track_obj = std::make_optional<TrackCopies>(5);
    auto result = resolve(track_obj, and_then([](auto&& x) { return std::make_optional<TrackCopies>(std::forward<decltype(x)>(x)); }));
    
    EXPECT_EQ(5, result.value().value);
    EXPECT_EQ(TrackCopies::copy_count, 1);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, AndThenReturnMovedTest) {
    TrackCopies::reset_counts();
    
    auto track_obj = std::make_optional<TrackCopies>(5);
    auto result = resolve(std::move(track_obj), and_then([](auto&& x) { return std::make_optional<TrackCopies>(std::forward<decltype(x)>(x)); }));

    EXPECT_EQ(5, result.value().value);
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 1);
}

TEST(MonadTests, AndThenReturnRValRefTest) {
    TrackCopies::reset_counts();
    
    auto track_obj = std::make_optional<TrackCopies>(5);
    auto track_obj2 = std::make_optional<TrackCopies>(9);

    auto result = resolve(std::move(track_obj),
                          and_then([&track_obj2](auto&& x) -> decltype(auto) {
                            track_obj2.value().value = x.value + 1;
                            return std::move(track_obj2);
                          }),
                          and_then([](auto&& x) { return std::make_optional<>(x.value); }));

    EXPECT_EQ(6, result.value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 1);
}