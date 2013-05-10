#ifndef traktor_render_BfnTextureAsset_H
#define traktor_render_BfnTextureAsset_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS BfnTextureAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	BfnTextureAsset();

	virtual void serialize(ISerializer& s);

private:
	friend class BfnTexturePipeline;

	bool m_bestFitFactorOnly;
};

	}
}

#endif	// traktor_render_BfnTextureAsset_H
