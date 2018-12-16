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

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class ProbePipeline;

	int32_t m_glossScale;
	int32_t m_glossBias;
	int32_t m_sizeDivisor;
};

	}
}
