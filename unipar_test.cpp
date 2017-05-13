#include <iostream>
#include <cassert>
#include <atomic>
#include <memory>
#include <functional>


#include "unipar/pick_choices.h"
#define UNIPAR_IMPL UNIPAR_DUMMY
#include "unipar/pick.h"

using namespace std;
using namespace unipar;

#if __cplusplus <= 201103L 

namespace std {
	template<typename T, typename... Args>
	std::unique_ptr<T> make_unique(Args&&... args) {
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
}

#endif

class SomeClass {
public:
	int i;
	
	SomeClass(int _i=1): i(_i) {
		std::cout << "Initialized with i = " << i << "\n";
	}
	
	SomeClass(const SomeClass& other) {
		std::cout << "Copied\n";
		i = other.i;
	}
	
	SomeClass(const SomeClass&& other) {
		std::cout << "Moved\n";
		i = other.i;
	}
	
	SomeClass& operator=(const SomeClass& other) {
		std::cout << "Assigned\n";
		i = other.i;
		return *this;
	}
	
	~SomeClass() {
		std::cout << "Destructed....";
	}
};

int main() {
	Parallel parallel;
	auto other_par = parallel.with<unique_ptr<SomeClass>>();
	std::cout << "init\n";
	other_par = Parallel().withFunc<unique_ptr<SomeClass>>([](){
		return make_unique<SomeClass>(6); 
	});
	std::cout << "set func\n";
	
	std::cout << "number of threads = " << other_par.getNumThreads() << "\n";
	std::atomic<int> sum(0);
	other_par.for_(0, 10, [&](unique_ptr<SomeClass>& a, int i) {
		sum += a->i;
	});
	
	std::cout << "sum is " << sum << " and should be " << 100 << "\n";
	//assert(sum == 100);
}
