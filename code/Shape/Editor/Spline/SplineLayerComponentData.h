#pragma once

#include "Core/Ref.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
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

class SplineLayerComponent;

class T_DLLCLASS SplineLayerComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	virtual Ref< SplineLayerComponent > createComponent(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const = 0;
};

	}
}
