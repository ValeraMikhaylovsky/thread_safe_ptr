#include <thread>
#include <memory>
#include <functional>
#include <chrono>
#include <iostream>

#include <QtTest>

#include "../../include/thread_safe_ptr.h"

using namespace std::chrono_literals;

class Counter
{
public:
    Counter() noexcept = default;
    int get() const noexcept {
        return count;
    }
    void set(int newCount) noexcept {
        count = newCount;
    }

    void increment() noexcept {
        ++count;
    }

    void decrement() noexcept {
        --count;
    }

    bool valid() const noexcept { return count >= 0; }

    bool invalid() const noexcept { return count < 0; }

private:
    int count {0};
};

class thread_safe : public QObject
{
    Q_OBJECT

public:
    thread_safe();
    ~thread_safe();

private slots:
    void test_change_value();
    void test_change_value_in_threads();
    void test_deadlock();
    void test_copy();

    void bench_single_call();
    void bench_multiple_call();

private:
    void writeRandomValue() {
        for (int i = 0; i < 10; ++i) {
            pObj->set(qrand()%1000);
            std::this_thread::sleep_for(1ms);
        }
    }
    void readRandomValue() {
        for (int i = 0; i < 10; ++i) {
            pObj->get();
            std::this_thread::sleep_for(1ms);
        }
    }
    void incrementValue() {
        for (int i = 0; i < 10; ++i) {
            pObj->increment();
            std::this_thread::sleep_for(1ms);
        }
    }
    void decrementValue() {
        for (int i = 0; i < 10; ++i) {
            pObj->decrement();
            std::this_thread::sleep_for(1ms);
        }
    }

    void setValueInThread(ts::thread_safe_ptr<Counter> p, int value) {
        if (p)
            p->set(value);
    }

private:
    ts::thread_safe_ptr<Counter> pObj;
    std::vector<std::unique_ptr<std::thread>> m_threads;
};

thread_safe::thread_safe()
{
    pObj = std::make_shared<Counter>();
}

thread_safe::~thread_safe()
{

}

void thread_safe::test_change_value()
{
    ts::thread_safe_ptr<Counter> pCounter(std::make_shared<Counter>());

    pCounter->set(3);
    QCOMPARE(pCounter->get(), 3);

    pCounter->increment();
    QCOMPARE(pCounter->get(), 4);

    pCounter->decrement();
    QCOMPARE(pCounter->get(), 3);
}

void thread_safe::test_change_value_in_threads()
{
    for (int i = 0; i < 10; ++i) {
        m_threads.emplace_back(std::make_unique<std::thread>(std::bind(&thread_safe::writeRandomValue, this)));
        m_threads.emplace_back(std::make_unique<std::thread>(std::bind(&thread_safe::readRandomValue , this)));
        m_threads.emplace_back(std::make_unique<std::thread>(std::bind(&thread_safe::incrementValue  , this)));
        m_threads.emplace_back(std::make_unique<std::thread>(std::bind(&thread_safe::decrementValue  , this)));
    }

    for (auto &p : m_threads) {
        if (p->joinable())
            p->join();
    }
}

void thread_safe::test_deadlock()
{
    ts::thread_safe_ptr<Counter> pCounter(std::make_shared<Counter>());

    pCounter->set(3);

    if (pCounter->valid() && (pCounter->get() == 3)) {
        QCOMPARE(pCounter->get(), 3);
    }
}

void thread_safe::test_copy()
{
    ts::thread_safe_ptr<Counter> pCounter(std::make_shared<Counter>());
    QCOMPARE(pCounter->get(), 0);

    std::thread t_thread(std::bind(std::bind(&thread_safe::setValueInThread, this, pCounter, 27)));
    if (t_thread.joinable())
        t_thread.join();

    QCOMPARE(pCounter->get(), 27);
}

void thread_safe::bench_single_call()
{
    ts::thread_safe_ptr<Counter> pCounter(std::make_shared<Counter>());

    QBENCHMARK {
        pCounter->get();
        pCounter->increment();
    }
}

void thread_safe::bench_multiple_call()
{
    ts::thread_safe_ptr<Counter> pCounter(std::make_shared<Counter>());

    QBENCHMARK {
        if (pCounter->valid() && (pCounter->get() == 0) && !pCounter->invalid()) {
            pCounter->increment();
        }
    }
}

QTEST_APPLESS_MAIN(thread_safe)

#include "tst_thread_safe.moc"
