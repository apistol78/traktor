#ifndef traktor_terrain_OceanEntityData_H
#define traktor_terrain_OceanEntityData_H

#include "Core/Math/Vector2.h"
#include "Resource/Id.h"
#include "World/Entity/EntityData.h"

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
class Terrain;

class T_DLLCLASS OceanEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	enum { MaxWaves = 32 };

	struct Wave
	{
		Vector2 direction;
		float amplitude;
		float phase;

		Wave()
		:	direction(0.0f, 0.0f)
		,	amplitude(0.0f)
		,	phase(0.0f)
		{
		}

		bool serialize(ISerializer& s);
	};

	void setWave(int index, const Wave& wave);

	const Wave& getWave(int index) const;

	virtual bool serialize(ISerializer& s);

	const resource::Id< Terrain >& getTerrain() const { return m_terrain; }

	const resource::Id< render::Shader >& getShaderWaves() const { return m_shaderWaves; }

	const resource::Id< render::Shader >& getShaderComposite() const { return m_shaderComposite; }

private:
	friend class OceanEntity;

	resource::Id< Terrain > m_terrain;
	resource::Id< render::Shader > m_shaderWaves;
	resource::Id< render::Shader > m_shaderComposite;
	Wave m_waves[MaxWaves];
};

	}
}

#endif	// traktor_terrain_OceanEntityData_H
