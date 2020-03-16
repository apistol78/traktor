#pragma once

#include "Core/Ref.h"
#include "Shape/Spline/SplineLayerComponentData.h"

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
class T_DLLCLASS CloneLayerData : public SplineLayerComponentData
{
	T_RTTI_CLASS;

public:
	CloneLayerData();

	virtual Ref< SplineLayerComponent > createComponent(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< world::EntityData > m_entity;
	float m_distance;
};

	}
}
