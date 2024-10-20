# Monadic Operations in C++17

Monadic operations implemented for C++17. 

See the accompanying tests for more examples.

```cpp
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <charconv>
#include <cmath>
#include <array>

#include "monadic_operations/monadic_operations.hpp"

std::optional<int> to_int(std::string_view sv)
{
    int r{};
    auto [ptr, ec]{std::from_chars(sv.data(), sv.data() + sv.size(), r)};
    if (ec == std::errc())
        return r;
    else
        return std::nullopt;
}

int main() {
    const std::array<std::string, 5> numbers{"4", "one", "-4", "9", "16"};

    std::cout << "square roots of even numbers:\n";
    for (const auto& number : numbers) {
        std::cout << resolve(to_int(number),
            and_then([](auto i) {
                return resolve(i >= 0 ? std::make_optional<int>(i) : std::nullopt,
                               filter([](auto i){ return i % 2 == 0; }),
                               transform([](auto i){ return std::to_string(std::sqrt(i)); }),
                               or_else([](){ return std::string{"unsupported number"}; }));
            })).value_or("invalid number") << "\n";
    }
}
```
possible output:

square roots of even numbers:

2.000000

invalid number

unsupported number

unsupported number

4.000000
