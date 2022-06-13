#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

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

/*! River component data.
 * \ingroup Terrain
 */
class T_DLLCLASS RiverComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	struct T_MATH_ALIGN16 ControlPoint
	{
		Vector4 position = Vector4::zero();
		float width = 1.0f;
		float tension = 1.0f;

		void serialize(ISerializer& s);
	};

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

	const AlignedVector< ControlPoint >& getPath() const { return m_path; }

	float getTileFactorV() const { return m_tileFactorV; }

private:
	resource::Id< render::Shader > m_shader;
	AlignedVector< ControlPoint > m_path;
	float m_tileFactorV = 1.0f;
};

	}
}

