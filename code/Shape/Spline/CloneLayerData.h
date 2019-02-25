#pragma once

#include "Core/Ref.h"
#include "Shape/Spline/ISplineLayerData.h"

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

/*! \brief
 * \ingroup Shape
 */
class T_DLLCLASS CloneLayerData : public ISplineLayerData
{
	T_RTTI_CLASS;

public:
	CloneLayerData();
	
	virtual Ref< ISplineLayer > createLayer(SplineEntity* owner, const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< world::EntityData > m_entity;
	float m_distance;
};

	}
}
