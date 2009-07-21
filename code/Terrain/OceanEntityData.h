#ifndef traktor_terrain_OceanEntityData_H
#define traktor_terrain_OceanEntityData_H

#include "World/Entity/EntityData.h"
#include "Resource/Proxy.h"
#include "Core/Math/Vector2.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
class Heightfield;

class T_DLLCLASS OceanEntityData : public world::EntityData
{
	T_RTTI_CLASS(OceanEntityData)

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

		bool serialize(Serializer& s);
	};

	OceanEntityData();

	void setWave(int index, const Wave& wave);

	const Wave& getWave(int index) const;

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< Heightfield >& getHeightfield() const { return m_heightfield; }

	inline const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

private:
	friend class OceanEntity;

	resource::Proxy< Heightfield > m_heightfield;
	resource::Proxy< render::Shader > m_shader;
	float m_altitude;
	Wave m_waves[MaxWaves];
};

	}
}

#endif	// traktor_terrain_OceanEntityData_H
