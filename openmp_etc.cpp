
#include <string>
#include <iostream>
#include <mutex>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <functional>
#include <cassert>
#include <thread>
#include <random>
#include <chrono>
using Clock = std::chrono::high_resolution_clock;

#include <omp.h>

using namespace std;

int rand() {
	random_device rd;
	auto d = uniform_int_distribution<int>(0, 100);
	return d(rd);
}

int do_work(int ID) {
	uint total = 0;
	for (uint i=0; i<5000; i++) {
		for (uint j=2; j<i; j++) {
			if (i%j == 0) {
				goto not_prime;
			}
		}
		total++;
		not_prime:;
	}
	assert(total == 671);
	cout << ID << endl;
	return total;
}

int main() {
	#pragma omp parallel num_threads(8)
	{
		cout << "This is thread " << omp_get_thread_num() << endl;
		#pragma omp for
		for (int i=0; i<20; i++) {
			do_work(omp_get_thread_num());
		}
	}
	
	cout << endl;
}
