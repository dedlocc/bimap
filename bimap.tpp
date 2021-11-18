#include "bimap.h"

template <typename L, typename R, typename CL, typename CR>
template <typename Tag>
typename bimap<L, R, CL, CR>::template base_iterator<Tag>::reference bimap<L, R, CL, CR>::base_iterator<Tag>::operator*() const noexcept
{
    return traits::get_key(&*it);
}

template <typename L, typename R, typename CL, typename CR>
template <typename Tag>
typename bimap<L, R, CL, CR>::template base_iterator<Tag>::pointer bimap<L, R, CL, CR>::base_iterator<Tag>::operator->() const noexcept
{
    return &traits::get_key(&*it);
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
    return flipped_iterator(static_cast<typename traits::flipped::node const &>(static_cast<node_base_t const &>(*it)));
}

template <typename L, typename R, typename CL, typename CR>
bimap<L, R, CL, CR>::bimap(bimap const &other) : bimap(other.set_left.key_comp(), other.set_right.key_comp())
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
    set_left.link(node);
    set_right.link(node);

    return left_iterator(node);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::erase_left(left_iterator it)
{
    auto old_it = it++;
    auto *ptr = static_cast<node_t *>(&set_left.unlink(old_it.it));
    set_right.unlink(old_it.flip().it);
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
    return set_left.find(left);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::right_iterator bimap<L, R, CL, CR>::find_right(right_t const &right) const noexcept
{
    return set_right.find(right);
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
        auto node = &static_cast<node_t &>(set_right.unlink(it_right.it));
        node->left = key;
        set_right.link(*node);
        return node->right;
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
        auto node = &static_cast<node_t &>(set_left.unlink(it_left.it));
        node->right = key;
        set_left.link(*node);
        return node->left;
    }

    return *insert(std::move(l), key);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::lower_bound_left(left_t const &left) const noexcept
{
    return set_left.lower_bound(left);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::upper_bound_left(left_t const &left) const noexcept
{
    return set_left.upper_bound(left);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::right_iterator bimap<L, R, CL, CR>::lower_bound_right(right_t const &right) const noexcept
{
    return set_right.lower_bound(right);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::right_iterator bimap<L, R, CL, CR>::upper_bound_right(right_t const &right) const noexcept
{
    return set_right.upper_bound(right);
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::begin_left() const noexcept
{
    return set_left.begin();
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::left_iterator bimap<L, R, CL, CR>::end_left() const noexcept
{
    return set_left.end();
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::right_iterator bimap<L, R, CL, CR>::begin_right() const noexcept
{
    return set_right.begin();
}

template <typename L, typename R, typename CL, typename CR>
typename bimap<L, R, CL, CR>::right_iterator bimap<L, R, CL, CR>::end_right() const noexcept
{
    return set_right.end();
}

template <typename L, typename R, typename CL, typename CR>
bool bimap<L, R, CL, CR>::empty() const noexcept
{
    return set_left.empty();
}

template <typename L, typename R, typename CL, typename CR>
std::size_t bimap<L, R, CL, CR>::size() const noexcept
{
    return set_left.size();
}

template <typename L, typename R, typename CL, typename CR>
bool operator==(bimap<L, R, CL, CR> const &a, bimap<L, R, CL, CR> const &b) noexcept
{
    if (a.size() != b.size()) {
        return false;
    }

    auto a_it = a.begin_left();
    auto b_it = b.begin_left();

    for (auto const a_end = a.end_left(); a_it != a_end; ++a_it, ++b_it) {
        auto &a_l = *a_it;
        auto &b_l = *b_it;
        auto &a_r = *a_it.flip();
        auto &b_r = *b_it.flip();
        auto comp_left = a.set_left.key_comp();
        auto comp_right = a.set_right.key_comp();

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
