#pragma once

#include <type_traits>

namespace traktor
{

/*! Construct/destruct items policy.
 * \ingroup Core
 *
 * Default policy when constructing or destroying
 * items in containers.
 */
template < typename ItemType, bool pod >
struct VectorConstructorBase
{
};

template < typename ItemType >
struct VectorConstructorBase < ItemType, false >
{
	static void construct(ItemType& uninitialized)
	{
		new (&uninitialized) ItemType();
	}

	static void construct(ItemType& uninitialized, const ItemType& source)
	{
		new (&uninitialized) ItemType(source);
	}

	static void destroy(ItemType& item)
	{
		item.~ItemType();
	}

	static void move(ItemType& uninitialized, ItemType& source)
	{
		new (&uninitialized) ItemType(std::move(source));
	}
};

template < typename ItemType >
struct VectorConstructorBase < ItemType, true >
{
	static void construct(ItemType& uninitialized)
	{
	}

	static void construct(ItemType& uninitialized, const ItemType& source)
	{
		uninitialized = source;
	}

	static void destroy(ItemType& item)
	{
	}

	static void move(ItemType& uninitialized, ItemType& source)
	{
		uninitialized = std::move(source);
	}
};

template < typename ItemType >
struct VectorConstructor : public VectorConstructorBase< ItemType, std::is_trivial< ItemType >::value >
{
};

}
