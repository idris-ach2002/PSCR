#ifndef INTVECTOR_H
#define INTVECTOR_H

#include <vector>
#include <functional> // For size_t

class IntVector {
public:
    std::vector<int> values;

    // Constructors
    IntVector() = default;
    IntVector(const std::vector<int>& values_) : values(values_) {}
    IntVector(size_t size) : values(size, 0) {}

    // Operator overloads
    IntVector operator+(const IntVector& other) const {
        std::vector<int> result(values.size());
        for (size_t i = 0; i < values.size(); ++i) {
            result[i] = values[i] + other.values[i];
        }
        return IntVector(result);
    }

    IntVector operator-(const IntVector& other) const {
        std::vector<int> result(values.size());
        for (size_t i = 0; i < values.size(); ++i) {
            result[i] = values[i] - other.values[i];
        }
        return IntVector(result);
    }

    bool operator==(const IntVector& other) const {
        return values == other.values;
    }

    bool operator<=(const IntVector& other) const {
        for (size_t i = 0; i < values.size(); ++i) {
            if (values[i] > other.values[i]) {
                return false;
            }
        }
        return true;
    }

    size_t hash() const {
        size_t seed = 0;
        for (int v : values) {
            seed ^= std::hash<int>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

#endif // INTVECTOR_H
