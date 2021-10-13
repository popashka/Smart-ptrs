#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <iostream>
#include <cassert>
#include <type_traits>

template <typename U>
class ControlBlockHolder : public ControlBlockBase {
public:
    template <typename... Args>
    ControlBlockHolder(Args&&... args) noexcept {
        new (&storage_) U(std::forward<Args>(args)...);
    }

    U* GetRawPointer() {
        return reinterpret_cast<U*>(&storage_);
    }

    ~ControlBlockHolder() {
        reinterpret_cast<U*>(&storage_)->~U();
    }

    void ResetPointer() override {
        reinterpret_cast<U*>(&storage_)->~U();
    }

    std::aligned_storage_t<sizeof(U), alignof(U)> storage_;
};

template <typename U>
class EnableSharedFromThisBase {};

template <typename T>
class EnableSharedFromThis;

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() {
    }

    SharedPtr(std::nullptr_t) {
    }

    template <typename U>
    SharedPtr(U* ptr) : ptr_(ptr) {
        if constexpr (std::is_convertible_v<U*, EnableSharedFromThisBase<U>*>) {
            EnableSharedFromThisConstruct(ptr_);
        } else {
            block_ = new ControlBlockPointer<U>(ptr);
            block_->ref_counter_ = 1;
        }
    }

    SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++block_->ref_counter_;
        }
    }
    SharedPtr(SharedPtr&& other) : ptr_(std::forward<T*>(other.ptr_)), block_(other.block_) {
        if (block_) {
            ++block_->ref_counter_;
            other.Reset();
        }
    }

    SharedPtr(T* ptr, ControlBlockBase* block) : ptr_(ptr), block_(block) {
        if (block_) {
            ++block_->ref_counter_;
        }
    }

    template <typename U>
    SharedPtr(const SharedPtr<U>& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++block_->ref_counter_;
        }
    }
    template <typename U>
    SharedPtr(SharedPtr<U>&& other) : ptr_(std::forward<U*>(other.ptr_)), block_(other.block_) {
        if (block_) {
            ++block_->ref_counter_;
            other.Reset();
        }
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr

    template <typename U>
    SharedPtr(const SharedPtr<U>& other, T* ptr) : ptr_(ptr), block_(other.block_) {
        ++block_->ref_counter_;
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.block_->ref_counter_ == 0) {
            throw BadWeakPtr();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        ++block_->ref_counter_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }
        Reset();
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (other.block_) {
            ++block_->ref_counter_;
        }
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) {
        Reset();
        ptr_ = std::forward<T*>(other.ptr_);
        block_ = other.block_;
        if (other.block_) {
            ++block_->ref_counter_;
        }
        other.Reset();
        return *this;
    }

    template <typename U>
    SharedPtr& operator=(const SharedPtr<U>& other) {
        Reset();
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (other.block_) {
            ++block_->ref_counter_;
        }
        return *this;
    }

    template <typename U>
    SharedPtr& operator=(SharedPtr<U>&& other) {
        Reset();
        ptr_ = std::forward<U*>(other.ptr_);
        block_ = other.block_;
        if (other.block_) {
            ++block_->ref_counter_;
        }
        other.Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() noexcept {
        if (block_) {
            if (block_->ref_counter_ + block_->weak_counter_ == 1) {
                delete block_;
                block_ = nullptr;
            } else {
                --block_->ref_counter_;
            }
        }
        if (ptr_ && block_ && block_->ref_counter_ == 0 && block_->weak_counter_ > 0) {
            block_->ResetPointer();
        }
        ptr_ = nullptr;
        block_ = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_) {
            if (block_->ref_counter_ + block_->weak_counter_ == 1) {
                // block_->~ControlBlockBase();
                delete block_;
            } else {
                --block_->ref_counter_;
            }
        }
        ptr_ = nullptr;
        block_ = nullptr;
    }

    template <typename U>
    void Reset(U* ptr) {
        Reset();
        ptr_ = ptr;
        if constexpr (std::is_convertible_v<U*, EnableSharedFromThisBase<U>*>) {
            EnableSharedFromThisConstruct(std::forward<U*>(ptr));
        } else {
            block_ = new ControlBlockPointer<U>(ptr);
            block_->ref_counter_ = 1;
        }
    }

    void Swap(SharedPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (!block_) {
            return 0;
        }
        return block_->ref_counter_;
    }
    explicit operator bool() const {
        if (!ptr_) {
            return false;
        }
        return (*ptr_);
    }

private:
    template <typename U>
    void EnableSharedFromThisConstruct(EnableSharedFromThis<U>* ptr) {
        if (!ptr_->self_.ptr_) {
            block_ = new ControlBlockPointer<U>(ptr_);
            block_->ref_counter_ = 1;
            ptr->self_ = *this;
            return;
        }
        block_ = ptr->self_.block_;
        ++block_->ref_counter_;
    }

    T* ptr_{nullptr};
    ControlBlockBase* block_{nullptr};

    template <typename U, typename... Args>
    friend SharedPtr<U> MakeShared(Args&&... args);

    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;

    template <typename U>
    friend class EnableSharedFromThis;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {

}

// Allocate memory only once
template <typename U, typename... Args>
SharedPtr<U> MakeShared(Args&&... args) {
    auto block = new ControlBlockHolder<U>(std::forward<Args>(args)...);
    SharedPtr<U> sp(block->GetRawPointer(), block);
    return sp;
}

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis : public EnableSharedFromThisBase<T> {
public:
    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(self_);
    }
    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<T>(self_);
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return self_;
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return self_;
    }

private:
    WeakPtr<T> self_;

    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;
};
