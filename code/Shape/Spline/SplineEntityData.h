#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "World/Entity/ComponentEntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;

	}

	namespace world
	{

class IEntityBuilder;

	}

	namespace shape
	{

class ControlPointEntityData;
class ISplineLayerData;
class SplineEntity;

/*! \brief
 * \ingroup Shape
 */
class T_DLLCLASS SplineEntityData : public world::ComponentEntityData
{
	T_RTTI_CLASS;

public:
	Ref< SplineEntity > createEntity(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	void addEntityData(ControlPointEntityData* controlPointEntity);

	virtual void setTransform(const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const RefArray< ControlPointEntityData >& getControlPointEntities() const { return m_controlPointEntities; }

	const RefArray< ISplineLayerData >& getLayers() const { return m_layers; }

private:
	RefArray< ControlPointEntityData > m_controlPointEntities;
	RefArray< ISplineLayerData > m_layers;
};

	}
}
