#include "ThreadPool.h"

#include <iostream>

using namespace pge;

void ThreadPool::WorkerThread::run(WorkerThread* pWorker) {
    while (true) {
        std::unique_lock<std::mutex> lock(pWorker->mutex);

        pWorker->conditionVariable.wait(lock, [pWorker] { return static_cast<bool>(pWorker->proceed); });

        pWorker->proceed = false;

        if (pWorker->pPool == nullptr)
            break;
        else {
            if (pWorker->item != nullptr) {
                pWorker->item->run(pWorker->workerIndex);
                pWorker->item->done = true;
            }

            pWorker->pPool->onWorkerAvailable(pWorker->workerIndex);
        }

        pWorker->conditionVariable.notify_one();
    }
}

void ThreadPool::onWorkerAvailable(size_t workerIndex) {
    std::lock_guard<std::mutex> lock(mutex);

    if (itemQueue.empty())
        availableThreadIndicies.push_back(workerIndex);
    else {
        // Assign new task
        workers[workerIndex]->item = itemQueue.front();
        itemQueue.pop_front();
        workers[workerIndex]->proceed = true;
    }
}

void ThreadPool::create(size_t numWorkers) {
    workers.resize(numWorkers);

    // Add all threads as available and launch threads
    for (size_t i = 0; i < workers.size(); i++) {
        workers[i].reset(new WorkerThread());

        availableThreadIndicies.push_back(i);

        // Block all threads as there are no tasks yet
        workers[i]->pPool = this;
        workers[i]->workerIndex = i;

        workers[i]->start();
    }
}

void ThreadPool::destroy() {
    //std::lock_guard<std::mutex> lock(mutex);

    itemQueue.clear();
    availableThreadIndicies.clear();

    for (size_t i = 0; i < workers.size(); i++) {
        {
            std::lock_guard<std::mutex> lock(workers[i]->mutex);
            workers[i]->item = nullptr;
            workers[i]->pPool = nullptr;
            workers[i]->proceed = true;
            workers[i]->conditionVariable.notify_one();
        }

        workers[i]->thread->join();
    }
}

void ThreadPool::addItem(const std::shared_ptr<WorkItem> &item) {
    std::lock_guard<std::mutex> lock(mutex);

    if (workersAvailable()) {
        size_t workerIndex = availableThreadIndicies.front();

        availableThreadIndicies.pop_front();

        std::lock_guard<std::mutex> lock(workers[workerIndex]->mutex);

        workers[workerIndex]->item = item;
        workers[workerIndex]->proceed = true;
        workers[workerIndex]->conditionVariable.notify_one();
    }
    else
        itemQueue.push_back(item);
}

void ThreadPool::wait() {
    // Try to aquire every mutex until no tasks are left
    while (true) {
        for (size_t i = 0; i < workers.size(); i++) {
            std::unique_lock<std::mutex> lock(workers[i]->mutex);

            WorkerThread* pWorker = workers[i].get();

            workers[i]->conditionVariable.wait(lock, [pWorker] { return !pWorker->proceed; });
        }

        std::lock_guard<std::mutex> lock(mutex);

        if (itemQueue.empty())
            break;
    }
}