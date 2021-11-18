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

    using node_left_t = intrusive::node<left_tag>;
    using node_right_t = intrusive::node<right_tag>;

    struct node_base_t : node_left_t, node_right_t
    {};

    struct node_t : node_base_t
    {
        template <typename L, typename R>
        node_t(L &&left, R &&right) :
            left(std::forward<L>(left)),
            right(std::forward<R>(right))
        {}

        struct get_left
        {
            left_t const &operator()(void const *node) const noexcept
            {
                return static_cast<node_t const *>(static_cast<node_left_t const *>(node))->left;
            };
        };

        struct get_right
        {
            right_t const &operator()(void const *node) const noexcept
            {
                return static_cast<node_t const *>(static_cast<node_right_t const *>(node))->right;
            };
        };

    private:
        left_t left;
        right_t right;

        friend struct bimap;
    };

    template <typename T>
    struct base_iterator;

    template <typename T>
    struct key_traits;

    template <>
    struct key_traits<left_tag>;

    template <>
    struct key_traits<right_tag>;

    template <>
    struct key_traits<left_tag>
    {
        using value = left_t;
        using node = node_left_t;
        static constexpr typename node_t::get_left get_key;
        using set = intrusive::set<node, value, decltype(get_key), left_tag, CompareLeft>;
        using iterator = base_iterator<left_tag>;
        using flipped = key_traits<right_tag>;
    };

    template <>
    struct key_traits<right_tag>
    {
        using value = right_t;
        using node = node_right_t;
        static constexpr typename node_t::get_right get_key;
        using set = intrusive::set<node, value, decltype(get_key), right_tag, CompareRight>;
        using iterator = base_iterator<right_tag>;
        using flipped = key_traits<left_tag>;
    };

    template <typename T>
    struct base_iterator
    {
    private:
        using traits = key_traits<T>;
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

        explicit base_iterator(typename traits::node const &node) : it(const_cast<typename traits::node *>(&node))
        {}

        friend struct bimap;
        friend flipped_iterator;
    };

public:
    using left_iterator  = base_iterator<left_tag>;
    using right_iterator = base_iterator<right_tag>;

    explicit bimap(CompareLeft compare_left = CompareLeft(), CompareRight compare_right = CompareRight()) :
        set_left(sentinel, key_traits<left_tag>::get_key, std::move(compare_left)),
        set_right(sentinel, key_traits<right_tag>::get_key, std::move(compare_right))
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
    mutable node_base_t sentinel;

    typename key_traits<left_tag>::set set_left;
    typename key_traits<right_tag>::set set_right;

    template <typename L, typename R>
    left_iterator insert_forward(L &&left, R &&right);
};

#include "bimap.tpp"
