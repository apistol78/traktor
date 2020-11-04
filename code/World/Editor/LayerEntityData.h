#pragma once

#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class ILayerAttribute;

/*! Layer entity data.
 * \ingroup World
 *
 * Used by editor to store editing layer information
 * in a scene asset.
 */
class T_DLLCLASS LayerEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	LayerEntityData();

	void setAttribute(const ILayerAttribute* attribute);

	const ILayerAttribute* getAttribute(const TypeInfo& attributeType) const;

	template < typename AttributeType >
	const AttributeType* getAttribute() const
	{
		return checked_type_cast< const AttributeType*, true >(getAttribute(type_of< AttributeType >()));
	}

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< const ILayerAttribute > m_attributes;
};

	}
}

