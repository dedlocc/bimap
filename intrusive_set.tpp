#include "intrusive_set.h"

#include <cassert>
#include <utility>

namespace intrusive {
template <typename T, typename Key, typename Tag, typename Compare>
typename set<T, Key, Tag, Compare>::iterator::reference set<T, Key, Tag, Compare>::iterator::operator*() const noexcept
{
    return *ptr;
}

template <typename T, typename Key, typename Tag, typename Compare>
typename set<T, Key, Tag, Compare>::iterator::pointer set<T, Key, Tag, Compare>::iterator::operator->() const noexcept
{
    return ptr;
}

template <typename T, typename Key, typename Tag, typename Compare>
typename set<T, Key, Tag, Compare>::iterator &set<T, Key, Tag, Compare>::iterator::operator++() noexcept
{
    if (ptr->right) {
        ptr = ptr->right;
        while (ptr->left) {
            ptr = ptr->left;
        }
        return *this;
    }

    while (ptr->parent && ptr != ptr->parent->left) {
        ptr = ptr->parent;
    }

    ptr = ptr->parent;
    return *this;
}

template <typename T, typename Key, typename Tag, typename Compare>
typename set<T, Key, Tag, Compare>::iterator set<T, Key, Tag, Compare>::iterator::operator++(int) & noexcept
{
    auto it = *this;
    ++*this;
    return it;
}

template <typename T, typename Key, typename Tag, typename Compare>
typename set<T, Key, Tag, Compare>::iterator &set<T, Key, Tag, Compare>::iterator::operator--() noexcept
{
    if (ptr->left) {
        ptr = ptr->left;
        while (ptr->right) {
            ptr = ptr->right;
        }
        return *this;
    }

    while (ptr->parent && ptr != ptr->parent->right) {
        ptr = ptr->parent;
    }

    ptr = ptr->parent;
    return *this;
}

template <typename T, typename Key, typename Tag, typename Compare>
typename set<T, Key, Tag, Compare>::iterator set<T, Key, Tag, Compare>::iterator::operator--(int) & noexcept
{
    auto it = *this;
    --*this;
    return it;
}

template <typename T, typename Key, typename Tag, typename Compare>
bool set<T, Key, Tag, Compare>::iterator::operator==(iterator other) const noexcept
{
    return ptr == other.ptr;
}

template <typename T, typename Key, typename Tag, typename Compare>
bool set<T, Key, Tag, Compare>::iterator::operator!=(iterator other) const noexcept
{
    return ptr != other.ptr;
}

template <typename T, typename Key, typename Tag, typename Compare>
set<T, Key, Tag, Compare>::set(set &&other) noexcept :
    sentinel(std::exchange(other.root, nullptr)),
    sz(std::exchange(other.sz, 0)),
    compare(std::move(other.compare))
{}

template <typename T, typename Key, typename Tag, typename Compare>
set<T, Key, Tag, Compare> &set<T, Key, Tag, Compare>::operator=(set &&other) noexcept
{
    std::swap(sentinel, other.root);
    std::swap(sz, other.sz);
    std::swap(compare, other.compare);
    return *this;
}

template <typename T, typename Key, typename Tag, typename Compare>
typename set<T, Key, Tag, Compare>::iterator set<T, Key, Tag, Compare>::link(T &e) noexcept
{
    node_t *p = sentinel;
    node_t **x_ptr = &sentinel->left;

    for (node_t *x = *x_ptr; x; x = *x_ptr) {
        p = x;
        if (auto &k1 = get_key(&e), &k2 = get_key(x); compare(k1, k2)) {
            x_ptr = &x->left;
        } else if (compare(k2, k1)) {
            x_ptr = &x->right;
        } else {
            return end();
        }
    }

    if (p->is_sentinel()) {
        sentinel->left = &e;
    } else {
        *x_ptr = &e;
    }
    e.parent = p;

    splay(&e);
    ++sz;
    return iterator(&e);
}

template <typename T, typename Key, typename Tag, typename Compare>
T &set<T, Key, Tag, Compare>::unlink(iterator it) noexcept
{
    auto x = const_cast<node_t *>(it.ptr);
    assert(x && !x->is_sentinel());

    if (x->left && x->right) {
        node_t *y = x->right;
        while (y->left) {
            y = y->left;
        }
        if (x != y->parent) {
            replace(y, y->right);
            y->right = x->right;
            y->right->parent = y;
        }
        replace(x, y);
        y->left = x->left;
        y->left->parent = y;
    } else {
        replace(x, x->left ? x->left : x->right);
    }

    x->left = x->right = nullptr;
    --sz;
    return static_cast<T &>(*x);
}

template <typename T, typename Key, typename Tag, typename Compare>
typename set<T, Key, Tag, Compare>::iterator set<T, Key, Tag, Compare>::lower_bound(Key const &key) const noexcept
{
    return iterator(lower_bound(key, sentinel->left));
}

template <typename T, typename Key, typename Tag, typename Compare>
typename set<T, Key, Tag, Compare>::iterator set<T, Key, Tag, Compare>::upper_bound(Key const &key) const noexcept
{
    auto it = lower_bound(key);
    if (it != end() && !compare(key, get_key(it.ptr))) {
        ++it;
    }
    return it;
}

template <typename T, typename Key, typename Tag, typename Compare>
typename set<T, Key, Tag, Compare>::iterator set<T, Key, Tag, Compare>::find(Key const &key) const noexcept
{
    auto it = lower_bound(key);
    if (it != end() && compare(key, get_key(it.ptr))) {
        it = end();
    }
    return it;
}

template <typename T, typename Key, typename Tag, typename Compare>
bool set<T, Key, Tag, Compare>::empty() const noexcept
{
    return sz == 0;
}

template <typename T, typename Key, typename Tag, typename Compare>
std::size_t set<T, Key, Tag, Compare>::size() const noexcept
{
    return sz;
}

template <typename T, typename Key, typename Tag, typename Compare>
typename set<T, Key, Tag, Compare>::iterator set<T, Key, Tag, Compare>::begin() const noexcept
{
    if (empty()) {
        return iterator(sentinel);
    }
    node_t *x = sentinel;
    while (x->left) {
        x = x->left;
    }
    return iterator(x);
}


template <typename T, typename Key, typename Tag, typename Compare>
typename set<T, Key, Tag, Compare>::iterator set<T, Key, Tag, Compare>::end() const noexcept
{
    return iterator(sentinel);
}

template <typename T, typename Key, typename Tag, typename Compare>
Compare set<T, Key, Tag, Compare>::key_comp() const noexcept
{
    return compare;
}

template <typename T, typename Key, typename Tag, typename Compare>
Key const &set<T, Key, Tag, Compare>::get_key(node_t const *ptr) const noexcept
{
    return static_cast<T const *>(ptr)->key;
}

template <typename T, typename Key, typename Tag, typename Compare>
void set<T, Key, Tag, Compare>::rotate(node_t *y) const noexcept
{
    assert(y);
    node_t *x = y->parent;
    assert(x);

    node_t **b;
    if (y == x->left) {
        x->left = y->right;
        b = &y->right;
    } else {
        x->right = y->left;
        b = &y->left;
    }

    if (*b) {
        (*b)->parent = x;
    }
    *b = x;

    replace(x, y);
    x->parent = y;
}

template <typename T, typename Key, typename Tag, typename Compare>
void set<T, Key, Tag, Compare>::splay(node_t *x) const noexcept
{
    assert(x);
    while (!x->parent->is_sentinel()) {
        if (!x->parent->parent->is_sentinel()) {
            if ((x == x->parent->left) == (x->parent == x->parent->parent->left)) {
                rotate(x->parent);
            } else {
                rotate(x);
            }
        }
        rotate(x);
    }
}

template <typename T, typename Key, typename Tag, typename Compare>
void set<T, Key, Tag, Compare>::replace(node_t const *old_child, node_t *new_child) const noexcept
{
    assert(old_child);
    node_t *p = old_child->parent;
    assert(p);
    if (p->is_sentinel()) {
        sentinel->left = new_child;
    } else if (old_child == p->left) {
        p->left = new_child;
    } else {
        p->right = new_child;
    }
    if (new_child) {
        new_child->parent = old_child->parent;
    }
}

template <typename T, typename Key, typename Tag, typename Compare>
typename set<T, Key, Tag, Compare>::node_t *set<T, Key, Tag, Compare>::lower_bound(Key const &key, node_t *x) const noexcept
{
    if (!x) {
        return sentinel;
    }

    if (compare(get_key(x), key)) {
        return lower_bound(key, x->right);
    }
    if (compare(key, get_key(x))) {
        if (auto lb = lower_bound(key, x->left); !lb->is_sentinel()) {
            return lb;
        }
    }

    return x;
}
}