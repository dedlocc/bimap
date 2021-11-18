#pragma once

#include <type_traits>
#include <iterator>

namespace intrusive {
struct default_tag;

template <typename T, typename Key, typename GetKey, typename Tag, typename Compare>
struct set;

template <typename Tag = default_tag>
struct node
{
    node() = default;

    node(node const &) = delete;
    node(node &&) = delete;

    node &operator=(node const &) = delete;
    node &operator=(node &&) = delete;

private:
    node *left {};
    node *right {};
    node *parent {};

    template <typename T, typename SKey, typename SGetKey, typename STag, typename SCompare>
    friend struct set;
};

template <typename T, typename Key, typename GetKey, typename Tag = default_tag, typename Compare = std::less<Key>>
struct set
{
    using node_t = node<Tag>;

    static_assert(std::is_convertible_v<T &, node_t &>, "value type is not convertible to node");

    struct iterator
    {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T const *;
        using reference = T const &;

        iterator() = default;

        explicit iterator(node_t *ptr) : ptr(ptr)
        {}

        reference operator*() const noexcept;

        pointer operator->() const noexcept;

        iterator &operator++() noexcept;
        iterator operator++(int) & noexcept;

        iterator &operator--() noexcept;
        iterator operator--(int) & noexcept;

        bool operator==(iterator other) const noexcept;
        bool operator!=(iterator other) const noexcept;

    private:
        node_t *ptr {};

        friend struct set;
    };

    explicit set(node_t &sentinel, GetKey get_key = GetKey(), Compare compare = Compare()) :
        root(nullptr),
        sentinel(&sentinel),
        sz(0),
        get_key(get_key),
        compare(std::move(compare))
    {}

    set(set const &) = delete;
    set &operator=(set const &) = delete;

    set(set &&other) noexcept;
    set &operator=(set &&other) noexcept;

    iterator link(T &) noexcept;
    T &unlink(iterator it) noexcept;

    iterator lower_bound(Key const &) const noexcept;
    iterator upper_bound(Key const &) const noexcept;
    iterator find(Key const &) const noexcept;

    bool empty() const noexcept;
    std::size_t size() const noexcept;

    iterator begin() const noexcept;
    iterator end() const noexcept;

    Compare key_comp() const noexcept;

private:
    mutable node_t *root;
    mutable node_t *sentinel;
    std::size_t sz;

    [[no_unique_address]] GetKey get_key;
    [[no_unique_address]] Compare compare;

    void rotate(node_t *y) const noexcept;
    void splay(node_t *x) const noexcept;
    void replace(node_t const *old_child, node_t *new_child) const noexcept;
    node_t *lower_bound(Key const &, node_t *) const noexcept;
};
}

#include "intrusive_set.tpp"
