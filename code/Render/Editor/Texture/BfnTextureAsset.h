#pragma once

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

	virtual void serialize(ISerializer& s) override final;

private:
	friend class BfnTexturePipeline;

	bool m_bestFitFactorOnly;
	bool m_collapseSymmetry;
	int32_t m_size;
};

	}
}

