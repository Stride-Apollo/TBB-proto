#pragma once

#include "unipar.h"

namespace unipar {
namespace internal {

class _DummyParallel {
public:
	// The dummy implementation will obviously never use more than 1 thread
	// However, to remain compatible with otherwise multithreaded code, we allow
	// this constructor.
	// Currently there is still is a hard guarantee that the various implementations
	// never use more than the specified amount of threads, but in the future this
	// number should be seen as a hint and not a hard limit.
	_DummyParallel(int=1) {}
	
	// Dummy non-multithreaded implementation
	template <typename IndexF, typename IndexL, typename IndexS, typename Func, typename RM>
	void parallelFor(IndexF first, IndexL last, IndexS step, const Func& f, RM& rm) {
		rm.create();
		for (IndexF i=first; i < last; i += step) {
			rm.call(f, i);
		}
	}
	
	inline int getNumThreads() const { return 1; }
};

}

using DummyParallel = internal::ParallelWrapper<internal::_DummyParallel>;

}
