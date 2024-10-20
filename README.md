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

    for (const auto& number : numbers) {
        std::cout << resolve(to_int(number),
            and_then([](auto i) {
                return resolve(std::make_optional<int>(i),
                               filter([](auto i){ return i >= 0; }),
                               transform([](auto i){ return std::to_string(std::sqrt(i)); }),
                               or_else([](){ return std::string{"negative number"}; }));
            })).value_or("invalid number") << "\n";
    }
}
```
possible result:

2.000000

invalid number

negative number

3.000000

4.000000
