#ifndef traktor_animation_ClothEntityData_H
#define traktor_animation_ClothEntityData_H

#include "Resource/Id.h"
#include "World/Entity/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class PhysicsManager;

	}

	namespace render
	{

class IRenderSystem;
class Shader;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class IEntityBuilder;

	}

	namespace animation
	{

class ClothEntity;

class T_DLLCLASS ClothEntityData : public world::EntityData
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

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Id< render::Shader > m_shader;
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
