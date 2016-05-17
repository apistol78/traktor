#ifndef traktor_hf_OcclusionLayerAttribute_H
#define traktor_hf_OcclusionLayerAttribute_H

#include "World/Editor/ILayerAttribute.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class T_DLLCLASS OcclusionLayerAttribute : public world::ILayerAttribute
{
	T_RTTI_CLASS;

public:
	OcclusionLayerAttribute();

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	bool trace() const { return m_trace; }

private:
	bool m_trace;
};

	}
}

#endif	// traktor_hf_OcclusionLayerAttribute_H
