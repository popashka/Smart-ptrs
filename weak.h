#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() {
    }

    WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++block_->weak_counter_;
        }
    }
    WeakPtr(WeakPtr&& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++block_->weak_counter_;
            other.Reset();
        }
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_) {
            ++block_->weak_counter_;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }
        Reset();
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (other.block_) {
            ++block_->weak_counter_;
        }
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        if (ptr_ == other.ptr_) {
            return *this;
        }
        Reset();
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (other.block_) {
            ++block_->weak_counter_;
        }
        other.Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_) {
            if (block_->ref_counter_ + block_->weak_counter_ == 1) {
                // block_->~ControlBlockBase();
                delete block_;
            } else {
                --block_->weak_counter_;
            }
        }
        ptr_ = nullptr;
        block_ = nullptr;
    }

    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (!block_) {
            return 0;
        }
        return block_->ref_counter_;
    }
    bool Expired() const {
        return (!block_ || !block_->ref_counter_);
    }
    SharedPtr<T> Lock() const {
        if (block_ && block_->ref_counter_) {
            return SharedPtr<T>(ptr_, block_);
        }
        return SharedPtr<T>();
    }

private:
    T* ptr_{nullptr};
    ControlBlockBase* block_{nullptr};

    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class EnableSharedFromThis;
};
