#pragma once

#include <exception>
#include <iostream>

class ControlBlockBase {
public:
    virtual void ResetPointer() = 0;

    virtual ~ControlBlockBase() = default;

    size_t ref_counter_{0};
    size_t weak_counter_{0};
};

template <typename U>
class ControlBlockPointer : public ControlBlockBase {
public:
    ControlBlockPointer(U* ptr) : ptr_(ptr) {
    }

    void ResetPointer() override {
        if (ptr_) {
            delete ptr_;
            ptr_ = nullptr;
        }
    }

    ~ControlBlockPointer() override {
        if (ptr_) {
            delete ptr_;
        }
    }

    U* ptr_;
};

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;
