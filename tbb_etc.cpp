
#include <iostream>
#include <chrono>
#include <vector>
using Clock = std::chrono::high_resolution_clock;

#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_for.h"

using namespace std;

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

int main() {
	int n = tbb::task_scheduler_init::default_num_threads();
	for (int p=1; p<=(n+2); ++p) {
		// Construct task scheduler with p threads
		tbb::task_scheduler_init init(p);
		auto start = Clock::now();
		
		// execute parallel algorithm using task or template algorithm here ...
		auto families = make_clusters(5678, 128);
		tbb::parallel_for<int>(0, families.size(), [&](size_t i) {
			Infector::execute(families[i]);
		});
		
		double t = chrono::duration_cast<chrono::milliseconds>(Clock::now() - start).count();
		cout << "time = " << t << " with " << p << " threads\n";
		// Implicitly destroy task scheduler.
	}
	return 0;
}
