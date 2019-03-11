#pragma once

#include "Editor/Asset.h"

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

class T_DLLCLASS RadianceProbeAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	RadianceProbeAsset();

	int32_t getGlossScale() const { return m_glossScale; }

	int32_t getGlossBias() const { return m_glossBias; }

	virtual void serialize(ISerializer& s) override final;

private:
	int32_t m_glossScale;
	int32_t m_glossBias;
};

	}
}
