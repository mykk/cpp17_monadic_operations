#include <gtest/gtest.h>
#include <monadic_operations.hpp>
#include "track_copies.hpp"

TEST(MonadTests, SimpleFilterTest) {
    EXPECT_EQ(25, resolve(std::make_optional<int>(25), filter([](auto x){ return x > 5; })).value());
    EXPECT_EQ(std::nullopt, resolve(std::make_optional<int>(25), filter([](auto x){ return x < 5; })));
    EXPECT_EQ(std::nullopt, resolve(std::optional<int>{}, filter([](auto x){ return x > 5; })));
}

TEST(MonadTests, FilterStringTest) {
    EXPECT_EQ(std::nullopt, resolve(std::make_optional<std::string>("hello"), filter([](const auto& x){ return x.size() > 10; })));
    EXPECT_EQ("world", resolve(std::make_optional<std::string>("world"), filter([](const auto& x){ return x.size() == 5; })).value());
}

TEST(MonadTests, FilterSubstringTest) {
    EXPECT_EQ("test", resolve(std::make_optional<std::string>("test"), filter([](const auto& x){ return x.find("es") != std::string::npos; })).value());
    EXPECT_EQ(std::nullopt, resolve(std::make_optional<std::string>("filter"), filter([](const auto& x){ return x.find("abc") != std::string::npos; })));
}

TEST(MonadTests, FilterNonCopyableTypeTest) {
    auto unique_ptr_opt = std::make_optional<std::unique_ptr<int>>(std::make_unique<int>(10));
    auto result = resolve(std::move(unique_ptr_opt), filter([](const std::unique_ptr<int>& p){ return *p > 5; }));

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(10, *result.value());

    auto result2 = resolve(std::move(result), filter([](const std::unique_ptr<int>& p){ return *p < 5; }));
    EXPECT_EQ(std::nullopt, result2);
}

TEST(MonadTests, FilterCustomStructTest) {
    struct Foo {
        int a;
        int b;
    };

    auto obj = std::make_optional<Foo>({10, 20});
    auto result = resolve(obj, filter([](const Foo& s) { return s.a + s.b > 25; }));

    EXPECT_EQ(10, result.value().a);
    EXPECT_EQ(20, result.value().b);

    auto result2 = resolve(obj, filter([](const Foo& s) { return s.a + s.b < 15; }));
    EXPECT_EQ(std::nullopt, result2);
}

TEST(MonadTests, FilterNestedOptionalTest) {
    auto nested_optional = std::make_optional(std::make_optional<int>(42));
    auto result = resolve(nested_optional, filter([](const auto& inner_opt){ return inner_opt.has_value() && inner_opt.value() > 40; }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(42, result.value().value());

    auto result2 = resolve(nested_optional, filter([](const auto& inner_opt){ return inner_opt.has_value() && inner_opt.value() < 10; }));
    EXPECT_EQ(std::nullopt, result2);
}

TEST(MonadTests, FilterMultipleConditionsTest) {
    auto track_obj = std::make_optional<int>(50);
    auto result = resolve(track_obj, filter([](int x) { return x > 5 && x < 100; }));

    EXPECT_EQ(50, result.value()); 

    auto result2 = resolve(track_obj, filter([](int x) { return x < 5 || x > 100; }));
    EXPECT_EQ(std::nullopt, result2);
}

TEST(MonadTests, FilterLValTest) {
    TrackCopies::reset_counts();
    
    auto track_obj = std::make_optional<TrackCopies>(25);
    auto result = resolve(track_obj, filter([](const auto& x) { return x.value > 5; }));

    EXPECT_EQ(25, result.value().value);
    EXPECT_EQ(TrackCopies::copy_count, 1);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, FilterMovedLValTest) {
    TrackCopies::reset_counts();
    
    auto track_obj = std::make_optional<TrackCopies>(25);
    auto result = resolve(std::move(track_obj), filter([](const auto& x) { return x.value > 5; }));

    EXPECT_EQ(25, result.value().value);
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 1);
}

TEST(MonadTests, FilterRValueTest) {
    TrackCopies::reset_counts();
    
    auto result = resolve(std::make_optional<TrackCopies>(25), filter([](const auto& x) { return x.value > 5; }));

    EXPECT_EQ(25, result.value().value);
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 1);
}

TEST(MonadTests, FilterLValReturnNullTest) {
    TrackCopies::reset_counts();
    
    auto track_obj = std::make_optional<TrackCopies>(25);
    auto result = resolve(track_obj, filter([](const auto& x) { return x.value < 5; }));

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, FilterMovedLValReturnNullTest) {
    TrackCopies::reset_counts();
    
    auto track_obj = std::make_optional<TrackCopies>(25);
    auto result = resolve(std::move(track_obj), filter([](const auto& x) { return x.value < 5; }));

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, FilterRValueReturnNullTest) {
    TrackCopies::reset_counts();
    
    auto result = resolve(std::make_optional<TrackCopies>(25), filter([](const auto& x) { return x.value < 5; }));

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, FilterConstLValTest) {
    TrackCopies::reset_counts();
    
    const auto track_obj = std::make_optional<const TrackCopies>(25);
    auto result = resolve(track_obj, filter([](const auto& x) { return x.value > 5; }));

    EXPECT_EQ(25, result.value().value);
    EXPECT_EQ(TrackCopies::copy_count, 1);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, FilterConstLValReturnNullTest) {
    TrackCopies::reset_counts();
    
    const auto track_obj = std::make_optional<const TrackCopies>(25);
    auto result = resolve(track_obj, filter([](const auto& x) { return x.value < 5; }));

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}
