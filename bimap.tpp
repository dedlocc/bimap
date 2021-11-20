#include "bimap.h"

template <typename L, typename R, typename CL, typename CR>
template <typename Tag>
typename bimap<L, R, CL, CR>::template base_iterator<Tag>::reference bimap<L, R, CL, CR>::base_iterator<Tag>::operator*() const noexcept
{
    return static_cast<typename traits::node const &>(*it).key;
}

template <typename L, typename R, typename CL, typename CR>
template <typename Tag>
typename bimap<L, R, CL, CR>::template base_iterator<Tag>::pointer bimap<L, R, CL, CR>::base_iterator<Tag>::operator->() const noexcept
{
    return &this->operator*();
}

template <typename L, typename R, typename CL, typename CR>
template <typename Tag>
typename bimap<L, R, CL, CR>::template base_iterator<Tag> &bimap<L, R, CL, CR>::base_iterator<Tag>::operator++() noexcept
{
    ++it;
    return *this;
}

template <typename L, typename R, typename CL, typename CR>
template <typename Tag>
typename bimap<L, R, CL, CR>::template base_iterator<Tag> bimap<L, R, CL, CR>::base_iterator<Tag>::operator++(int) & noexcept
{
    auto res = *this;
    ++it;
    return res;
}

template <typename L, typename R, typename CL, typename CR>
template <typename Tag>
typename bimap<L, R, CL, CR>::template base_iterator<Tag> &bimap<L, R, CL, CR>::base_iterator<Tag>::operator--() noexcept
{
    --it;
    return *this;
}

template <typename L, typename R, typename CL, typename CR>
template <typename Tag>
typename bimap<L, R, CL, CR>::template base_iterator<Tag> bimap<L, R, CL, CR>::base_iterator<Tag>::operator--(int) & noexcept
{
    auto res = *this;
    --it;
    return res;
}

template <typename L, typename R, typename CL, typename CR>
template <typename Tag>
bool bimap<L, R, CL, CR>::base_iterator<Tag>::operator==(base_iterator other) const noexcept
{
    return it == other.it;
}

template <typename L, typename R, typename CL, typename CR>
template <typename Tag>
bool bimap<L, R, CL, CR>::base_iterator<Tag>::operator!=(base_iterator other) const noexcept
{
    return it != other.it;
}

template <typename L, typename R, typename CL, typename CR>
template <typename T>
typename bimap<L, R, CL, CR>::template base_iterator<T>::flipped_iterator bimap<L, R, CL, CR>::base_iterator<T>::flip() const noexcept
{
    auto &node = *it;
    using flipped_node_t = typename traits::flipped::base_node const &;

    if (node.is_sentinel()) {
        return flipped_iterator(static_cast<flipped_node_t>(static_cast<sentinel_t const &>(node)));
    }

    return flipped_iterator(static_cast<flipped_node_t>(static_cast<node_t const &>(node)));
}

template <typename L, typename R, typename CL, typename CR>
bimap<L, R, CL, CR>::bimap(bimap const &other) : bimap(other.left_set.key_comp(), other.right_set.key_comp())
{
    auto const end = other.end_left();
    for (auto it = other.begin_left(); it != end; ++it) {
        insert(*it, *it.flip());
    }
}

template <typename L, typename R, typename CL, typename CR>
bimap<L, R, CL, CR> &bimap<L, R, CL, CR>::operator=(bimap const &other)
{
    erase_left(begin_left(), end_left());

    auto const end = other.end_left();
    for (auto it = other.begin_left(); it != end; ++it) {
        insert(*it, *it.flip());
    }

    return *this;
}

template <typename L, typename R, typename CL, typename CR>
void bimap<L, R, CL, CR>::swap(bimap &other) noexcept
{
    bimap tmp = std::move(other);
    other = std::move(*this);
    *this = std::move(tmp);
}

template <typename L, typename R, typename CL, typename CR>
bimap<L, R, CL, CR>::~bimap()
{
    erase_left(begin_left(), end_left());
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::insert(left_t const &left, right_t const &right)
{
    return insert_forward(left, right);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::insert(left_t &&left, right_t const &right)
{
    return insert_forward(std::move(left), right);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::insert(left_t const &left, right_t &&right)
{
    return insert_forward(left, std::move(right));
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::insert(left_t &&left, right_t &&right)
{
    return insert_forward(std::move(left), std::move(right));
}

template <typename L, typename R, typename CL, typename CR>
template <typename Left, typename Right>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::insert_forward(Left &&left, Right &&right)
{
    if (find_left(left) != end_left() || find_right(right) != end_right()) {
        return end_left();
    }

    auto &node = *new node_t(std::forward<Left>(left), std::forward<Right>(right));
    left_set.link(node);
    right_set.link(node);

    return left_iterator(node);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::erase_left(left_iterator it)
{
    auto old_it = it++;
    auto *ptr = static_cast<node_t *>(&left_set.unlink(old_it.it));
    right_set.unlink(old_it.flip().it);
    delete ptr;
    return it;
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::right_iterator bimap<L, R, CL, CR>::erase_right(right_iterator it)
{
    return erase_left(it.flip()).flip();
}

template <typename L, typename R, typename CL, typename CR>
bool bimap<L, R, CL, CR>::erase_left(left_t const &left)
{
    if (auto it = find_left(left); it != end_left()) {
        erase_left(it);
        return true;
    }
    return false;
}

template <typename L, typename R, typename CL, typename CR>
bool bimap<L, R, CL, CR>::erase_right(right_t const &right)
{
    if (auto it = find_right(right); it != end_right()) {
        erase_right(it);
        return true;
    }
    return false;
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::erase_left(left_iterator first, left_iterator last)
{
    while (first != last) {
        erase_left(first++);
    }
    return last;
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::right_iterator bimap<L, R, CL, CR>::erase_right(right_iterator first, right_iterator last)
{
    while (first != last) {
        erase_right(first++);
    }
    return last;
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::find_left(left_t const &left) const noexcept
{
    return left_set.find(left);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::right_iterator bimap<L, R, CL, CR>::find_right(right_t const &right) const noexcept
{
    return right_set.find(right);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::right_t const &bimap<L, R, CL, CR>::at_left(left_t const &key) const
{
    if (auto it = find_left(key); it != end_left()) {
        return *it.flip();
    }
    throw std::out_of_range("No such element");
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_t const &bimap<L, R, CL, CR>::at_right(right_t const &key) const
{
    if (auto it = find_right(key); it != end_right()) {
        return *it.flip();
    }
    throw std::out_of_range("No such element");
}

template <typename L, typename R, typename CL, typename CR>
template <typename, typename>
typename bimap<L, R, CL, CR>::right_t const &bimap<L, R, CL, CR>::at_left_or_default(left_t const &key)
{
    auto it_left = find_left(key);
    if (it_left != end_left()) {
        return *it_left.flip();
    }

    right_t r {};
    auto it_right = find_right(r);
    if (it_right != end_right()) {
        auto &node = right_set.unlink(it_right.it);
        static_cast<typename left_key_traits::node &>(static_cast<node_t &>(node)).key = key;
        right_set.link(node);
        return node.key;
    }

    return *insert(key, std::move(r)).flip();
}

template <typename L, typename R, typename CL, typename CR>
template <typename, typename>
typename bimap<L, R, CL, CR>::left_t const &bimap<L, R, CL, CR>::at_right_or_default(const right_t &key)
{
    auto it_right = find_right(key);
    if (it_right != end_right()) {
        return *it_right.flip();
    }

    left_t l {};
    auto it_left = find_left(l);
    if (it_left != end_left()) {
        auto &node = left_set.unlink(it_left.it);
        static_cast<typename right_key_traits::node &>(static_cast<node_t &>(node)).key = key;
        left_set.link(node);
        return node.key;
    }

    return *insert(std::move(l), key);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::lower_bound_left(left_t const &left) const noexcept
{
    return left_set.lower_bound(left);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::upper_bound_left(left_t const &left) const noexcept
{
    return left_set.upper_bound(left);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::right_iterator bimap<L, R, CL, CR>::lower_bound_right(right_t const &right) const noexcept
{
    return right_set.lower_bound(right);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::right_iterator bimap<L, R, CL, CR>::upper_bound_right(right_t const &right) const noexcept
{
    return right_set.upper_bound(right);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::begin_left() const noexcept
{
    return left_set.begin();
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::end_left() const noexcept
{
    return left_set.end();
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::right_iterator bimap<L, R, CL, CR>::begin_right() const noexcept
{
    return right_set.begin();
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::right_iterator bimap<L, R, CL, CR>::end_right() const noexcept
{
    return right_set.end();
}

template <typename L, typename R, typename CL, typename CR>
bool bimap<L, R, CL, CR>::empty() const noexcept
{
    return left_set.empty();
}

template <typename L, typename R, typename CL, typename CR>
std::size_t bimap<L, R, CL, CR>::size() const noexcept
{
    return left_set.size();
}

template <typename L, typename R, typename CL, typename CR>
bool operator==(bimap<L, R, CL, CR> const &a, bimap<L, R, CL, CR> const &b) noexcept
{
    if (a.size() != b.size()) {
        return false;
    }

    auto const a_end = a.end_left();

    for (auto a_it = a.begin_left(), b_it = b.begin_left(); a_it != a_end; ++a_it, ++b_it) {
        auto &a_l = *a_it;
        auto &b_l = *b_it;
        auto &a_r = *a_it.flip();
        auto &b_r = *b_it.flip();
        auto comp_left = a.left_set.key_comp();
        auto comp_right = a.right_set.key_comp();

        if (comp_left(a_l, b_l) || comp_left(b_l, a_l) || comp_right(a_r, b_r) || comp_right(b_r, a_r)) {
            return false;
        }
    }
    return true;
}

template <typename L, typename R, typename CL, typename CR>
bool operator!=(bimap<L, R, CL, CR> const &a, bimap<L, R, CL, CR> const &b) noexcept
{
    return !(a == b);
}
