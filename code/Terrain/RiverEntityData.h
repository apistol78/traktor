#ifndef traktor_terrain_RiverEntityData_H
#define traktor_terrain_RiverEntityData_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
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

class Shader;

	}

	namespace terrain
	{

/*! \brief River entity data.
 * \ingroup Terrain
 */
class T_DLLCLASS RiverEntityData : public world::EntityData
{
	T_RTTI_CLASS;

public:
	struct T_MATH_ALIGN16 ControlPoint
	{
		Vector4 position;
		float width;
		float tension;

		ControlPoint();

		void serialize(ISerializer& s);
	};

	RiverEntityData();

	virtual void serialize(ISerializer& s);

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

	const AlignedVector< ControlPoint >& getPath() const { return m_path; }

	float getTileFactorV() const { return m_tileFactorV; }

private:
	resource::Id< render::Shader > m_shader;
	AlignedVector< ControlPoint > m_path;
	float m_tileFactorV;
};

	}
}

#endif	// traktor_terrain_RiverEntityData_H
