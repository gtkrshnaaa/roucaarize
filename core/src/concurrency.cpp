/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * concurrency.cpp - Implementation of the lightweight async executor
 */

#include "concurrency.hpp"
#include "value.hpp"
#include <pthread.h>
#include <stdexcept>
#include <iostream>

namespace roucaarize {

struct TaskContext {
    std::function<Value()> task;
    std::promise<Value> promise;
};

static void* threadRunner(void* arg) {
    std::unique_ptr<TaskContext> ctx(static_cast<TaskContext*>(arg));
    try {
        Value res = ctx->task();
        ctx->promise.set_value(std::move(res));
    } catch (...) {
        ctx->promise.set_exception(std::current_exception());
    }
    return nullptr;
}

std::shared_ptr<std::future<Value>> AsyncExecutor::spawn(std::function<Value()> task) {
    auto ctx = new TaskContext{std::move(task), std::promise<Value>()};
    auto future = std::make_shared<std::future<Value>>(ctx->promise.get_future());
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // Force 512KB stack size max (Linux default is 8MB)
    // allowing tens of thousands of concurrent futures without draining RAM
    pthread_attr_setstacksize(&attr, 512 * 1024);
    
    pthread_t thread;
    if (pthread_create(&thread, &attr, threadRunner, ctx) != 0) {
        delete ctx;
        pthread_attr_destroy(&attr);
        throw std::runtime_error("OS failed to spawn a new lightweight thread (Resource exhaustion?)");
    }
    
    // Detach the thread since we sync over std::future, we don't need pthread_join
    pthread_detach(thread);
    pthread_attr_destroy(&attr);
    
    return future;
}

} // namespace roucaarize
