#pragma once

#include <vector>
#include <forward_list>
#include <utility>
#include <cstddef>

template<typename K, typename V>
class HashMap {
public:
    // Entry stores a const key and a mutable value
    struct Entry {  
        const K key_;
        V value_;
        Entry(const K key, V v) : key_(key), value_(v) {}
    };

    using Bucket = std::forward_list<Entry>;
    using Table  = std::vector<Bucket>;

    // Constructw< with a number of buckets (must be >= 1)
    // on crée un vector de nbuckets initialisé par défaut càd
    // on appelle le constructeur par défaut de forward_list 
    // qui crée de liste vide càd le head pointe vers null ptr
    // puis lorsque on va faire le emplace_front car liste vide on le change
    HashMap(std::size_t nbuckets = 1024) : buckets_(nbuckets) {};

    // Return pointer to value associated with key, or nullptr if not found.
    // Only iterate the appropriate bucket.
    V* get(const K& key) {
        //Calcule le hash de la clé
        std::size_t h = std::hash<K>()(key);
        //la postion de la clé se trouve à hash/nbuckets
        std::size_t position = h % buckets.size();
        //On cherche de la liste chaînnée et on évite la copie
        Bucket & my_list = buckets[position];
        V * res = nullptr;
        /*for(auto it = my_list.begin(); it != my_list.end(); it++) {
            Entry & e = *it;
            if(e.key_ == key){
                res = &e.value_;
                break;
            }
        }*/

        for(Entry & e : my_list) {
            if(e.key_ == key){
                res = &e.value_;
                break;
            }
        }
        // ou avec un find if
        /*
        auto it = std::find_if(my_list.begin(), my_list.end(), [&](cont auto &e) {return e.key_ == key});
        if(it != my_list.end())
            res = &((*it).value_);
        */
        return res;
    }

    // Insert or update (key,value).
    // Returns true if an existing entry was updated, false if a new entry was inserted.
    bool put(const K& key, const V& value) {
        //Calcule le hash de la clé
        std::size_t h = std::hash<K>()(key);
        //la postion de la clé se trouve à hash/nbuckets
        std::size_t position = h % buckets.size();
        //On cherche de la liste chaînnée et on évite la copie (d'ou la référence)
        Bucket & my_list = buckets[position];
        // flag pour savoir si c'est un update ou insert
        bool res = false; // default (insert)
        for(Entry & e : my_list) {
            if(e.key_ == key){
                e.value_ = value;
                res = true;
                break;
            }
        }
        if(res == false) {
            my_list.emplace_front(key, value);
            count_++;
        }
        return res;
    }

    // Current number of stored entries
    std::size_t size() const {
        std::size_t s = 0;
        for(Bucket & l : buckets_) {
            for(Entry & e : l) {
                s++;
            }
        }
        return s;
    }

    // Convert table contents to a vector of key/value pairs.
    std::vector<std::pair<K,V>> toKeyValuePairs() const {
        std::vector<std::pair<K, V>> vec();
        vec.reserve(size()); // càd on demande une pré allocation de size()
        for(Bucket & l : buckets_) {
            for(Entry & e : l) {
                vec.emplace_back(std::make_pair(e.key_, e.value_));
            }
        }
        return vec;
    }

    // Optional: number of buckets
    std::size_t bucket_count() const {
        std::size_t s = 0;
        for(Bucket & l : buckets_) {s++;}
        return s;
    }

private:
    Table buckets_;
    std::size_t count_ = 0;
};
