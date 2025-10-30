#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>


template <typename T>
class Result {
private:
    T valeur;
    bool disponible = false;
    std::mutex mt;
    std::condition_variable cv;

public:
    bool set(const T& val) {
        std::unique_lock<std::mutex> lk(mt);
        if (disponible) return false;      // déjà défini
        valeur = val;
        disponible = true;
        cv.notify_all();                   // réveille les threads en attente
        return true;
    }

    const T& get() {
        std::unique_lock<std::mutex> lk(mt);
        cv.wait(lk, [this]() { return disponible; });  // attend que la valeur soit définie
        disponible = false;
        return valeur;
    }
};

void producerThread (Result<int>& result) {
    std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate work
    result.set(42);
}

void consumerThread (Result<int>& result) {
    std::cout << "Value received: " << result.get() << std::endl;
}

int main() {
    Result<int> result;

    // TODO : creer un thread qui invoque "producerThread" et un thread qui invoque "consumerThread"
    std::thread prod(producerThread, std::ref(result));
    std::thread cons(consumerThread, std::ref(result));

    prod.join();
    cons.join();

    return 0;
}
