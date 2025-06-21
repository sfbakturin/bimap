#pragma once

#include "bimap_element.h"

#include <memory>
#include <utility>

namespace bimap_details
{
	template< typename Key, bool Tree, typename Comparator >
	struct comparator : Comparator
	{
		using key_t = Key;
		using base_t = element_base;
		using data_t = element_value< Tree, key_t >;

		void swap(comparator& other) noexcept
		{
			std::swap(static_cast< Comparator& >(*this), static_cast< Comparator& >(other));
		}

		const key_t& get_storage(base_t* node) const noexcept { return static_cast< data_t* >(node)->get(); }

		comparator(const comparator& cmp) : Comparator(static_cast< Comparator const & >(cmp)) {}

		comparator(comparator&& cmp) noexcept : Comparator(std::move(static_cast< Comparator&& >(cmp))) {}

		comparator(const Comparator& cmp) : Comparator(cmp) {}

		comparator(Comparator&& cmp) noexcept : Comparator(std::move(cmp)) {}

		comparator& operator=(comparator&& other) noexcept
		{
			if (this != std::addressof(other))
			{
				comparator(std::move(other)).swap(*this);
			}
			return *this;
		}

		bool operator()(base_t* left, base_t* right) const noexcept
		{
			return this->operator()(get_storage(left), get_storage(right));
		}

		bool operator()(base_t* left, const key_t& right) const noexcept
		{
			return this->operator()(get_storage(left), right);
		}

		bool operator()(const key_t& left, base_t* right) const noexcept
		{
			return this->operator()(left, get_storage(right));
		}

		bool operator()(const key_t& left, const key_t& right) const noexcept
		{
			return Comparator::operator()(left, right);
		}
	};
}	 // namespace bimap_details
