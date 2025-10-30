#include <thread>
#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include <condition_variable>

using namespace std;

int work(int id) {
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate work
    std::cout << "Thread " << id << " done\n";
    return 42;
}

int main (int argc, const char **argv) {
    if (argc < 3) {
        std::cerr << "Invoke N and K" << std::endl;
        exit(1);
    }

    int N = atoi(argv[1]);
    int K = atoi(argv[2]);
	/*
    std::mutex mtx;
    std::condition_variable cv;
    int active = 0;

    std::vector<std::thread> threads;
    threads.reserve(N);

    for (int i = 0; i < N; i++) {
        threads.emplace_back([i, &mtx, &cv, &active, K]() {
            // attendre une "place libre"
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&]() { return active < K; });
                active++;
            }

            work(i);

            // libérer la "place"
            {
                std::lock_guard<std::mutex> lock(mtx);
                active--;
            }
            cv.notify_one();
        });
    }

    for (auto &t : threads)
        t.join();

    std::cout << "All threads finished cleanly.\n";
    return 0;
	*/

	// lancer N thread
	vector<thread> threads;
	threads.reserve(argc-2);
	int active = 0;
	int lastJoin=0;
	for (int i=0; i < N ; i++) {
		threads.emplace_back(work, i);
		active++;
		if (active >=K) {
			// garantir au plus K thread actifs.
			threads[lastJoin].join();
			lastJoin++;
		}
	}
	// sortie propre
	for (int i=lastJoin; i < N ; i++) {
		auto & t=threads[i];
		t.join();
	}
	return 0;
}
