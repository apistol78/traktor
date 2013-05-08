#ifndef traktor_terrain_OceanEntityData_H
#define traktor_terrain_OceanEntityData_H

#include "Core/Math/Vector2.h"
#include "Resource/Id.h"
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
	namespace render
	{

class IRenderSystem;
class Shader;

	}

	namespace terrain
	{

class OceanEntity;

class T_DLLCLASS OceanEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

	const resource::Id< render::Shader >& getShaderComposite() const { return m_shaderComposite; }

private:
	friend class OceanEntity;

	resource::Id< render::Shader > m_shaderComposite;
};

	}
}

#endif	// traktor_terrain_OceanEntityData_H
