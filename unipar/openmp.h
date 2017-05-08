#pragma once

#include "unipar.h"

#include <omp.h>

namespace unipar {

class OpenmpParallel : public Parallel {
public:
	OpenmpParallel() {
		#pragma omp parallel
		{
			m_nthreads = omp_get_num_threads();
		}
	}
	
	OpenmpParallel(int nthreads): Parallel(nthreads) {}
	
	template <typename IndexF, typename IndexL, typename Func>
	void for_(IndexF first, IndexL last, const Func& f) {
		for_(first, last, typename utils::largest<IndexF, IndexL>::type(1), f);
	}
	
	template <typename IndexF, typename IndexL, typename IndexS, typename Func>
	void for_(IndexF first, IndexL last, IndexS step, const Func& f) {
		#pragma omp parallel for num_threads(m_nthreads) schedule(runtime)
		for (IndexF i=first; i < last; i += step) {
			f(i);
		}
	}
};


}
