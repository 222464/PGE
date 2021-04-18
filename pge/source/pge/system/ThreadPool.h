#pragma once

#include "Uncopyable.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <vector>
#include <list>
#include <memory>

namespace pge {
    class ThreadPool {
    public:
        class WorkItem {
        private:
            std::atomic_bool done;
        public:
            WorkItem() {
                done = false;
            }

            virtual ~WorkItem() {}

            virtual void run(size_t threadIndex) = 0;

            bool isDone() const {
                return done;
            }

            friend ThreadPool;
        };
    private:
        class WorkerThread : public Uncopyable {
        public:
            std::unique_ptr<std::thread> thread;
            std::mutex mutex;
            std::condition_variable conditionVariable;

            std::atomic_bool proceed;

            std::shared_ptr<WorkItem> item;

            ThreadPool* pPool;
            size_t workerIndex;

            WorkerThread()
                : pPool(nullptr), workerIndex(0)
            {
                proceed = false;
            }

            void start() {
                thread.reset(new std::thread(&WorkerThread::run, this));
            }

            static void run(WorkerThread* pWorker);
        };

        std::mutex mutex;

        std::vector<std::unique_ptr<WorkerThread>> workers;

        std::list<size_t> availableThreadIndicies;

        std::list<std::shared_ptr<WorkItem>> itemQueue;

        void onWorkerAvailable(size_t workerIndex);

    public:
        ~ThreadPool() {
            destroy();
        }

        void create(size_t numWorkers);
        void destroy();

        void addItem(const std::shared_ptr<WorkItem> &item);

        bool workersAvailable() const {
            return !availableThreadIndicies.empty();
        }

        size_t getNumWorkers() const {
            return workers.size();
        }

        void wait();
    };
}