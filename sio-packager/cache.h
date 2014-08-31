#ifndef CACHE_H
#define CACHE_H

#include <vector>
#include <string>
#include <algorithm>

template <class Entry>
class Cache {
    std::vector<Entry> entries_;
public:

    void sort() {
        std::sort (entries_.begin(), entries_.end(), [] (const Entry &e1, const Entry &e2) { return e1 < e2; });
    }

    void add(Entry &entry) {
        entries_.push_back (entry);
    }

    bool search(const Entry &entry) {
        return std::binary_search (entries_.begin(), entries_.end(), entry, [] (const Entry &e1, const Entry &e2) { return e1 < e2; });
    }

    std::vector<Entry> &entries() {
        return entries_;
    }

    int size() const {
        return entries_.size();
    }
};

#endif //CACHE_H
