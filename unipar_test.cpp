#include <iostream>

#include "unipar/pick_choices.h"
#define UNIPAR_IMPL UNIPAR_DUMMY
#include "unipar/pick.h"

class SomeClass {
public:
	SomeClass(int _i=1): i(_i) {}
	int i;
	
	SomeClass(const SomeClass& other) {
		std::cout << "Copied\n";
		i = other.i;
	}
};

int main() {
	std::cout << "Got here\n";
	Parallel parallel;
	std::cout << "Now here\n";
	auto some_par = parallel.with<SomeClass>(4);
	std::cout << "\nRight here\n";
	auto other_par = some_par.withFunc<SomeClass>([](){ return SomeClass(6); });
	std::cout << "Haha here\n";
	
	int sum = 0;
	other_par.for_(0, 10, [&](SomeClass& a, SomeClass& b, int i) {
		sum += a.i + b.i;
	});
	
	std::cout << "sum is " << sum << " and should be " << 100 << "\n";
}
