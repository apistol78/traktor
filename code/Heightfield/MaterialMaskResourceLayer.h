#ifndef traktor_hf_MaterialMaskResourceLayer_H
#define traktor_hf_MaterialMaskResourceLayer_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace hf
	{

class T_DLLCLASS MaterialMaskResourceLayer : public ISerializable
{
	T_RTTI_CLASS;

public:
	const RefArray< ISerializable >& getParams() const;

	virtual void serialize(ISerializer& s);

private:
	friend class MaterialMaskPipeline;

	RefArray< ISerializable > m_params;
};

	}
}

#endif	// traktor_hf_MaterialMaskResourceLayer_H
