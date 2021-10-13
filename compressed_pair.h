#pragma once

#include <type_traits>
#include <cstddef>
#include <iostream>

template <typename T, size_t I, bool EnableEBO = std::is_empty_v<T> && !std::is_final_v<T>>
struct CompressedPairElement {
    T value_ = T();

    CompressedPairElement() {
    }

    CompressedPairElement(T& value) : value_(std::forward<T>(value)){
    }

    template<typename Tp>
    CompressedPairElement(Tp&& value) : value_(std::forward<Tp>(value)){
    }

    const T& Get() const {
        return value_;
    }

    T& Get() {
        return value_;
    }
};

template <typename T, size_t I>
struct CompressedPairElement<T, I, true> : public T {
    CompressedPairElement() = default;

    template<typename Tp>
    CompressedPairElement(Tp&&) {
    }

    const T& Get() const {
        return *this;
    }

    T& Get() {
        return *this;
    }
    // getters return *this
};

// Me think, why waste time write lot code, when few code do trick.
template <typename F, typename S>
class CompressedPair : private CompressedPairElement<F, 0>, private CompressedPairElement<S, 1> {
    using First = CompressedPairElement<F, 0>;
    using Second = CompressedPairElement<S, 1>;

public:
    CompressedPair() = default;

    template <typename Sp>
    CompressedPair(F& first, Sp&& second) : First(first), Second(std::forward<Sp>(second)) {
    }

    template <typename Sp>
    CompressedPair(F&& first, Sp&& second) : First(first), Second(std::forward<Sp>(second)) {
    }

    F& GetFirst() {
        return First::Get();
    }

    const F& GetFirst() const {
        return First::Get();
    }

    S& GetSecond() {
        return Second::Get();
    }

    const S& GetSecond() const {
        return Second::Get();
    };
};