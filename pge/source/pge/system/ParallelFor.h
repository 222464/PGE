#pragma once

#include <algorithm>
#include <thread>

namespace pge {
	template<typename Iterator, class Function>
	void parallelFor(const Iterator &first, const Iterator &last, Function &&f, const int nthreads = 1, const int threshold = 1000)
	{
		const unsigned int group = std::max(std::max(ptrdiff_t(1), ptrdiff_t(abs(threshold))), ((last - first)) / abs(nthreads));

		std::vector<std::thread> threads;

		Iterator it = first;

		for (; it < last - group; it += group)
			threads.push_back(std::thread([=, &f](){ std::for_each(it, std::min(it + group, last), f); }));

		std::for_each(it, last, f);
		std::for_each(threads.begin(), threads.end(), [](std::thread& x){ x.join(); });
	}
}