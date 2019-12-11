#pragma once

#include <list>
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

/*! Sequence texture resource.
 * \ingroup Render
 */
class T_DLLCLASS SequenceTextureResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	SequenceTextureResource();

	virtual void serialize(ISerializer& s) override final;

private:
	friend class SequenceTexturePipeline;
	friend class SequenceTextureFactory;

	float m_rate;
	std::list< resource::Id< ITexture > > m_textures;
};

	}
}

