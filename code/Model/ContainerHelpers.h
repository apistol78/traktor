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

struct MaterialPredicate
{
	inline static bool compare(float a, float b)
	{
		return std::abs(a - b) <= 1e-4f;
	}

	inline bool operator () (const Material& a, const Material& b) const
	{
		return
			a.getDiffuseMap().name == b.getDiffuseMap().name &&
			a.getDiffuseMap().channel == b.getDiffuseMap().channel &&
			a.getSpecularMap().name == b.getSpecularMap().name &&
			a.getSpecularMap().channel == b.getSpecularMap().channel &&
			a.getEmissiveMap().name == b.getEmissiveMap().name &&
			a.getEmissiveMap().channel == b.getEmissiveMap().channel &&
			a.getReflectiveMap().name == b.getReflectiveMap().name &&
			a.getReflectiveMap().channel == b.getReflectiveMap().channel &&
			a.getNormalMap().name == b.getNormalMap().name &&
			a.getNormalMap().channel == b.getNormalMap().channel &&
			a.getColor() == b.getColor() &&
			compare(a.getDiffuseTerm(), b.getDiffuseTerm()) &&
			compare(a.getSpecularTerm(), b.getSpecularTerm()) &&
			compare(a.getSpecularRoughness(), b.getSpecularRoughness()) &&
			compare(a.getEmissive(), b.getEmissive()) &&
			compare(a.getReflective(), b.getReflective()) &&
			compare(a.getRimLightIntensity(), b.getRimLightIntensity()) &&
			a.getBlendOperator() == b.getBlendOperator() &&
			a.isDoubleSided() == b.isDoubleSided();
	}
};

//@}

	}
}

#endif	// traktor_model_ContainerHelpers_H
