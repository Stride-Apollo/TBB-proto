#pragma once

#include <utility>

#include "utils.h"

namespace unipar {
namespace internal {

template <typename Impl, typename... TFs>
class ParallelWrapper {
public:
	ParallelWrapper(const Impl& impl): m_impl(impl) {}
	
	template <typename IndexF, typename IndexL, typename Func>
	void for_(IndexF first, IndexL last, const Func& f) {
		for_(first, last, typename unipar::utils::largest<IndexF, IndexL>::type(1), f);
	}
	
	// Dummy non-multithreaded implementation
	template <typename IndexF, typename IndexL, typename IndexS, typename Func>
	void for_(IndexF first, IndexL last, IndexS step, const Func& f) {
		m_impl.parallelFor(first, last, step, f, m_resource_manager);
	}
	
	template <typename T, typename Func>
	auto withFunc(Func f) {
		using TF = utils::TypeFunc<T, decltype(f)>;
		return ParallelWrapper<Impl, TF, TFs...>(m_impl, m_resource_manager, TF(f));
	}
	
	template <typename T, typename... Args>
	auto with(Args... args) {
		return withFunc<T>([=](){
			return T(args...);
		});
	}
	
	// Kinda private, however friend'ing is rather hard with variadic templates
	template <typename PrevResMan, typename newTF>
	ParallelWrapper(Impl& impl, const PrevResMan& rm, newTF&& tf)
		: m_impl(impl), m_resource_manager(std::forward<newTF>(tf), rm) {}
	
	
	// Proxying
	Impl& impl() { return m_impl; };
	const Impl& impl() const { return m_impl; };
	
	int getNumThreads() const { return m_impl.getNumThreads(); }
	
	ParallelWrapper(int nthreads): m_impl(nthreads) {}
	ParallelWrapper(): m_impl() {}
	
protected:
	Impl m_impl;
	utils::ResourceManager<TFs...> m_resource_manager;
};

}
}

