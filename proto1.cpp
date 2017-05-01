
#include <string>
#include <iostream>
#include <mutex>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <functional>
#include <cassert>
#include <chrono>
using Clock = std::chrono::high_resolution_clock;

#include <omp.h>

#include <tbb/parallel_reduce.h>
#include <tbb/task_scheduler_init.h>
#include <tbb/blocked_range.h>

using namespace std;


Clock::duration no_mp(size_t max, size_t nprimes) {
	auto start = Clock::now();
	
	vector<size_t> primes;
	
	for (size_t i=0; i<max; i++) {
		for (size_t j=2; j<i; j++) {
			if (i%j == 0) goto not_a_prime;
		}
		primes.push_back(i);
		
		not_a_prime:;
	}
	
	assert(primes.size() == nprimes);
	return (Clock::now() - start);
}

Clock::duration no_mp_count(size_t max, size_t nprimes) {
	auto start = Clock::now();
	size_t count = 0;
	
	for (size_t i=0; i<max; i++) {
		for (size_t j=2; j<i; j++) {
			if (i%j == 0) goto not_a_prime;
		}
		count++;
		
		not_a_prime:;
	}
	
	assert(count == nprimes);
	return (Clock::now() - start);
}


Clock::duration open_mp(size_t max, size_t nprimes) {
	auto start = Clock::now();
	
	mutex m;
	vector<size_t> primes;
	size_t nthreads = 8;
	size_t chunksize = max/nthreads;
	size_t extra = max%nthreads;
	
	#pragma omp parallel num_threads(nthreads)
	{
		size_t thread = omp_get_thread_num();
		vector<size_t> buffer;
		
		size_t i = thread*chunksize;
		size_t end = ((thread+1)*chunksize) + (thread+1==nthreads? extra : 0);
		for (; i<end; i++) {
			for (size_t j=2; j<i; j++) {
				if (i%j == 0) goto not_a_prime;
			}
			buffer.push_back(i);
			
			not_a_prime:;
		}
		
		m.lock();
		primes.reserve(buffer.size());
		copy(buffer.begin(), buffer.end(), back_inserter(primes));
		m.unlock();
	}
	
	assert(primes.size() == nprimes);
	return (Clock::now() - start);
}


Clock::duration open_mp_count(size_t max, size_t nprimes) {
	auto start = Clock::now();
	size_t count = 0;
	
	#pragma omp parallel for
	for (size_t i=0; i<max; i++) {
		for (size_t j=2; j<i; j++) {
			if (i%j == 0) goto not_a_prime;
		}
		#pragma omp atomic
			count++;
		
		not_a_prime:;
	}
	
	assert(count == nprimes);
	return (Clock::now() - start);
}


Clock::duration intel_tbb(size_t max, size_t nprimes) {
	auto start = Clock::now();
	
	vector<size_t> primes = tbb::parallel_reduce(
		tbb::blocked_range<size_t>(0, max),  // Range
		vector<size_t>(), // Identity
		[](const tbb::blocked_range<size_t>& r, vector<size_t> buffer) -> vector<size_t> {
			for (size_t i=r.begin(); i<r.end(); ++i) {
				for (size_t j=2; j<i; j++) {
					if (i%j == 0) goto not_a_prime;
				}
				buffer.push_back(i);
				
				not_a_prime:;
			}
			return buffer;
		},
		[](const vector<size_t>& a, const vector<size_t>& b) -> vector<size_t> {
			vector<size_t> c = a;
			c.reserve(b.size());
			copy(b.begin(), b.end(), back_inserter(c));
			return c;
		}
	);
	
	assert(primes.size() == nprimes);
	
	return (Clock::now() - start);
}


Clock::duration intel_tbb_count(size_t max, size_t nprimes) {
	auto start = Clock::now();
	
	size_t count = tbb::parallel_reduce(
		tbb::blocked_range<size_t>(0, max),  // Range
		0, // Identity
		[](const tbb::blocked_range<size_t>& r, size_t temp) -> size_t {
			for (size_t i=r.begin(); i<r.end(); ++i) {
				for (size_t j=2; j<i; j++) {
					if (i%j == 0) goto not_a_prime;
				}
				temp++;
				
				not_a_prime:;
			}
			return temp;
		},
		plus<size_t>()
	);
	
	assert(count == nprimes);
	
	return (Clock::now() - start);
}


int main() {
	map<string, function<Clock::duration(size_t, size_t)>> funcs;
	funcs["Nope"] = no_mp;
	funcs["OMP"] = open_mp;
	funcs["TBB"] = intel_tbb;
	funcs["NopeC"] = no_mp_count;
	funcs["OMPC"] = open_mp_count;
	funcs["TBBC"] = intel_tbb_count;
	
	vector<size_t> sizes =  {5000, 50000, 75000, 100000};
	vector<size_t> primes = {671,  5135,  7395,  9594};
	
	cout << "max";
	for (auto& it: funcs) {
		cout << "\t" << it.first;
	}
	cout << "\n";
	
	for (size_t i=0; i<sizes.size(); i++) {
		cout << sizes[i] << "\t";
		for (auto& it: funcs) {
			auto dur = it.second(sizes[i], primes[i]);
			cout << chrono::duration_cast<chrono::milliseconds>(dur).count() << "\t";
		}
		cout << "\n";
	}
}
