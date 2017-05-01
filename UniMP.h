#pragma once

#include <omp.h>

#include "tbb/parallel_for.h"

// Some utilities
// --------------
namespace utils {

// Taken from http://stackoverflow.com/a/1953137/2678118
template<bool, typename T1, typename T2>
struct is_cond {
	typedef T1 type;
};

template<typename T1, typename T2>
struct is_cond<false, T1, T2> {
	typedef T2 type;
};

template<typename T1, typename T2>
struct largest {
	typedef typename is_cond< (sizeof(T1)>sizeof(T2)), T1, T2>::type type;
};

template<typename T1, typename T2, typename T3>
struct largest3 {
	typedef typename is_cond< (sizeof(T1)>sizeof(T2)), T1, T2>::type _type_temp;
	typedef typename is_cond< (sizeof(_type_temp)>sizeof(T3)), _type_temp, T3>::type type;
};

}

// ParallelRegions
// ---------------

class ParallelRegion {
public:
	ParallelRegion(size_t nthreads): m_nthreads(nthreads) {}
	
	ParallelRegion() {
		m_nthreads = 8;  // TODO automatically determine nthreads
	}
	
	template <typename IndexF, typename IndexL, typename Func>
	void for_(IndexF first, IndexL last, const Func& f) {
		for_(first, last, typename utils::largest<IndexF, IndexL>::type(1), f);
	}
	
	// Dummy non-multithreaded implementation
	template <typename IndexF, typename IndexL, typename IndexS, typename Func>
	void for_(IndexF first, IndexL last, IndexS step, const Func& f) {
		for (IndexF i=first; i < last; i += step) {
			f(i);
		}
	}
	
protected:
	size_t m_nthreads;
};

class OpenmpParallelRegion : public ParallelRegion {
public:
	template <typename IndexF, typename IndexL, typename Func>
	void for_(IndexF first, IndexL last, const Func& f) {
		ParallelRegion::for_(first, last, f);
	}
	
	template <typename IndexF, typename IndexL, typename IndexS, typename Func>
	void for_(IndexF first, IndexL last, IndexS step, const Func& f) {
		#pragma omp parallel for num_threads(m_nthreads) schedule(runtime)
		for (IndexF i=first; i < last; i += step) {
			f(i);
		}
	}
};

class TbbParallelRegion : public ParallelRegion {
public:
	template <typename IndexF, typename IndexL, typename Func>
	void for_(IndexF first, IndexL last, const Func& f) {
		ParallelRegion::for_(first, last, f);
	}
	
	template <typename IndexF, typename IndexL, typename IndexS, typename Func>
	void for_(IndexF first, IndexL last, IndexS step, const Func& f) {
		tbb::parallel_for<
			typename utils::largest3<IndexF, IndexL, IndexS>,
			typename utils::largest3<IndexF, IndexL, IndexS>,
			typename utils::largest3<IndexF, IndexL, IndexS>>(first, last, step, f);
	}
};

// TODO build config, pick only the available configs
