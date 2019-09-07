#pragma once

#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace shape
	{
	
class T_DLLCLASS SolidMaterial : public ISerializable
{
	T_RTTI_CLASS;

public:
	const Guid& getAlbedo() const { return m_albedo; }

	const Guid& getNormal() const { return m_normal; }

	const Guid& getRoughness() const { return m_roughness; }

	const Guid& getMetalness() const { return m_metalness; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_albedo;
	Guid m_normal;
	Guid m_roughness;
	Guid m_metalness;
};

	}
}
