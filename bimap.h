#pragma once

#include <cstddef>

#include "intrusive_set.h"

template <typename Left, typename Right, typename CompareLeft, typename CompareRight>
struct bimap;

template <typename L, typename R, typename CL, typename CR>
bool operator==(bimap<L, R, CL, CR> const &a, bimap<L, R, CL, CR> const &b) noexcept;

template <typename L, typename R, typename CL, typename CR>
bool operator!=(bimap<L, R, CL, CR> const &a, bimap<L, R, CL, CR> const &b) noexcept;

template <typename Left, typename Right,
    typename CompareLeft = std::less<Left>, typename CompareRight = std::less<Right>>
struct bimap
{
    using left_t = Left;
    using right_t = Right;

private:
    struct left_tag;
    struct right_tag;

    template <typename T>
    struct base_iterator;

    template <typename T>
    struct node_with_key
    {
        explicit node_with_key(T const &key) : key(key)
        {}

        explicit node_with_key(T &&key) : key(std::move(key))
        {}

        T key;
    };

    struct right_key_traits;

    struct left_key_traits
    {
        using value = left_t;
        using base_node = intrusive::node<left_tag>;
        struct node : base_node, node_with_key<value>
        {
            using node_with_key<value>::node_with_key;
        };
        using set = intrusive::set<node, value, left_tag, CompareLeft>;
        using iterator = base_iterator<left_tag>;
        using flipped = right_key_traits;
    };

    struct right_key_traits
    {
        using value = right_t;
        using base_node = intrusive::node<right_tag>;
        struct node : base_node, node_with_key<value>
        {
            using node_with_key<value>::node_with_key;
        };
        using set = intrusive::set<node, value, right_tag, CompareRight>;
        using iterator = base_iterator<right_tag>;
        using flipped = left_key_traits;
    };

    template <typename T>
    struct base_iterator
    {
    private:
        using traits = std::conditional_t<std::is_same_v<T, left_tag>, left_key_traits, right_key_traits>;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = typename traits::value;
        using pointer = value_type const *;
        using reference = value_type const &;
        using flipped_iterator = typename traits::flipped::iterator;

        base_iterator() = default;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        base_iterator &operator++() noexcept;
        base_iterator operator++(int) & noexcept;

        base_iterator &operator--() noexcept;
        base_iterator operator--(int) & noexcept;

        bool operator==(base_iterator other) const noexcept;
        bool operator!=(base_iterator other) const noexcept;

        flipped_iterator flip() const noexcept;

    private:
        typename traits::set::iterator it;

        base_iterator(typename traits::set::iterator it) : it(it)
        {}

        explicit base_iterator(typename traits::base_node const &node) : it(const_cast<typename traits::base_node *>(&node))
        {}

        friend struct bimap;
        friend flipped_iterator;
    };

    struct node_t : left_key_traits::node, right_key_traits::node
    {
        template <typename L, typename R>
        node_t(L &&left, R &&right) :
            left_key_traits::node(std::forward<L>(left)),
            right_key_traits::node(std::forward<R>(right))
        {}
    };

public:
    using left_iterator = typename left_key_traits::iterator;
    using right_iterator = typename right_key_traits::iterator;

    explicit bimap(CompareLeft compare_left = CompareLeft(), CompareRight compare_right = CompareRight()) :
        left_set(sentinel, std::move(compare_left)),
        right_set(sentinel, std::move(compare_right))
    {}

    bimap(bimap const &other);
    bimap(bimap &&other) noexcept = default;

    bimap &operator=(bimap const &other);
    bimap &operator=(bimap &&other) noexcept = default;

    void swap(bimap &other) noexcept;

    ~bimap();

    left_iterator insert(left_t const &left, right_t const &right);
    left_iterator insert(left_t const &left, right_t &&right);
    left_iterator insert(left_t &&left, right_t const &right);
    left_iterator insert(left_t &&left, right_t &&right);

    left_iterator erase_left(left_iterator it);
    right_iterator erase_right(right_iterator it);

    bool erase_left(left_t const &left);
    bool erase_right(right_t const &right);

    left_iterator erase_left(left_iterator first, left_iterator last);
    right_iterator erase_right(right_iterator first, right_iterator last);

    left_iterator find_left(left_t const &left) const noexcept;
    right_iterator find_right(right_t const &right) const noexcept;

    right_t const &at_left(left_t const &key) const;
    left_t const &at_right(right_t const &key) const;

    template <typename R = right_t, typename = std::enable_if_t<std::is_default_constructible_v<R>>>
    right_t const &at_left_or_default(left_t const &key);

    template <typename L = left_t, typename = std::enable_if_t<std::is_default_constructible_v<L>>>
    left_t const &at_right_or_default(right_t const &key);

    left_iterator lower_bound_left(left_t const &left) const noexcept;
    left_iterator upper_bound_left(left_t const &left) const noexcept;

    right_iterator lower_bound_right(right_t const &right) const noexcept;
    right_iterator upper_bound_right(right_t const &right) const noexcept;

    left_iterator begin_left() const noexcept;
    left_iterator end_left() const noexcept;

    right_iterator begin_right() const noexcept;
    right_iterator end_right() const noexcept;

    bool empty() const noexcept;
    std::size_t size() const noexcept;

    friend bool operator==<>(bimap const &a, bimap const &b) noexcept;
    friend bool operator!=<>(bimap const &a, bimap const &b) noexcept;

private:
    struct sentinel_t : left_key_traits::base_node, right_key_traits::base_node
    {} sentinel;

    typename left_key_traits::set left_set;
    typename right_key_traits::set right_set;

    template <typename L, typename R>
    left_iterator insert_forward(L &&left, R &&right);
};

#include "bimap.tpp"
