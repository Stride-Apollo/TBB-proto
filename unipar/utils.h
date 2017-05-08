#pragma once

#include <utility>
#include <type_traits>

namespace unipar {
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



template <typename _Type, typename _Func>
struct TypeFunc {
	using Type = _Type;
	using Func = _Func;
	Func func;
	
	static_assert(std::is_convertible<decltype(func()), Type>::value,
				  "The function has to return a type castable to the specified type");
	
	TypeFunc(const Func& _func): func(_func) {}
};


template <typename... TFs>
struct ResourceManager;

template <typename TF, typename... Rest>
struct ResourceManager<TF, Rest...> {
	TF tf;
	typename TF::Type value;
	ResourceManager<Rest...> rest;
	
	ResourceManager(TF _tf, Rest&&... rest_tf): tf(_tf), rest(std::forward<Rest>(rest_tf)...) {}
	
	ResourceManager(TF _tf, const ResourceManager<Rest...>& _rest): tf(_tf), rest(_rest) {}
	
	void create() {
		value = tf.func();
		rest.create();
	}
	
	template <typename F, typename... Args>
	auto call(const F& func, Args&&... args) {
		return rest.call(func, value, std::forward<Args>(args)...);
	}
};

template <>
struct ResourceManager<> {
	void create() {}
	
	template <typename F, typename... Args>
	auto call(const F& func, Args&&... args) {
		return func(args...);
	}
};


}
}
