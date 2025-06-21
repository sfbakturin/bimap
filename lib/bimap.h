#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

template< typename Lt, typename Rt, typename CLt, typename CRt >
struct bimap;

#include "bimap_element.h"
#include "bimap_iterator.h"
#include "bimap_tree.h"

template< typename Left, typename Right, typename CompareLeft = std::less< Left >, typename CompareRight = std::less< Right > >
struct bimap
{
  public:
	using left_t = Left;
	using right_t = Right;

	using left_iterator = bimap_details::base_iterator< left_t, right_t, true >;
	using right_iterator = bimap_details::base_iterator< left_t, right_t, false >;

  private:
	using base_t = bimap_details::element_base;
	using data_t = bimap_details::element_data< left_t, right_t >;
	using value_left_t = bimap_details::element_value< true, left_t >;
	using value_right_t = bimap_details::element_value< false, right_t >;

	std::size_t m_count;
	bimap_details::tree< left_t, true, CompareLeft > m_left_tree;
	bimap_details::tree< right_t, false, CompareRight > m_right_tree;

	template< typename left_t_f = left_t, typename right_t_f = right_t >
	left_iterator insert_impl(left_t_f&& left, right_t_f&& right)
	{
		if (find_left(left) != end_left() || find_right(right) != end_right())
		{
			return end_left();
		}
		m_count++;
		data_t* elem = new data_t(std::forward< left_t_f >(left), std::forward< right_t_f >(right));
		base_t* inserted = m_left_tree.insert(static_cast< base_t* >(static_cast< value_left_t* >(elem)));
		m_right_tree.insert(static_cast< base_t* >(static_cast< value_right_t* >(elem)));
		return left_iterator(inserted);
	}

	void erase_impl(base_t* left_to_delete, base_t* right_to_delete) noexcept
	{
		m_count--;
		m_left_tree.erase(left_to_delete);
		m_right_tree.erase(right_to_delete);
		delete static_cast< data_t* >(static_cast< value_left_t* >(left_to_delete));
	}

	void clear() noexcept { erase_left(begin_left(), end_left()); }

  public:
	void swap(bimap& other) noexcept
	{
		std::swap(m_count, other.m_count);
		m_left_tree.swap(other.m_left_tree);
		m_right_tree.swap(other.m_right_tree);
	}

	// Creates a bimap that does not contain any pairs.
	bimap(CompareLeft compare_left = CompareLeft(), CompareRight compare_right = CompareRight()) :
		m_count(0), m_left_tree(std::move(compare_left)), m_right_tree(std::move(compare_right))
	{
		m_left_tree.set_another_tree(m_right_tree.end());
		m_right_tree.set_another_tree(m_left_tree.end());
	}

	bimap(const bimap& other) :
		m_count(0), m_left_tree(other.m_left_tree.get_comparator()), m_right_tree(other.m_right_tree.get_comparator())
	{
		m_left_tree.set_another_tree(m_right_tree.end());
		m_right_tree.set_another_tree(m_left_tree.end());
		try
		{
			for (left_iterator it = other.begin_left(); it != other.end_left(); it++)
			{
				insert(*it, *(it.flip()));
			}
		} catch (...)
		{
			clear();
			throw;
		}
	}

	bimap(bimap&& other) noexcept :
		m_count(other.m_count), m_left_tree(std::move(other.m_left_tree)), m_right_tree(std::move(other.m_right_tree))
	{
	}

	bimap& operator=(const bimap& other)
	{
		if (this != std::addressof(other))
		{
			bimap(other).swap(*this);
		}
		return *this;
	}

	bimap& operator=(bimap&& other) noexcept
	{
		if (this != std::addressof(other))
		{
			bimap(std::move(other)).swap(*this);
		}
		return *this;
	}

	// Invalidates all iterators referencing elements of this bimap
	// (including iterators referencing elements after the last ones).
	~bimap() { clear(); }

	// Insert a pair (left, right), returns an iterator to left.
	// If such left or such right already exists in the bimap, no insertion
	// occurs and end_left() is returned.
	left_iterator insert(const left_t& left, const right_t& right) { return insert_impl(left, right); }

	left_iterator insert(const left_t& left, right_t&& right) { return insert_impl(left, std::move(right)); }

	left_iterator insert(left_t&& left, const right_t& right) { return insert_impl(std::move(left), right); }

	left_iterator insert(left_t&& left, right_t&& right) { return insert_impl(std::move(left), std::move(right)); }

	// Removes an element and its pair.
	// erase of an invalid iterator is undefined.
	// erase(end_left()) and erase(end_right()) are undefined.
	// Let it refer to some element e.
	// erase invalidates all iterators referring to e and to the element paired to e.
	left_iterator erase_left(left_iterator it) noexcept
	{
		left_iterator res(std::next(it));
		erase_impl(it.value, it.flip().value);
		return res;
	}

	// Similar to erase, but by key, removes the element if it is present, otherwise
	// does nothing. Returns whether the pair was deleted.
	bool erase_left(const left_t& left) noexcept
	{
		left_iterator found = find_left(left);
		if (found != end_left())
		{
			erase_left(found);
			return true;
		}
		else
		{
			return false;
		}
	}

	right_iterator erase_right(right_iterator it) noexcept
	{
		right_iterator res(std::next(it));
		erase_impl(it.flip().value, it.value);
		return res;
	}

	bool erase_right(const right_t& right) noexcept
	{
		right_iterator found = find_right(right);
		if (found != end_right())
		{
			erase_right(found);
			return true;
		}
		else
		{
			return false;
		}
	}

	// erase from range, removes [first, last), returns an iterator to the last
	// element after the deleted sequence.
	left_iterator erase_left(left_iterator first, left_iterator last) noexcept
	{
		for (left_iterator it(first); it != last; it = erase_left(it))
		{
		}
		return last;
	}

	right_iterator erase_right(right_iterator first, right_iterator last) noexcept
	{
		for (right_iterator it(first); it != last; it = erase_right(it))
		{
		}
		return last;
	}

	// Returns an iterator over the element. If not found, the corresponding end().
	left_iterator find_left(const left_t& left) const noexcept
	{
		base_t* found = m_left_tree.find(left);
		if (!found)
		{
			return end_left();
		}
		else
		{
			return left_iterator(found);
		}
	}

	right_iterator find_right(const right_t& right) const noexcept
	{
		base_t* found = m_right_tree.find(right);
		if (!found)
		{
			return end_right();
		}
		else
		{
			return right_iterator(found);
		}
	}

	// Returns the opposite element by element.
	// If the element does not exist, throws std::out_of_range.
	const right_t& at_left(const left_t& key) const
	{
		left_iterator found = find_left(key);
		if (found != end_left())
		{
			return *found.flip();
		}
		else
		{
			throw std::out_of_range("No such element was found!");
		}
	}

	const left_t& at_right(const right_t& key) const
	{
		right_iterator found = find_right(key);
		if (found != end_right())
		{
			return *found.flip();
		}
		else
		{
			throw std::out_of_range("No such element was found!");
		}
	}

	// Returns the opposite element by element.
	// If the element does not exist, adds it to the bimap and to the opposite.
	// side puts the default element, a reference to which it returns.
	// If the default element is already in the opposite pair, it must change the
	// corresponding element to the requested one (see tests).
	template< typename = std::is_default_constructible< Right > >
	const right_t& at_left_or_default(const left_t& key)
	{
		left_iterator found_left = find_left(key);
		if (found_left != end_left())
		{
			return *found_left.flip();
		}
		right_t default_element{};
		right_iterator found_right = find_right(default_element);
		if (found_right != end_right())
		{
			erase_right(found_right);
		}
		return *(insert(key, std::move(default_element)).flip());
	}

	template< typename = std::is_default_constructible< Left > >
	const left_t& at_right_or_default(const right_t& key)
	{
		right_iterator found_right = find_right(key);
		if (found_right != end_right())
		{
			return *found_right.flip();
		}
		left_t default_element{};
		left_iterator found_left = find_left(default_element);
		if (found_left != end_left())
		{
			erase_left(found_left);
		}
		return *insert(std::move(default_element), key);
	}

	// lower and upper bounds on each side.
	// Return iterators to the corresponding elements.
	// See std::lower_bound, std::upper_bound.
	left_iterator lower_bound_left(const left_t& left) const noexcept { return left_iterator(m_left_tree.next(left)); }

	left_iterator upper_bound_left(const left_t& left) const noexcept { return left_iterator(m_left_tree.prev(left)); }

	right_iterator lower_bound_right(const right_t& right) const noexcept
	{
		return right_iterator(m_right_tree.next(right));
	}

	right_iterator upper_bound_right(const right_t& right) const noexcept
	{
		return right_iterator(m_right_tree.prev(right));
	}

	// Returns an iterator to the minimum order left.
	left_iterator begin_left() const noexcept { return left_iterator(m_left_tree.begin()); }

	left_iterator end_left() const noexcept { return left_iterator(m_left_tree.end()); }

	// Returns an iterator to the minimum order of right.
	right_iterator begin_right() const noexcept { return right_iterator(m_right_tree.begin()); }

	right_iterator end_right() const noexcept { return right_iterator(m_right_tree.end()); }

	// Check for emptiness.
	bool empty() const noexcept { return !m_count; }

	// Returns the size of the bimap (number of pairs).
	std::size_t size() const noexcept { return m_count; }

	friend bool operator==(const bimap& a, const bimap& b) noexcept
	{
		if (a.m_count != b.m_count)
		{
			return false;
		}
		for (left_iterator it_a = a.begin_left(), it_b = b.begin_left(); it_a != a.end_left(); it_a++, it_b++)
		{
			if (!a.m_left_tree.is_equals(*it_a, *it_b) || !a.m_right_tree.is_equals(*(it_a.flip()), *(it_b.flip())))
			{
				return false;
			}
		}
		return true;
	}

	friend bool operator!=(const bimap& a, const bimap& b) noexcept { return !(a == b); }
};
