#include <gtest/gtest.h>
#include <monadic_operations.hpp>

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

TEST(MonadTests, AndThenWithNestedOptionals) {
    auto nested_opt = std::make_optional<int>(5);
    EXPECT_EQ(5, resolve(std::make_optional<int>(2), and_then([&nested_opt](int) { return nested_opt; })).value());
    EXPECT_EQ(std::nullopt, resolve(std::optional<int>{}, and_then([&nested_opt](int) { return nested_opt; })));
}

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

TEST(MonadTests, AndThenWithOptionalToOptional) {
    auto maybe_even = [](int x) { return x % 2 == 0 ? std::make_optional<int>(x) : std::nullopt; };

    auto opt_result = resolve(std::make_optional<int>(3), and_then(maybe_even));

    EXPECT_EQ(std::nullopt, opt_result);

    auto valid_result = resolve(std::make_optional<int>(4), and_then(maybe_even));

    EXPECT_EQ(4, valid_result.value());
}

struct TrackCopies {
    static int copy_count;
    static int move_count;

    TrackCopies() = default;
    
    TrackCopies(const TrackCopies&) {
        ++copy_count;
    }

    TrackCopies& operator=(const TrackCopies&) {
        ++copy_count;
        return *this;
    }

    TrackCopies(TrackCopies&&) noexcept {
        ++move_count;
    }

    TrackCopies& operator=(TrackCopies&&) noexcept {
        ++move_count;
        return *this;
    }

    static void reset_counts() {
        copy_count = 0;
        move_count = 0;
    }
};

int TrackCopies::copy_count = 0;
int TrackCopies::move_count = 0;

TEST(MonadTests, NoCopyOnTransformTest) {
    TrackCopies::reset_counts();
        
    auto result = resolve(std::make_optional<int>(2), transform([](auto) { return TrackCopies(); }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 1);
}

TEST(MonadTests, NoCopyMoveOnForwardInTransformTest) {
    TrackCopies::reset_counts();
    
    auto result = resolve(std::make_optional<TrackCopies>(), transform([](auto&& x) -> decltype(auto) { return std::forward<decltype(x)>(x); }));

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
}

TEST(MonadTests, NoCopyMoveOnAndThenTest) {
    TrackCopies::reset_counts();
    
    auto result = resolve(std::make_optional<int>(2), and_then([](auto) { return std::make_optional<TrackCopies>(); }));
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(TrackCopies::copy_count, 0);
    EXPECT_EQ(TrackCopies::move_count, 0);
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

TEST(MonadTests, NoCopyMoveOnOptionalOrElseAndTransformTest) {
    TrackCopies::reset_counts();

    auto result = resolve(std::optional<TrackCopies>{}, 
                          or_else([]() { return std::make_optional<TrackCopies>(); }),
                          transform([](auto&&){ return 1; }));
    
    EXPECT_EQ(1, result.has_value());
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
