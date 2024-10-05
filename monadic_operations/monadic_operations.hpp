#pragma once 

#include <optional>
#include <utility>

/*
returns function that wraps given function. given function should return a value or a reference to a value.
returned function takes optional<T>.
returned function returns value from original function wrapped into optional if input has value, nullopt otherwise.
*/
template <typename F>
auto transform(F&& f) -> decltype(auto) {
    return [f = std::forward<F>(f)](auto&& x) -> decltype(auto) {
        using ResultType = decltype(f(std::forward<decltype(x.value())>(x.value())));
        using OptionalResultType = std::conditional_t<
            std::is_reference_v<ResultType>,
            std::reference_wrapper<std::remove_reference_t<ResultType>>,
            ResultType>;

        if (x.has_value())
            return std::optional<OptionalResultType>{f(std::forward<decltype(x.value())>(x.value()))};
        return std::optional<OptionalResultType>{};
    };
}

/*
returns function that wraps given function. given function should return optional or reference to an optional.
returned function takes optional<T>
returned function returns optional from the original function if input has value, nullopt otherwise. 
if original function returns reference to an optional, the result will be optional to reference to the value of returned optional.
*/
template <typename F>
auto and_then(F&& f) -> decltype(auto) {    
    return [f = std::forward<F>(f)](auto&& x) -> decltype(auto) {
        using ResultType = decltype(f(std::forward<decltype(x.value())>(x.value())));
        using OptionalValueType = std::remove_reference_t<decltype(f(std::forward<decltype(x.value())>(x.value())).value())>;

        using OptionalValueResultType = std::conditional_t<
            std::is_reference_v<ResultType>,
            std::reference_wrapper<std::remove_reference_t<OptionalValueType>>,
            OptionalValueType>;

        if (x.has_value()) {
            if constexpr (std::is_reference_v<ResultType>) {
                if (ResultType result = f(std::forward<decltype(x.value())>(x.value())))
                    return std::optional<OptionalValueResultType>(result.value());
            } else {
                return f(std::forward<decltype(x.value())>(x.value()));
            }
        }

        return std::optional<OptionalValueResultType>{};
    };
}

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template <typename T>
constexpr bool is_optional_v = is_optional<T>::value;

/*
returns function that wraps given function. given function should return a value, or a reference to a value, or an optional or reference to an optional.
returned function takes optional<T>.
returned function returns optional from the original function if input has no value, nullopt otherwise.
if original function returns reference to a value, the result will be optional to reference to that value or reference to input value.
if original function returns reference to an optional, the result will be optional to reference to the value of returned optional or to reference to input value.
*/
template <typename F>
auto or_else(F&& f) -> decltype(auto) {
    return [f = std::forward<F>(f)](auto&& x) -> decltype(auto) {
        if constexpr (is_optional_v<std::remove_reference_t<decltype(f())>>) {
            using ResultType = decltype(f());
            using OptionalValueResultType = std::conditional_t<
                std::is_reference_v<ResultType>,
                std::reference_wrapper<std::remove_reference_t<decltype(f().value())>>,
                std::remove_reference_t<decltype(f().value())>>;
            
            if constexpr (std::is_reference_v<ResultType>) {
                if (x.has_value())
                    return std::optional<OptionalValueResultType>{x.value()};
            
                if (ResultType result = f())
                    return std::optional<OptionalValueResultType>{result.value()};
                else 
                    return std::optional<OptionalValueResultType>{};
            } else {
                if (x.has_value())
                    return std::optional<OptionalValueResultType>{std::forward<decltype(x.value())>(x.value())};
                return f();
            }
        } else {
            using OptionalValueType = decltype(f());
            using OptionalValueResultType = std::conditional_t<
                std::is_reference_v<OptionalValueType>,
                std::reference_wrapper<std::remove_reference_t<OptionalValueType>>,
                OptionalValueType>;

            if (x.has_value())
                return std::optional<OptionalValueResultType>{std::forward<decltype(x)>(x).value()};
            return std::optional<OptionalValueResultType>{f()};
        }
    };
}

//TODO:
//filter
//combine

template <typename T, typename Monad>
auto resolve(T&& maybe_value, Monad&& monad) -> decltype(auto) {
    return monad(std::forward<T>(maybe_value));
}

template <typename T, typename Monad, typename... Monads>
auto resolve(T&& maybe_value, Monad&& monad, Monads&&... monads) -> decltype(auto) {
    return resolve(monad(std::forward<T>(maybe_value)), std::forward<Monads>(monads)...);
}

