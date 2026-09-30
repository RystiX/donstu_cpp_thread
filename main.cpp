#include <iostream>
#include <vector>
#include <thread>
#include <sstream>
#include <future>
#include <condition_variable>

#include "threadfuncs.h"

void resultThread(std::promise<std::string> prom)
{
    int completed = 0;

    for (int i = 0; i < 3; ++i) {
        ++completed;
    }

    prom.set_value(
        "Выполнено итераций: " + std::to_string(completed)
    );
}

void producer(
    int& buffer,
    bool& ready,
    bool& done,
    std::mutex& mutex,
    std::condition_variable& cv)
{
    for (int i = 1; i <= 10; ++i) {
        std::unique_lock<std::mutex> lock(mutex);

        cv.wait(lock, [&] {
            return !ready;
        });

        buffer = i;
        ready = true;

        lock.unlock();
        cv.notify_one();
    }

    {
        std::lock_guard<std::mutex> lock(mutex);
        done = true;
    }

    cv.notify_one();
}

void consumer(
    int& buffer,
    bool& ready,
    bool& done,
    std::mutex& mutex,
    std::condition_variable& cv,
    Logger& logger)
{
    while (true) {
        int value;

        {
            std::unique_lock<std::mutex> lock(mutex);

            cv.wait(lock, [&] {
                return ready || done;
            });

            if (!ready && done) {
                break;
            }

            value = buffer;
            ready = false;
        }

        logger.writeLine(
            "Consumer received: " + std::to_string(value)
        );

        cv.notify_one();
    }
}

int main()
{
    about();

    Logger logger("output.log");

    std::cout << "main: opened file: 'output.log'\n";

    // Promise / Future
    std::promise<std::string> prom;
    std::future<std::string> fut = prom.get_future();

    std::thread resultT(
        resultThread,
        std::move(prom)
    );

    std::cout << fut.get() << "\n";

    resultT.join();

    // Producer / Consumer
    std::mutex bufferMutex;
    std::condition_variable cv;

    int buffer = 0;
    bool ready = false;
    bool done = false;

    logger.writeLine("--- producer-consumer start ---");

    std::thread producerThread(
        producer,
        std::ref(buffer),
        std::ref(ready),
        std::ref(done),
        std::ref(bufferMutex),
        std::ref(cv)
    );

    std::thread consumerThread(
        consumer,
        std::ref(buffer),
        std::ref(ready),
        std::ref(done),
        std::ref(bufferMutex),
        std::ref(cv),
        std::ref(logger)
    );

    producerThread.join();
    consumerThread.join();

    logger.writeLine("--- producer-consumer finish ---");

    // Worker threads
    std::vector<ThreadArgs> args(COUNT_THREADS);

    for (int i = 0; i < COUNT_THREADS; ++i) {
        std::ostringstream oss;
        oss << "T" << i;

        args[i].id = i;
        args[i].tag = oss.str();
    }

    std::vector<std::thread> threads;
    threads.reserve(COUNT_THREADS);

    for (int i = 0; i < COUNT_THREADS; ++i) {
        threads.emplace_back(
            funcThread,
            std::cref(args[i]),
            std::ref(logger)
        );
    }

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "counter = " << counter << "\n";

    logger.writeLine("main: all threads finished, file closed");

    return 0;
}