#ifndef traktor_world_DecalEntityData_H
#define traktor_world_DecalEntityData_H

#include "Resource/Id.h"
#include "World/Entity/EntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace world
	{

/*! \brief Decal entity data.
 * \ingroup World
 */
class T_DLLCLASS DecalEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	DecalEntityData();

	virtual bool serialize(ISerializer& s);

	float getSize() const { return m_size; }

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	float m_size;
	resource::Id< render::Shader > m_shader;
};

	}
}

#endif	// traktor_world_DecalEntityData_H
