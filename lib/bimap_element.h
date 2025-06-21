#pragma once

#include <memory>
#include <utility>

namespace bimap_details
{
	struct element_base
	{
		element_base* left = nullptr;
		element_base* right = nullptr;
		element_base* parent = nullptr;

		void relink_parent(element_base* other) noexcept
		{
			if (parent)
			{
				if (parent->right == this)
				{
					parent->right = other;
				}
				else
				{
					parent->left = other;
				}
			}
		}

		void relink_child() noexcept
		{
			if (left)
			{
				left->parent = this;
			}
			if (right)
			{
				right->parent = this;
			}
		}

		void relink_self(element_base& other) noexcept
		{
			std::swap(left, other.left);
			std::swap(right, other.right);
			std::swap(parent, other.parent);
		}

		void swap(element_base& other) noexcept
		{
			relink_parent(&other);
			other.relink_parent(this);
			relink_self(other);
			relink_child();
			other.relink_child();
		}

		element_base() noexcept = default;

		element_base(element_base&& other) noexcept { swap(other); }

		element_base& operator=(element_base&& other) noexcept
		{
			if (this != std::addressof(other))
			{
				element_base(std::move(other)).swap(*this);
			}
			return *this;
		}

		element_base* min(element_base* node) const noexcept
		{
			if (node->left)
			{
				return min(node->left);
			}
			else
			{
				return node;
			}
		}

		element_base* max(element_base* node) const noexcept
		{
			if (node->right)
			{
				return max(node->right);
			}
			else
			{
				return node;
			}
		}

		element_base* next(element_base* node) const noexcept
		{
			if (!node->right)
			{
				if (node->parent->left == node)
				{
					node = node->parent;
				}
				else
				{
					element_base* transport = node->parent;
					while (transport && transport->right == node)
					{
						node = transport;
						transport = transport->parent;
					}
					if (transport)
					{
						node = transport;
					}
				}
			}
			else
			{
				node = node->right;
				node = min(node);
			}
			return node;
		}

		element_base* prev(element_base* node) const noexcept
		{
			if (!node->left)
			{
				if (node->parent->right == node)
				{
					node = node->parent;
				}
				else
				{
					element_base* transport = node->parent;
					while (transport && transport->left == node)
					{
						node = transport;
						transport = transport->parent;
					}
					if (transport)
					{
						node = transport;
					}
				}
			}
			else
			{
				node = node->left;
				node = max(node);
			}
			return node;
		}
	};

	template< bool Tree, typename Storage >
	struct element_value : element_base
	{
		Storage storage;

		element_value(const Storage& storage) : storage(storage) {}
		element_value(Storage&& storage) noexcept : storage(std::move(storage)) {}

		const Storage& get() const noexcept { return storage; }
		Storage& get() noexcept { return storage; }
	};

	template< typename Key, typename Value >
	struct element_data : element_value< true, Key >, element_value< false, Value >
	{
		template< typename Key_f = Key, typename Value_f = Value >
		element_data(Key_f&& key, Value_f&& value) :
			element_value< true, Key >(std::forward< Key_f >(key)), element_value< false, Value >(std::forward< Value_f >(value))
		{
		}
	};
}	 // namespace bimap_details
