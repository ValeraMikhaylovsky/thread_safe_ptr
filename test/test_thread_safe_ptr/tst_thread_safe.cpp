#include <thread>
#include <memory>
#include <functional>
#include <iostream>

#include <QtTest>

#include "../../include/thread_safe_ptr.h"

class Counter
{
public:
    Counter() = default;
    int get() const {
        std::cout << Q_FUNC_INFO << count << std::endl;
        return count;
    }
    void set(int newCount) {
        std::cout << Q_FUNC_INFO << newCount << std::endl;
        count = newCount;
    }

    void increment() {
        ++count;
        std::cout << Q_FUNC_INFO << count << std::endl;
    }

    void decrement() {
        --count;
        std::cout << Q_FUNC_INFO << count << std::endl;
    }

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

private:
    void writeRandomValue() {
        for (int i = 0; i < 100; ++i)
            pObj->set(qrand()%1000);
    }
    void readRandomValue() {
        for (int i = 0; i < 100; ++i)
            pObj->get();
    }
    void incrementValue() {
        for (int i = 0; i < 100; ++i)
            pObj->increment();
    }
    void decrementValue() {
        for (int i = 0; i < 100; ++i)
            pObj->decrement();
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

QTEST_APPLESS_MAIN(thread_safe)

#include "tst_thread_safe.moc"
