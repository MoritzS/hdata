#ifndef _HIERARCHIE_H
#define _HIERARCHIE_H

#include <exception>
#include <vector>

#include "bptree.h"

class hierarchy_error
: public std::exception {
public:
    virtual const char* what() const noexcept {
        return "hierarchy error";
    }
};

class hierarchy_key_not_found
: public hierarchy_error {
public:
    virtual const char* what() const noexcept {
        return "hierarchy error: key not found";
    }
};

template <
    class KeyType,
    class ValueType
>
class Hierarchy {
public:
    typedef BPTree<ValueType, KeyType> ValueTree;

protected:
    ValueTree values;

public:
    Hierarchy(ValueTree values)
    : values(values) {
    }

    Hierarchy()
    : values() {
    }

    virtual ~Hierarchy() {
    }

    ValueType search(KeyType key) {
        ValueType v;
        if (!values.search(key, v)) {
            throw hierarchy_key_not_found();
        }
        return v;
    }

    virtual bool exists(KeyType const key, size_t const version) const = 0;
    virtual size_t num_childs(KeyType const key, size_t const version) const = 0;
    virtual std::vector<KeyType> children(KeyType const key, size_t const version) const = 0;
    virtual bool is_ancestor(KeyType const parent, KeyType const child, size_t const version) const = 0;

    virtual bool exists(KeyType const key) const {
        return exists(key, 0);
    }

    virtual size_t num_childs(KeyType const key) const {
        return num_childs(key, 0);
    }

    virtual std::vector<KeyType> children(KeyType const key) const {
        return children(key, 0);
    };

    virtual bool is_ancestor(KeyType const parent, KeyType const child) const {
        return is_ancestor(parent, child, 0);
    };

    virtual void insert(KeyType const parent, KeyType const key, ValueType const value) = 0;
    virtual void remove(KeyType const key) = 0;
    virtual size_t commit() = 0;
};

#endif
