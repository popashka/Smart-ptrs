#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t

// some deleters

template <typename T>
class DefaultDeleter {
public:
    void operator()(T* p) {
        delete p;
    }
};

template <typename T>
class DefaultDeleter<T[]> {
public:
    void operator()(T* p) {
        delete[] p;
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Copy Constructors

    explicit UniquePtr(T* ptr = nullptr) : elem_(ptr, Deleter()) {
        // elem_.GetFirst() = ptr;
        // elem_.GetSecond() = DefaultDeleter<T>();
    }

    // explicit UniquePtr(T* ptr = nullptr, Deleter &deleter = DefaultDeleter<T>()) : elem_(ptr,
    // deleter){
    //}

    template <typename Up>
    UniquePtr(T* ptr, Up&& deleter) noexcept : elem_(ptr, std::forward<Up>(deleter)) {
    }

    // Move Constructors

    UniquePtr(UniquePtr<T, Deleter>& other) = delete;
    UniquePtr& operator=(UniquePtr<T, Deleter>& other) = delete;

    /*template <typename D>
    UniquePtr(UniquePtr<T, D>&& other) noexcept :
          elem_({other.elem_.GetFirst(), std::forward<D>(other.elem_.GetSecond())}){
    }*/

    template <typename U, typename D>
    UniquePtr(UniquePtr<U, D>&& other) noexcept
        : elem_({other.elem_.GetFirst(), std::forward<D>(other.elem_.GetSecond())}) {
        other.elem_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    // Move operators

    template <typename U, typename D>
    UniquePtr& operator=(UniquePtr<U, D>&& other) noexcept {
        if (elem_.GetFirst() == other.elem_.GetFirst()) {
            return *this;
        }
        T* oldptr = elem_.GetFirst();
        elem_.GetFirst() = other.elem_.GetFirst();
        other.elem_.GetFirst() = nullptr;
        if (oldptr != nullptr) {
            elem_.GetSecond()(oldptr);
        }
        elem_.GetSecond() = std::forward<Deleter>(other.elem_.GetSecond());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        T* oldptr = elem_.GetFirst();
        elem_.GetFirst() = nullptr;
        if (oldptr != nullptr) {
            elem_.GetSecond()(oldptr);
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (elem_.GetFirst() != nullptr) {
            elem_.GetSecond()(elem_.GetFirst());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* oldptr = elem_.GetFirst();
        elem_.GetFirst() = nullptr;
        return oldptr;
    }
    void Reset(T* ptr = nullptr) {
        T* oldptr = elem_.GetFirst();
        elem_.GetFirst() = ptr;
        if (oldptr != nullptr) {
            elem_.GetSecond()(oldptr);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(elem_.GetFirst(), other.elem_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        if (elem_.GetFirst() != nullptr) {
            return elem_.GetFirst();
        }
        return nullptr;
    }
    Deleter& GetDeleter() {
        return elem_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return elem_.GetSecond();
    }
    explicit operator bool() const {
        return elem_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *static_cast<T*>(elem_.GetFirst());
    }

    T* operator->() const {
        return static_cast<T*>(elem_.GetFirst());
    }

private:
    CompressedPair<T*, Deleter> elem_;

    template <typename U, typename D>
    friend class UniquePtr;
};

///////////////////////////////////////////////////
// Specialization for arrays

template <typename T, class Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Copy Constructors

    explicit UniquePtr(T* ptr = nullptr) : elem_(ptr, Deleter()) {
        // elem_.GetFirst() = ptr;
        // elem_.GetSecond() = DefaultDeleter<T>();
    }

    // explicit UniquePtr(T* ptr = nullptr, Deleter &deleter = DefaultDeleter<T>()) : elem_(ptr,
    // deleter){
    //}

    template <typename Up>
    UniquePtr(T* ptr, Up&& deleter) noexcept : elem_(ptr, std::forward<Up>(deleter)) {
    }

    // Move Constructors

    UniquePtr(UniquePtr<T, Deleter>& other) = delete;
    UniquePtr& operator=(UniquePtr<T, Deleter>& other) = delete;

    template <typename D>
    UniquePtr(UniquePtr<T, D>&& other) noexcept :
          elem_({other.elem_.GetFirst(), std::forward<D>(other.elem_.GetSecond())}){
    }

    template <typename U, typename D>
    UniquePtr(UniquePtr<U, D>&& other) noexcept
        : elem_({other.elem_.GetFirst(), std::forward<D>(other.elem_.GetSecond())}) {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    // Move operators

    template <typename U, typename D>
    UniquePtr& operator=(UniquePtr<U, D>&& other) noexcept {
        if (elem_.GetFirst() == other.elem_.GetFirst()) {
            return *this;
        }
        T* oldptr = elem_.GetFirst();
        elem_.GetFirst() = other.elem_.GetFirst();
        other.elem_.GetFirst() = nullptr;
        elem_.GetSecond()(oldptr);
        elem_.GetSecond() = std::forward<Deleter>(other.elem_.GetSecond());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        T* oldptr = elem_.GetFirst();
        elem_.GetFirst() = nullptr;
        if (oldptr != nullptr) {
            elem_.GetSecond()(oldptr);
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        elem_.GetSecond()(elem_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* oldptr = elem_.GetFirst();
        elem_.GetFirst() = nullptr;
        return oldptr;
    }
    void Reset(T* ptr = nullptr) {
        T* oldptr = elem_.GetFirst();
        elem_.GetFirst() = ptr;
        if (oldptr != nullptr) {
            elem_.GetSecond()(oldptr);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(elem_.GetFirst(), other.elem_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        if (elem_.GetFirst() != nullptr) {
            return elem_.GetFirst();
        }
        return nullptr;
    }
    Deleter& GetDeleter() {
        return elem_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return elem_.GetSecond();
    }
    explicit operator bool() const {
        return elem_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    const T operator[](size_t ind) const {
        return (elem_.GetFirst() + ind);
    }

    T& operator[](size_t ind) {
        return *static_cast<T*>(elem_.GetFirst() + ind);
    }

private:
    CompressedPair<T*, Deleter> elem_;

    template <typename U, typename D>
    friend class UniquePtr;
};
