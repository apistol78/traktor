#ifndef traktor_hf_MaterialMaskResource_H
#define traktor_hf_MaterialMaskResource_H

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

class T_DLLCLASS MaterialMaskResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	MaterialMaskResource(uint32_t size = 0);

	uint32_t getSize() const;

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_size;
};

	}
}

#endif	// traktor_hf_MaterialMaskResource_H
