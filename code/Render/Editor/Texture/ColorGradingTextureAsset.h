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

class T_DLLCLASS ColorGradingTextureAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	ColorGradingTextureAsset();

	float getGamma() const { return m_gamma; }

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	float m_gamma;
};

	}
}
