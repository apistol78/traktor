#pragma once

#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ITexture;

/*! Alias texture resource.
 * \ingroup Render
 */
class T_DLLCLASS AliasTextureResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	AliasTextureResource() = default;

	explicit AliasTextureResource(const resource::Id< ITexture >& texture);

	virtual void serialize(ISerializer& s) override final;

private:
	friend class AliasTextureFactory;

	resource::Id< ITexture > m_texture;
};

	}
}
