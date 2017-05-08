#pragma once

#include "unipar.h"

#include "tbb/parallel_for.h"
#include "tbb/task_scheduler_init.h"

namespace unipar {

/*
class TbbParallel : public Parallel {
public:
	TbbParallel(int nthreads = -1): Parallel(nthreads) {}
	
	template <typename IndexF, typename IndexL, typename Func>
	void for_(IndexF first, IndexL last, const Func& f) {
		for_(first, last, typename utils::largest<IndexF, IndexL>::type(1), f);
	}
	
	// TBB dislikes manually setting the amount of threads a LOT. In order not introduce
	// unexpected behaviour, we can't use task_scheduler_init for this instance, we need
	// to create one for this call separately.
	// In order to avoid this overhead, don't manually set the amount of threads or create
	// a tbb::task_scheduler_init at the start of your program.
	template <typename IndexF, typename IndexL, typename IndexS, typename Func>
	void for_(IndexF first, IndexL last, IndexS step, const Func& f) {
		if (m_nthreads == -1) {
			tbb::parallel_for<typename utils::largest3<IndexF, IndexL, IndexS>::type, Func>(first, last, step, f);
		} else {
			tbb::task_scheduler_init init(m_nthreads);
			tbb::parallel_for<typename utils::largest3<IndexF, IndexL, IndexS>::type, Func>(first, last, step, f);
		}
	}
	
	int getNumThreads() {
		if (m_nthreads == -1) {
			return tbb::task_scheduler_init::default_num_threads();
		} else {
			return Parallel::getNumThreads();
		}
	}
};
*/

}
