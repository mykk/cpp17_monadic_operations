#pragma once 

#include <optional>
#include <utility>

/*
returns function that wraps given function. given function should return a value or a reference to a value.
returned function takes optional<T>.
returned function returns value from original function wrapped into optional if input has value, nullopt otherwise.
*/
template <typename F>
auto transform(F&& f) {
    return [f = std::forward<F>(f)](auto&& x) {
        using ResultType = decltype(f(std::forward<decltype(x)>(x).value()));
        using OptionalResultType = std::conditional_t<
            std::is_reference_v<ResultType> && !std::is_rvalue_reference_v<ResultType>,
            std::reference_wrapper<std::remove_reference_t<ResultType>>,
            std::remove_reference_t<ResultType>>;

        if (x.has_value()) {
            return std::optional<OptionalResultType>{f(*std::forward<decltype(x)>(x))};
        }
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
auto and_then(F&& f) {
    return [f = std::forward<F>(f)](auto&& x) {
        using ResultType = decltype(f(std::forward<decltype(x)>(x).value()));
        using OptionalValueType = std::remove_reference_t<decltype(f(std::forward<decltype(x)>(x).value()).value())>;
        using OptionalValueResultType = std::conditional_t<
            std::is_reference_v<ResultType> && !std::is_rvalue_reference_v<ResultType>,
            std::reference_wrapper<std::remove_reference_t<OptionalValueType>>,
            OptionalValueType>;

        if (x.has_value()) {
            if constexpr (std::is_reference_v<ResultType> && !std::is_rvalue_reference_v<ResultType>) {
                if (ResultType result = f(*std::forward<decltype(x)>(x))) {
                    return std::optional<OptionalValueResultType>(*result);
                }
                return std::optional<OptionalValueResultType>{};
            } else {
                return f(*std::forward<decltype(x)>(x));
            }
        }

        return std::optional<OptionalValueResultType>{};
    };
    
}

template <typename T>
constexpr inline bool is_optional_v = false;

template <typename T>
constexpr inline bool is_optional_v<std::optional<T>> = true;

template <typename T>
using is_optional = std::bool_constant<is_optional_v<T>>;

/*
returns function that wraps given function. given function should return a value, or a reference to a value, or an optional or reference to an optional.
returned function takes optional<T>.
returned function returns optional from the original function if input has no value, nullopt otherwise.
if original function returns reference to a value, the result will be optional to reference to that value or reference to input value.
if original function returns reference to an optional, the result will be optional to reference to the value of returned optional or to reference to input value.
*/
template <typename F>
auto or_else(F&& f) -> decltype(auto)  {
    return [f = std::forward<F>(f)](auto&& x) -> decltype(auto) {
        if constexpr (is_optional_v<std::remove_reference_t<decltype(f())>>) {
            using ResultType = decltype(f());

            using OptionalValueResultType = std::conditional_t<
                std::is_reference_v<ResultType> && !std::is_rvalue_reference_v<ResultType>,
                std::reference_wrapper<std::remove_reference_t<decltype(f().value())>>,
                std::remove_reference_t<decltype(f().value())>>;
            
            if constexpr (std::is_reference_v<ResultType> && !std::is_rvalue_reference_v<ResultType>) {
                if (x.has_value()) {
                    return std::optional<OptionalValueResultType>{*x}; //it's a ref. no need too forward anything.
                }
            
                if (ResultType result = f()) {
                    return std::optional<OptionalValueResultType>{*result};
                }
                return std::optional<OptionalValueResultType>{};
            } else {
                if constexpr(std::is_rvalue_reference_v<ResultType>) {
                    if (x.has_value()) {
                        return std::forward<decltype(x)>(x);
                    }
                    return f();
                } else {
                    if (x.has_value()) {
                        return std::optional<OptionalValueResultType>{*std::forward<decltype(x)>(x)};
                    }
                    return f();
                }
            }
        } else {
            using OptionalValueType = decltype(f());
            using OptionalValueResultType = std::conditional_t<
                std::is_reference_v<OptionalValueType> && !std::is_rvalue_reference_v<OptionalValueType>,
                std::reference_wrapper<std::remove_reference_t<OptionalValueType>>,
                std::remove_reference_t<OptionalValueType>>;

            if (x.has_value()) {
                return std::optional<OptionalValueResultType>{*std::forward<decltype(x)>(x)};
            }
            return std::optional<OptionalValueResultType>{f()};
        }
    };
}

template <typename F>
auto filter(F&& f) {
    return [f = std::forward<F>(f)](auto&& x) {
        if (x.has_value()) {
            if (f(*x)) {
                return std::forward<decltype(x)>(x);
            }
        }

        return std::optional<std::remove_reference_t<decltype(std::forward<decltype(x)>(x).value())>>{}; 
    };
}

//TODO:
//combine
//flatten

template <typename T, typename Monad>
auto resolve(T&& maybe_value, Monad&& monad) -> decltype(auto) {
    return monad(std::forward<T>(maybe_value));
}

template <typename T, typename Monad, typename... Monads>
auto resolve(T&& maybe_value, Monad&& monad, Monads&&... monads) -> decltype(auto) {
    return resolve(monad(std::forward<T>(maybe_value)), std::forward<Monads>(monads)...);
}

