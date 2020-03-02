#pragma once

#include "Core/Ref.h"
#include "Shape/Spline/LayerComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityData;

	}

	namespace shape
	{

/*!
 * \ingroup Shape
 */
class T_DLLCLASS CloneLayerData : public LayerComponentData
{
	T_RTTI_CLASS;

public:
	CloneLayerData();

	virtual Ref< LayerComponent > createComponent(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< world::EntityData > m_entity;
	float m_distance;
};

	}
}
