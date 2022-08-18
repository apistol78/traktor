#pragma once

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
int32_t addId(ContainerType& v, ItemType item)
{
	v.push_back(item);
	return (int32_t)(v.size() - 1);
}

template < typename ContainerType, typename ItemType, typename ItemPredicate >
int32_t addUniqueId(ContainerType& v, ItemType item, const ItemPredicate& predicate = ItemPredicate())
{
	for (int32_t i = 0; i < (int32_t)v.size(); ++i)
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

struct MaterialPredicate
{
	inline static bool compare(float a, float b)
	{
		return std::abs(a - b) <= 1e-4f;
	}

	inline bool operator () (const Material& a, const Material& b) const
	{
		return
			a.getName() == b.getName() &&
			a.getDiffuseMap().name == b.getDiffuseMap().name &&
			a.getDiffuseMap().channel == b.getDiffuseMap().channel &&
			a.getSpecularMap().name == b.getSpecularMap().name &&
			a.getSpecularMap().channel == b.getSpecularMap().channel &&
			a.getRoughnessMap().name == b.getRoughnessMap().name &&
			a.getRoughnessMap().channel == b.getRoughnessMap().channel &&
			a.getMetalnessMap().name == b.getMetalnessMap().name &&
			a.getMetalnessMap().channel == b.getMetalnessMap().channel &&
			a.getTransparencyMap().name == b.getTransparencyMap().name &&
			a.getTransparencyMap().channel == b.getTransparencyMap().channel &&
			a.getEmissiveMap().name == b.getEmissiveMap().name &&
			a.getEmissiveMap().channel == b.getEmissiveMap().channel &&
			a.getReflectiveMap().name == b.getReflectiveMap().name &&
			a.getReflectiveMap().channel == b.getReflectiveMap().channel &&
			a.getNormalMap().name == b.getNormalMap().name &&
			a.getNormalMap().channel == b.getNormalMap().channel &&
			a.getColor() == b.getColor() &&
			compare(a.getDiffuseTerm(), b.getDiffuseTerm()) &&
			compare(a.getSpecularTerm(), b.getSpecularTerm()) &&
			compare(a.getRoughness(), b.getRoughness()) &&
			compare(a.getTransparency(), b.getTransparency()) &&
			compare(a.getEmissive(), b.getEmissive()) &&
			compare(a.getReflective(), b.getReflective()) &&
			a.getBlendOperator() == b.getBlendOperator() &&
			a.isDoubleSided() == b.isDoubleSided();
	}
};

//@}

	}
}

