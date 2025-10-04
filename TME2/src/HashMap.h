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
        std::size_t position = h % buckets_.size();
        //On cherche de la liste chaînnée et on évite la copie
        Bucket & my_list = buckets_[position];
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
        if(count_ >= buckets_.size() * 0.8)
            grow();
        //Calcule le hash de la clé
        std::size_t h = std::hash<K>()(key);
        //la postion de la clé se trouve à hash/nbuckets
        std::size_t position = h % buckets_.size();
        //On cherche de la liste chaînnée et on évite la copie (d'ou la référence)
        Bucket & my_list = buckets_[position];
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
        for(const Bucket & l : buckets_) {
            for(const Entry & e : l) {
                s++;
            }
        }
        return s;
    }

    // Convert table contents to a vector of key/value pairs.
    std::vector<std::pair<K,V>> toKeyValuePairs() const {
        std::vector<std::pair<K, V>> vec;
        vec.reserve(size()); // càd on demande une pré allocation de size()
        for(const Bucket & l : buckets_) {
            for(const Entry & e : l) {
                vec.emplace_back(std::make_pair(e.key_, e.value_));
            }
        }
        return vec;
    }

    // Optional: number of buckets
    std::size_t bucket_count() const {
        std::size_t s = 0;
        for(const Bucket & l : buckets_) {s++;}
        return s;
    }

    /*
    12) BONUS: Si la taille actuelle est supérieure ou égale à 80\% du nombre de buckets, la table est considérée surchargée :
    la plupart des accès vont nécessiter d'itérer des listes. On souhaite dans ce cas doubler la taille d'allocation 
    (nombre de buckets).  Ecrivez une fonction membre \texttt{void grow()} qui agrandit (double le nombre de buckets) 
    d'une table contenant déjà des éléments.  Quelle est la complexité de cette réindexation ?

    Indices : créez une table vide mais allouée à la bonne taille, itérer nos entrées actuelles 
    et les insérer une par une dans la cible, écraser notre table avec celle de l'autre (voire utiliser un `std::move`).
    */

    void grow() {
        Table tmp(buckets_.size() * 2); // nouvelle table avec 2x plus de buckets

        for (const Bucket & l : buckets_) {
            for (const Entry & e : l) {
                std::size_t h = std::hash<K>()(e.key_);
                std::size_t pos = h % tmp.size();
                tmp[pos].emplace_front(e.key_, e.value_);
            }
        }

        buckets_ = std::move(tmp);
    }

    /*
    On parcourt tous les éléments existants (il y en a count_). O(N)
    Chaque insertion est O(1) amorti (car liste chaînée + modulo rapide).
    Donc complexité = O(n) avec n = nombre d’éléments déjà stockés.
    Comme grow() ne se déclenche que lorsque la table est presque pleine (load factor > 0.8),
    ça donne en pratique un coût amorti O(1) par insertion (c’est le principe des hashmaps).
    */

private:
    Table buckets_;
    std::size_t count_ = 0;
};
