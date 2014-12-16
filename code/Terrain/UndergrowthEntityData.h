#ifndef traktor_terrain_UndergrowthEntityData_H
#define traktor_terrain_UndergrowthEntityData_H

#include "Resource/Id.h"
#include "Terrain/UndergrowthEntity.h"
#include "World/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class MaterialMask;

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

	namespace terrain
	{

class Terrain;

/*! \brief Undergrowth entity data.
 * \ingroup Terrain
 */
class T_DLLCLASS UndergrowthEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	UndergrowthEntity* createEntity(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

	virtual void serialize(ISerializer& s);

	const resource::Id< Terrain >& getTerrain() const { return m_terrain; }

	const resource::Id< hf::MaterialMask >& getMaterialMask() const { return m_materialMask; }

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Id< Terrain > m_terrain;
	resource::Id< hf::MaterialMask > m_materialMask;
	resource::Id< render::Shader > m_shader;
	UndergrowthEntity::Settings m_settings;
};

	}
}

#endif	// traktor_terrain_UndergrowthEntityData_H
