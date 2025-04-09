#ifndef COROUTINEUSE_TASK_TASK_H_
#define COROUTINEUSE_TASK_TASK_H_


#include "task_promise.h"
#include <coroutine>

template <typename ResultType>
struct Task {
    using promise_type = TaskPromise<ResultType>;

    explicit Task(std::coroutine_handle<promise_type> h) noexcept: handle(h) {}
    Task(Task &&rhs) noexcept: handle(std::exchange(rhs.handle, {})) {}
    Task(Task &) = delete;
    Task& operator=(Task &) = delete;
    ~Task() {
        if (handle) {
            handle.destroy();
        }
    }

    ResultType get_result() {
        return handle.promise().get_result();
    }

    Task& then(std::function<void(ResultType)> &&func) {
        // 异步操作就是注册对应的回调即可
        handle.promise().on_completed([func] (auto result) {
            try {
                func(result.get_or_throw());
            } catch (std::exception &e) {
                // ignore, 留到catching里面再被捕获
            }
        });
        return *this;
    }

    Task& catching(std::function<void(std::exception &)> &&func) {
        handle.promise().on_completed([func] (auto result) {
            try {
                result.get_or_throw();
            } catch (std::exception &e) {
                func(e);
            }
        });
        return *this;
    }

    Task& finally(std::function<void()> &&func) {
        handle.promise().on_completed([func] (auto result) {
            func();
        });
        return *this;
    }
private:
    std::coroutine_handle<promise_type> handle;
};

#endif //COROUTINEUSE_TASK_TASK_H_