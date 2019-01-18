#pragma once

#include "Resource/Id.h"
#include "World/EntityData.h"

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

class ITexture;

	}

	namespace world
	{

/*! \brief Directional light entity data.
 * \ingroup World
 */
class T_DLLCLASS DirectionalLightEntityData : public EntityData
{
	T_RTTI_CLASS;

public:
	DirectionalLightEntityData();

	virtual void serialize(ISerializer& s) override final;

	const Vector4& getColor() const { return m_color; }

	bool getCastShadow() const { return m_castShadow; }

private:
	Vector4 m_color;
	bool m_castShadow;
};

	}
}
