#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <cassert>
#include <iostream>

namespace ts
{

template<typename T>
class thread_safe_ptr
{
    class proxy final {
    public:
        proxy(T &ref, std::mutex &m) : m_ref(ref), m_mutex(m)
        {
            m_mutex.lock();
            std::cout << "lock in thread " << std::this_thread::get_id() << std::endl;
        }
        ~proxy()
        {
            std::cout << "unlock in thread " << std::this_thread::get_id() << std::endl;
            m_mutex.unlock();
        }

        T* operator->() { return &m_ref; }
        const T* operator->() const { return &m_ref; }

    private:
        T &m_ref;
        std::mutex &m_mutex;
    };

public:
    thread_safe_ptr() noexcept = default;
    thread_safe_ptr(std::shared_ptr<T> p) : m_mutex(std::make_shared<std::mutex>()), m_ptr(p) {}
    thread_safe_ptr(const thread_safe_ptr<T> &p) : m_mutex(p.m_mutex), m_ptr(p.m_ptr) {}
    thread_safe_ptr(thread_safe_ptr<T> &&p) noexcept = default;

    thread_safe_ptr<T>& operator=(const thread_safe_ptr &ref)
    {
        if (this != &ref) {
            if (m_mutex) {
                auto t_mutex = m_mutex;
                std::lock_guard<std::mutex> t_locker(*t_mutex.get());

                m_mutex = ref.m_mutex;
                m_ptr   = ref.m_ptr;
            }
            else {
                m_mutex = ref.m_mutex;
                m_ptr   = ref.m_ptr;
            }
        }
        return *this;
    }
    thread_safe_ptr<T>& operator=(thread_safe_ptr &&ref) = default;
    thread_safe_ptr<T>& operator=(std::shared_ptr<T> ref)
    {
        if (m_ptr != ref) {
            if (m_mutex) {
                auto t_mutex = m_mutex;
                std::lock_guard<std::mutex> t_locker(*t_mutex.get());

                m_mutex = std::make_shared<std::mutex>();
                m_ptr   = ref;
            }
            else {
                m_mutex = std::make_shared<std::mutex>();
                m_ptr   = ref;
            }
        }
        return *this;
    }

    T* get() const { m_ptr.get(); }

    proxy operator->()              { return proxy(*get_for_write(), *m_mutex.get()); }
    const proxy operator->() const  { return proxy(*get_for_read() , *m_mutex.get()); }

    operator bool() const noexcept { return m_ptr.get() != nullptr; }

private:
    T* get_for_write()            { assert(m_ptr); return m_ptr.get();  }
    const T* get_for_read() const { assert(m_ptr); return m_ptr.get();  }

private:
    std::shared_ptr<T> m_ptr;
    mutable std::shared_ptr<std::mutex> m_mutex;
};

} // namespace ts
