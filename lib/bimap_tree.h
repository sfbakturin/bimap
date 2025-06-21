#pragma once

#include "bimap_comparator.h"
#include "bimap_element.h"

#include <utility>

namespace bimap_details
{
	template< typename Key, bool Tree, typename Comparator >
	struct tree : comparator< Key, Tree, Comparator >
	{
	  private:
		using key_t = Key;
		using base_t = element_base;

		mutable base_t root;

		void zig(element_base* child) const noexcept
		{
			element_base* parent = child->parent;
			if (parent->left == child)
			{
				element_base* child_right = child->right;
				child->right = parent;
				parent->parent = child;
				parent->left = child_right;
				if (child_right)
				{
					child_right->parent = parent;
				}
			}
			else
			{
				element_base* child_left = child->left;
				child->left = parent;
				parent->parent = child;
				parent->right = child_left;
				if (child_left)
				{
					child_left->parent = parent;
				}
			}
			child->parent = nullptr;
		}

		void zig_zig(element_base* child) const noexcept
		{
			element_base* parent = child->parent;
			element_base* grand_parent = parent->parent;
			if (grand_parent->left == parent && parent->left == child)
			{
				element_base* child_right = child->right;
				element_base* parent_child = parent->right;
				child->parent = grand_parent->parent;
				if (child->parent)
				{
					if (child->parent->right == grand_parent)
					{
						child->parent->right = child;
					}
					else
					{
						child->parent->left = child;
					}
				}
				grand_parent->parent = parent;
				parent->right = grand_parent;
				parent->parent = child;
				child->right = parent;
				parent->left = child_right;
				grand_parent->left = parent_child;
				if (child_right)
				{
					child_right->parent = parent;
				}
				if (parent_child)
				{
					parent_child->parent = grand_parent;
				}
			}
			else
			{
				element_base* child_parent = parent->left;
				element_base* child_left = child->left;
				child->parent = grand_parent->parent;
				if (child->parent)
				{
					if (child->parent->right == grand_parent)
					{
						child->parent->right = child;
					}
					else
					{
						child->parent->left = child;
					}
				}
				grand_parent->parent = parent;
				parent->left = grand_parent;
				parent->parent = child;
				child->left = parent;
				parent->right = child_left;
				grand_parent->right = child_parent;
				if (child_parent)
				{
					child_parent->parent = grand_parent;
				}
				if (child_left)
				{
					child_left->parent = parent;
				}
			}
		}

		void zig_zag(element_base* child) const noexcept
		{
			element_base* parent = child->parent;
			element_base* grand_parent = parent->parent;
			if (grand_parent->left == parent && parent->right == child)
			{
				element_base* child_left = child->left;
				element_base* child_right = child->right;
				child->parent = grand_parent->parent;
				if (child->parent)
				{
					if (child->parent->right == grand_parent)
					{
						child->parent->right = child;
					}
					else
					{
						child->parent->left = child;
					}
				}
				child->right = grand_parent;
				child->left = parent;
				grand_parent->parent = child;
				parent->parent = child;
				parent->right = child_left;
				grand_parent->left = child_right;
				if (child_left)
				{
					child_left->parent = parent;
				}
				if (child_right)
				{
					child_right->parent = grand_parent;
				}
			}
			else
			{
				element_base* child_left = child->left;
				element_base* child_right = child->right;
				child->parent = grand_parent->parent;
				if (child->parent)
				{
					if (child->parent->right == grand_parent)
					{
						child->parent->right = child;
					}
					else
					{
						child->parent->left = child;
					}
				}
				child->left = grand_parent;
				child->right = parent;
				grand_parent->parent = child;
				parent->parent = child;
				grand_parent->right = child_left;
				parent->left = child_right;
				if (child_left)
				{
					child_left->parent = grand_parent;
				}
				if (child_right)
				{
					child_right->parent = parent;
				}
			}
		}

		element_base* splay_impl(element_base* child) const noexcept
		{
			while (child->parent)
			{
				element_base* parent = child->parent;
				element_base* grand_parent = parent->parent;
				if (!grand_parent)
				{
					zig(child);
				}
				else if ((grand_parent->left == parent && parent->left == child) ||
						 (grand_parent->right == parent && parent->right == child))
				{
					zig_zig(child);
				}
				else
				{
					zig_zag(child);
				}
			}
			return child;
		}

		element_base* merge(element_base* left, element_base* right) const noexcept
		{
			if (!left && !right)
				return nullptr;
			if (left && !right)
			{
				left->parent = nullptr;
				return left;
			}
			if (!left)
			{
				right->parent = nullptr;
				return right;
			}
			left->parent = nullptr;
			left = splay_impl(left->max(left));
			left->right = right;
			right->parent = left;
			return left;
		}

		void splay(base_t* node) const noexcept
		{
			root.left->parent = nullptr;
			root.left = splay_impl(node);
			if (root.left)
			{
				root.left->parent = &root;
			}
		}

	  public:
		void swap(tree& other) noexcept
		{
			std::swap(root, other.root);

			if (root.left)
			{
				root.left->parent = &root;
			}

			if (other.root.left)
			{
				other.root.left->parent = &(other.root);
			}

			std::swap(static_cast< comparator< Key, Tree, Comparator >& >(*this),
					  static_cast< comparator< Key, Tree, Comparator >& >(other));
		}

		tree(const comparator< Key, Tree, Comparator >& cmp) : comparator< Key, Tree, Comparator >(cmp) {}

		tree(comparator< Key, Tree, Comparator >&& cmp) noexcept : comparator< Key, Tree, Comparator >(std::move(cmp))
		{
		}

		tree(Comparator&& cmp) noexcept : comparator< Key, Tree, Comparator >(std::move(cmp)) {}

		tree(tree&& other) noexcept :
			comparator< Key, Tree, Comparator >(std::move(static_cast< comparator< Key, Tree, Comparator >&& >(other))),
			root(std::move(other.root))

		{
		}

		base_t* begin() const noexcept
		{
			if (root.left)
			{
				splay(root.min(root.left));
				return root.left;
			}
			else
			{
				return &root;
			}
		}

		base_t* end() const noexcept { return &root; }

		void set_another_tree(base_t* another_tree) noexcept { root.right = another_tree; }

		base_t* find(const key_t& to_find, bool flag = true) const noexcept
		{
			base_t* transfer_prev = &root;
			base_t* transfer = root.left;

			while (transfer)
			{
				transfer_prev = transfer;
				if (comparator< Key, Tree, Comparator >::operator()(to_find, transfer))
				{
					transfer = transfer->left;
				}
				else if (comparator< Key, Tree, Comparator >::operator()(transfer, to_find))
				{
					transfer = transfer->right;
				}
				else
				{
					splay(transfer);
					return transfer;
				}
			}

			return (flag ? nullptr : transfer_prev);
		}

		base_t* insert(base_t* inserted)
		{
			base_t* transfer_parent = &root;
			base_t* transfer = root.left;

			while (transfer)
			{
				transfer_parent = transfer;
				if (comparator< Key, Tree, Comparator >::operator()(inserted, transfer))
				{
					transfer = transfer->left;
				}
				else
				{
					transfer = transfer->right;
				}
			}

			inserted->parent = transfer_parent;

			if (transfer_parent == &root)
			{
				root.left = inserted;
			}
			else if (comparator< Key, Tree, Comparator >::operator()(transfer_parent, inserted))
			{
				transfer_parent->right = inserted;
			}
			else
			{
				transfer_parent->left = inserted;
			}

			splay(inserted);

			return inserted;
		}

		base_t* next(const Key& value) const noexcept
		{
			base_t* found = find(value, false);

			if (found == end() ||
				(!comparator< Key, Tree, Comparator >::operator()(value, found) &&
				 !comparator< Key, Tree, Comparator >::operator()(found, value)) ||
				comparator< Key, Tree, Comparator >::operator()(value, found))
			{
				return found;
			}

			return found->next(found);
		}

		base_t* prev(const Key& value) const noexcept
		{
			base_t* found = find(value, false);

			if (found == end() || comparator< Key, Tree, Comparator >::operator()(value, found))
			{
				return found;
			}

			return found->next(found);
		}

		void erase(base_t* node) noexcept
		{
			splay(node);
			root.left = merge(node->left, node->right);
			node->left = nullptr;
			node->right = nullptr;

			if (root.left)
			{
				root.left->parent = &root;
			}
		}

		bool is_equals(const key_t& a, const key_t& b) const noexcept
		{
			return !comparator< Key, Tree, Comparator >::operator()(a, b) && !comparator< Key, Tree, Comparator >::operator()(b, a);
		}

		const comparator< Key, Tree, Comparator >& get_comparator() const noexcept
		{
			return static_cast< comparator< Key, Tree, Comparator > const & >(*this);
		}
	};
}	 // namespace bimap_details
