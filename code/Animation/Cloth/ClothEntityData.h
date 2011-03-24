#ifndef traktor_animation_ClothEntityData_H
#define traktor_animation_ClothEntityData_H

#include "Physics/PhysicsManager.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Proxy.h"
#include "World/Entity/SpatialEntityData.h"
#include "World/Entity/EntityBuilder.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class ClothEntity;

class T_DLLCLASS ClothEntityData : public world::SpatialEntityData
{
	T_RTTI_CLASS;

public:
	struct Anchor
	{
		uint32_t x;
		uint32_t y;

		bool serialize(ISerializer& s);
	};

	ClothEntityData();

	Ref< ClothEntity > createEntity(
		world::IEntityBuilder* builder,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		physics::PhysicsManager* physicsManager
	) const;

	virtual bool serialize(ISerializer& s);

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

private:
	mutable resource::Proxy< render::Shader > m_shader;
	uint32_t m_resolutionX;
	uint32_t m_resolutionY;
	float m_scale;
	std::vector< Anchor > m_anchors;
	uint32_t m_solverIterations;
	float m_damping;
};

	}
}

#endif	// traktor_animation_ClothEntityData_H
