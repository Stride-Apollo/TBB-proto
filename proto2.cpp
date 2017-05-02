
#include <string>
#include <iostream>
#include <mutex>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <functional>
#include <cassert>
#include <chrono>
using Clock = std::chrono::high_resolution_clock;

#include <omp.h>

#include <tbb/parallel_for.h>

#include "UniMP.h"

using namespace std;

// This file tries to emulate the main OpenMP usage of stride better:
/*
#pragma omp parallel num_threads(m_num_threads)
{
	const unsigned int thread = omp_get_thread_num();

	#pragma omp for schedule(runtime)
	for (size_t i = 0; i < m_households.size(); i++) {
		Infector<log_level, track_index_case>::execute(
				m_households[i], m_disease_profile, m_rng_handler[thread], m_calendar);
	}
	#pragma omp for schedule(runtime)
	for (size_t i = 0; i < m_school_clusters.size(); i++) {
		Infector<log_level, track_index_case>::execute(
				m_school_clusters[i], m_disease_profile, m_rng_handler[thread], m_calendar);
	}
	
	...3 more of those ...
}
*/

struct Cluster {
	vector<int> numbers;
	
	Cluster(int size) {
		for (int i=0; i<size; i++) {
			numbers.push_back(i);
		}
	}
};

struct Infector {
	/// Will update each number to the next prime
	static void execute(Cluster& c) {
		for (int& i: c.numbers) {
			for (int j=i+1; ; j++) {
				for (int k=2; k<j; k++) {
					if (j%k == 0) goto not_prime;
					
				}
				
				// Found the next prime
				i = j;
				break;
				
				not_prime:;
			}
		}
	}
};

using Clusters = vector<Cluster>;

Clusters make_clusters(int amount, int size) {
	vector<Cluster> res;
	for (int i=0; i<amount; i++) {
		res.push_back(Cluster(size));
	}
	return res;
}


Clock::duration none(Clusters families, Clusters schools, Clusters communities) {
	auto start = Clock::now();
	
	for (int step=0; step<5; step++) {
		for (size_t i=0; i<families.size(); i++) {
			Infector::execute(families[i]);
		}
		
		for (size_t i=0; i<schools.size(); i++) {
			Infector::execute(schools[i]);
		}
		
		for (size_t i=0; i<communities.size(); i++) {
			Infector::execute(communities[i]);
		}
	}
	
	return (Clock::now() - start);
}

Clock::duration openmp(Clusters families, Clusters schools, Clusters communities) {
	auto start = Clock::now();
	
	for (int step=0; step<5; step++) {
		//cout << "Step " << step << endl;
		
		#pragma omp parallel num_threads(8)
		{
			#pragma omp for schedule(runtime)
			for (size_t i=0; i<families.size(); i++) {
				Infector::execute(families[i]);
			}
			
			#pragma omp for schedule(runtime)
			for (size_t i=0; i<schools.size(); i++) {
				Infector::execute(schools[i]);
			}
			
			#pragma omp for schedule(runtime)
			for (size_t i=0; i<communities.size(); i++) {
				Infector::execute(communities[i]);
			}
		}
	}
	
	return (Clock::now() - start);
}

Clock::duration openmp2(Clusters families, Clusters schools, Clusters communities) {
	auto start = Clock::now();
	
	for (int step=0; step<5; step++) {
		#pragma omp parallel for schedule(runtime) num_threads(8)
		for (size_t i=0; i<families.size(); i++) {
			Infector::execute(families[i]);
		}
			
		#pragma omp parallel for schedule(runtime) num_threads(8)
		for (size_t i=0; i<schools.size(); i++) {
			Infector::execute(schools[i]);
		}
			
		#pragma omp parallel for schedule(runtime) num_threads(8)
		for (size_t i=0; i<communities.size(); i++) {
			Infector::execute(communities[i]);
		}
	}
	
	return (Clock::now() - start);
}


Clock::duration intel_tbb(Clusters families, Clusters schools, Clusters communities) {
	auto start = Clock::now();
	
	for (int step=0; step<5; step++) {
		tbb::parallel_for<int>(0, families.size(), [&](size_t i) {
			Infector::execute(families[i]);
		});
		
		tbb::parallel_for<int>(0, schools.size(), [&](size_t i) {
			Infector::execute(schools[i]);
		});
		
		tbb::parallel_for<int>(0, communities.size(), [&](size_t i) {
			Infector::execute(communities[i]);
		});
	}
	
	return (Clock::now() - start);
}

template <typename UniMpType>
Clock::duration unimp(Clusters families, Clusters schools, Clusters communities) {
	auto start = Clock::now();
	
	for (int step=0; step<5; step++) {
		UniMpType region;
		
		region.for_(0, families.size(), [&](size_t i) {
			Infector::execute(families[i]);
		});
		
		region.for_(0, schools.size(), [&](size_t i) {
			Infector::execute(schools[i]);
		});
		
		region.for_(0, communities.size(), [&](size_t i) {
			Infector::execute(communities[i]);
		});
	}
	
	return (Clock::now() - start);
}

template <typename Func>
Clock::duration measure(const Func& f) {
	vector<Clock::duration> times;
	for (int i=0; i<20; i++) {
		times.push_back(f());
	}
	sort(times.begin(), times.end());
	return accumulate(times.begin(), times.begin()+10, Clock::duration()) / 10;
}

int main() {
	map<string, function<Clock::duration(Clusters, Clusters, Clusters)>> funcs;
	funcs["none"] = none;
	funcs["omp"] = openmp;
	funcs["omp2"] = openmp2;
	funcs["tbb"] = intel_tbb;
	funcs["um_none"] = unimp<ParallelRegion>;
	funcs["um_omp"] = unimp<OpenmpParallelRegion>;
 	funcs["um_tbb"] = unimp<TbbParallelRegion>;
	
	vector<size_t> sizes = {1, 2, 4, 8, 16, 32};
	
	cout << "mulpl";
	for (auto& it: funcs) {
		cout << "\t" << it.first;
	}
	cout << "\n";
	
	for (size_t i=0; i<sizes.size(); i++) {
		size_t size = sizes[i];
		cout << size << "\t";
		auto families = make_clusters(size*356, 6);
		auto schools = make_clusters(size*50, 224);
		auto communities = make_clusters(size*5, 724);
		
		for (auto& it: funcs) {
			auto dur = measure([&](){
				return it.second(families, schools, communities);
			});
			cout << chrono::duration_cast<chrono::milliseconds>(dur).count() << "\t";
		}
		cout << "\n";
	}
}

