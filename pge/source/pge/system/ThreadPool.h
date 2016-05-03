#pragma once

#include <pge/system/Uncopyable.h>

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
			std::atomic_bool _done;
		public:
			WorkItem() {
				_done = false;
			}

			virtual ~WorkItem() {}

			virtual void run(size_t threadIndex) = 0;

			bool isDone() const {
				return _done;
			}

			friend ThreadPool;
		};
	private:
		class WorkerThread : public Uncopyable {
		public:
			std::unique_ptr<std::thread> _thread;
			std::mutex _mutex;
			std::condition_variable _conditionVariable;

			std::atomic_bool _proceed;

			std::shared_ptr<WorkItem> _item;

			ThreadPool* _pPool;
			size_t _workerIndex;

			WorkerThread()
				: _pPool(nullptr), _workerIndex(0)
			{
				_proceed = false;
			}

			void start() {
				_thread.reset(new std::thread(&WorkerThread::run, this));
			}

			static void run(WorkerThread* pWorker);
		};

		std::mutex _mutex;

		std::vector<std::unique_ptr<WorkerThread>> _workers;

		std::list<size_t> _availableThreadIndicies;

		std::list<std::shared_ptr<WorkItem>> _itemQueue;

		void onWorkerAvailable(size_t workerIndex);

	public:
		~ThreadPool() {
			destroy();
		}

		void create(size_t numWorkers);
		void destroy();

		void addItem(const std::shared_ptr<WorkItem> &item);

		bool workersAvailable() const {
			return !_availableThreadIndicies.empty();
		}

		size_t getNumWorkers() const {
			return _workers.size();
		}

		void wait();
	};
}