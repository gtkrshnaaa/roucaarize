/**
 * Copyright (c) 2026 Gilang Teja Krishna
 * github.com/gtkrshnaaa
 *
 * concurrency.hpp - Lightweight custom pthread wrapper for RAM efficiency
 */

#ifndef ROUCAARIZE_CONCURRENCY_HPP
#define ROUCAARIZE_CONCURRENCY_HPP

#include <future>
#include <functional>
#include <memory>

namespace roucaarize {

/**
 * Custom Async Executor.
 * Bypasses std::async to force 512 KB Linux OS Thread sizes.
 */
class AsyncExecutor {
public:
    static std::shared_ptr<std::future<class Value>> spawn(std::function<class Value()> task);
};

} // namespace roucaarize

#endif // ROUCAARIZE_CONCURRENCY_HPP
