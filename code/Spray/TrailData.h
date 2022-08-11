#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
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

	namespace resource
	{

class IResourceManager;

	}

	namespace spray
	{

class Trail;

/*! Ribbon trail persistent data.
 * \ingroup Spray
 */
class TrailData : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< Trail > createTrail(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s) override final;

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Id< render::Shader > m_shader;
	float m_width = 1.0f;
	float m_age = 1.0f;
	float m_lengthThreshold = 1.0f;
	float m_breakThreshold = 0.0f;
};

	}
}

