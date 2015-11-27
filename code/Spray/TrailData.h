#ifndef traktor_spray_TrailData_H
#define traktor_spray_TrailData_H

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

class TrailData : public ISerializable
{
	T_RTTI_CLASS;

public:
	TrailData();

	Ref< Trail > createTrail(resource::IResourceManager* resourceManager) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const resource::Id< render::Shader >& getShader() const { return m_shader; }

private:
	resource::Id< render::Shader > m_shader;
	float m_width;
	float m_age;
	float m_lengthThreshold;
	float m_breakThreshold;
};

	}
}

#endif	// traktor_spray_TrailData_H
