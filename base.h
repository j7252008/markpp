#include <type_traits>
#include <variant>
#include <optional>
#include <functional>

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
// 显式推导指引（ C++20 起不需要）
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <template <class...> class Target, class T>
struct is_template_of {
    static const bool value = false;
};
template <template <class...> class Target, class... Args>
struct is_template_of<Target, Target<Args...>> {
    static const bool value = true;
};

template <typename T, typename E>
struct Result {
    std::optional<T> Ok;
    std::optional<E> Err;
};

template <typename ValueType, typename... FuncTypes>
void match(ValueType&& v, FuncTypes&&... funcs)
{
    constexpr auto optioal_match = [](auto v, auto&& f1, auto&& f2) {
        if (v)
            f1(v.value());
        else
            f2();
    };
    constexpr auto result_match = [](auto v, auto&& f1, auto&& f2) {
        if (v.Ok)
            f1(v.Ok.value());
        else
            f2(v.Err.value());
    };
    if constexpr (is_template_of<std::variant, std::decay_t<ValueType>>::value)
        std::visit(overloaded { std::forward<FuncTypes>(funcs)... }, std::forward<ValueType>(v));
    else if constexpr (is_template_of<Result, std::decay_t<ValueType>>::value)
        result_match(v, std::forward<FuncTypes>(funcs)...);
    else if constexpr (is_template_of<std::optional, std::decay_t<ValueType>>::value)
        optioal_match(v, std::forward<FuncTypes>(funcs)...);
    else
        static_assert(false, "non-support type!");
}
