#pragma once

#include "bimap_element.h"
#include <type_traits>

#include <cstddef>
#include <iterator>

namespace bimap_details
{
	template< typename Key, typename Value, bool Tree >
	struct base_iterator
	{
	  public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = std::conditional_t< Tree, Key, Value >;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

	  private:
		using value_type_another = std::conditional_t< Tree, Value, Key >;
		using base_t = element_base;
		using elem_t = element_value< Tree, value_type >;
		using elem_another_t = element_value< !Tree, value_type_another >;
		using doub_t = element_data< Key, Value >;

		base_t* value = nullptr;

		template< typename FLt, typename FRt, typename FCLt, typename FCRt >
		friend struct ::bimap;

		friend struct base_iterator< Key, Value, !Tree >;

		base_iterator(base_t* value) noexcept : value(value) {}

	  public:
		// The element that the iterator currently refers to.
		// Dereferencing the iterator end_left() is undefined.
		// Dereferencing an invalid iterator is undefined.
		value_type const & operator*() const noexcept { return static_cast< elem_t* >(value)->get(); }

		value_type const * operator->() const noexcept
		{
			return static_cast< pointer >(&static_cast< elem_t* >(value)->get());
		}

		// Move to the next largest left.
		// The increment of the end_left() iterator is undefined.
		// The increment of an invalid iterator is undefined.
		base_iterator& operator++() noexcept
		{
			value = value->next(value);
			return *this;
		}

		base_iterator operator++(int) noexcept
		{
			base_iterator res(*this);
			++(*this);
			return res;
		}

		// Go to the previous left by value.
		// Decrement of the begin_left() iterator is undefined.
		// Decrement of an invalid iterator is undefined.
		base_iterator& operator--() noexcept
		{
			value = value->prev(value);
			return *this;
		}

		base_iterator operator--(int) noexcept
		{
			base_iterator res(*this);
			--(*this);
			return res;
		}

		// left_iterator refers to the left element of some pair.
		// This function returns an iterator to the right element of the same pair.
		// end_left().flip() returns end_right().
		// end_right().flip() returns end_left().
		// flip() of an invalid iterator is undefined.
		base_iterator< Key, Value, !Tree > flip() const noexcept
		{
			if (value->parent)
			{
				return base_iterator< Key, Value, !Tree >(static_cast< base_t* >(
					static_cast< elem_another_t* >(static_cast< doub_t* >(static_cast< elem_t* >(value)))));
			}
			else
			{
				return base_iterator< Key, Value, !Tree >(value->right);
			}
		}

		bool operator==(const base_iterator& other) const noexcept { return value == other.value; }

		bool operator!=(const base_iterator& other) const noexcept { return value != other.value; }
	};
}	 // namespace bimap_details
