#ifndef traktor_terrain_RiverEntityData_H
#define traktor_terrain_RiverEntityData_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Resource/Proxy.h"
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

class Shader;

	}

	namespace terrain
	{

class T_DLLCLASS RiverEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	struct T_MATH_ALIGN16 ControlPoint
	{
		Vector4 position;
		float width;

		ControlPoint();

		bool serialize(ISerializer& s);
	};

	virtual bool serialize(ISerializer& s);

	const resource::Proxy< render::Shader >& getShader() const { return m_shader; }

	const AlignedVector< ControlPoint >& getPath() const { return m_path; }

private:
	resource::Proxy< render::Shader > m_shader;
	AlignedVector< ControlPoint > m_path;
};

	}
}

#endif	// traktor_terrain_RiverEntityData_H
