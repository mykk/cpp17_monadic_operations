#include <gtest/gtest.h>
#include <monadic_operations.hpp>
#include "track_copies.hpp"

TEST(MonadTests, SimpleTransformTest) {
    EXPECT_EQ(25, resolve(std::make_optional<int>(5), transform([](auto x){ return x * x; })).value());
    EXPECT_EQ(std::nullopt, resolve(std::optional<int>{}, transform([](auto x){ return x * x; })));

    int five = 5;
    auto ref_to_five = resolve(std::make_optional<int>(25), transform([&five](auto)-> int& { return five; })).value();
    EXPECT_EQ(5, ref_to_five);
    ref_to_five.get() = 6;
    EXPECT_EQ(6, five);
    
    EXPECT_EQ(std::nullopt, resolve(std::optional<int>{}, transform([&five](auto) -> int& { return five;})));

    int const_five = 5;
    auto ref_to_const_five = resolve(std::make_optional<int>(25), transform([&const_five](auto) -> const int& { return const_five; })).value();

    EXPECT_EQ(5, ref_to_const_five);
    EXPECT_EQ(std::nullopt, resolve(std::optional<int>{}, transform([&const_five](auto) -> const int& { return const_five; })));
}

TEST(MonadTests, TransformWithNonTrivialType) {
    struct Foo {
        int x;
        explicit Foo(int val) : x(val) {}
    };

    EXPECT_EQ(10, resolve(std::make_optional<Foo>(Foo{5}), transform([](const Foo& foo) { return foo.x * 2; })).value());

    EXPECT_EQ(std::nullopt, resolve(std::optional<Foo>{}, transform([](const Foo& foo) { return foo.x * 2; })));

    auto foo = resolve(std::make_optional<int>(30), transform([](auto x) { return Foo(x + 5); }));
    EXPECT_EQ(35, foo.value().x);

    EXPECT_EQ(std::nullopt, resolve(std::optional<int>{}, transform([](auto x) { return Foo(x + 5); })));
}

TEST(MonadTests, NestedOptionalsTransform) {
    auto nested = std::make_optional<std::optional<int>>(5);
    auto flattened = resolve(nested, transform([](auto opt) { return opt.value() * 2; }));
    EXPECT_EQ(10, flattened.value());

    auto empty_nested = resolve(std::optional<std::optional<int>>{}, transform([](auto opt) { return opt.value() * 2; }));
    EXPECT_EQ(std::nullopt, empty_nested);
}

TEST(MonadTests, TransformWithLambdaState) {
    int multiplier = 3;
    auto stateful_lambda = [&multiplier](int x) { return x * multiplier; };

    EXPECT_EQ(15, resolve(std::make_optional<int>(5), transform(stateful_lambda)).value());
    
    multiplier = 4;
    EXPECT_EQ(20, resolve(std::make_optional<int>(5), transform(stateful_lambda)).value());
}

TEST(MonadTests, NoCopyOnTransformTest) {
    TrackCopies::reset_counts();
        
    auto result = resolve(std::make_optional<int>(2), transform([](auto) { return TrackCopies(); }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 1);
}

TEST(MonadTests, NoCopyMoveOnForwardInTransformTest) {
    TrackCopies::reset_counts();
    
    auto result = resolve(std::make_optional<TrackCopies>(12), transform([](auto&& x) { return x.value; }));

    EXPECT_EQ(12, result.value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, NoCopyMoveOnLValueTransformTest) {
    TrackCopies::reset_counts();
        
    auto result = resolve(std::make_optional<TrackCopies>(), transform([](auto&&) { return 1; }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, NoCopyMoveOnMovedLValueTransformTest) {
    TrackCopies::reset_counts();
        
    std::optional<TrackCopies> track_obj = std::make_optional<TrackCopies>();
    auto result = resolve(std::move(track_obj), transform([](auto&&) { return 1; }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, NoCopyOnReturnRefInTransformTest) {
    TrackCopies::reset_counts();

    std::optional<TrackCopies> track_obj = std::make_optional<TrackCopies>();
    
    auto result = resolve(track_obj, transform([](const auto& x) -> const auto& { return x; }), transform([](const auto&) { return 1; }));

    EXPECT_EQ(1, result.value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

