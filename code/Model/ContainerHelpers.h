#ifndef traktor_model_ContainerHelpers_H
#define traktor_model_ContainerHelpers_H

#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Model/Material.h"

namespace traktor
{
	namespace model
	{

//@{
// \ingroup Model

template < typename ContainerType, typename ItemType >
int addId(ContainerType& v, ItemType item)
{
	v.push_back(item);
	return int(v.size() - 1);
}

template < typename ContainerType, typename ItemType, typename ItemPredicate >
int addUniqueId(ContainerType& v, ItemType item, const ItemPredicate& predicate = ItemPredicate())
{
	for (int i = 0; i < int(v.size()); ++i)
	{
		if (predicate(v[i], item))
			return i;
	}
	return addId< ContainerType, ItemType >(v, item);
}

template < typename ItemType >
struct DefaultPredicate
{
	inline bool operator () (const ItemType& a, const ItemType& b) const
	{
		return a == b;
	}
};

struct MaterialNamePredicate
{
	inline bool operator () (const Material& a, const Material& b) const
	{
		return a.getName() == b.getName();
	}
};

struct PositionPredicate
{
	inline bool operator () (const Vector4& a, const Vector4& b) const
	{
		return (b - a).length() <= 0.01f;
	}
};

struct ColorPredicate
{
	inline bool operator () (const Vector4& a, const Vector4& b) const
	{
		return (b - a).length() <= 1.0f / (4.0f * 256.0f);
	}
};

struct NormalPredicate
{
	inline bool operator () (const Vector4& a, const Vector4& b) const
	{
		return (b - a).length() <= 0.0001f;
	}
};

struct TexCoordPredicate
{
	inline bool operator () (const Vector2& a, const Vector2& b) const
	{
		return (b - a).length() <= 0.001f;
	}
};

//@}

	}
}

#endif	// traktor_model_ContainerHelpers_H
