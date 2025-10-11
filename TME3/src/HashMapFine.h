#pragma once

#include <vector>
#include <forward_list>
#include <utility>
#include <cstddef>
#include <mutex>

template<typename K, typename V>
class HashMapFine {
public:
    // Entry stores a const key and a mutable value
    struct Entry {
        const K key;
        V value;
        Entry(const K& k, const V& v) : key(k), value(v) {}
    };

    using Bucket = std::forward_list<Entry>;
    using Table  = std::vector<Bucket>;

    // Construct with a number of buckets (must be >= 1)
    HashMapFine(std::size_t nbuckets = 4096) : buckets_(nbuckets) , vect_m(nbuckets){}

    // Increment frequency for the given word
    void incrementFrequency(const K& key, V delta = 1) {
        //ici le compteur est en atomic obligatoire
        //car un thread verroue sur un bucket i et un autre sur un bucket j
        //les occurences son bonnes mais le compteur est en data race (i et j incrémente au même temps)
        std::size_t idx = std::hash<K>{}(key) % buckets_.size();
        // On verrou sur le bucket concerné
        std::unique_lock<std::mutex> lock(vect_m[idx]);
        for (Entry &e : buckets_[idx]) {
            if (e.key == key) { e.value += delta; compteur++; return; }
        }
        buckets_[idx].emplace_front(key, delta);
        compteur++;
    }

    // Convert table contents to a vector of key/value pairs.
    std::vector<std::pair<K,V>> toKeyValuePairs() const {
        std::vector<std::pair<K,V>> out;
        for (const auto &bucket : buckets_) {
            for (const auto &e : bucket) {
                out.emplace_back(e.key, e.value);
            }
        }
        return out;
    }
    int getTotalWords() const {
        return compteur;
    }

private:
    Table buckets_;
    mutable std::vector<std::mutex> vect_m;
    // Obligatoire car y'a pas un seule mutex (vecteur)
    std::atomic<int> compteur = 0;
};
