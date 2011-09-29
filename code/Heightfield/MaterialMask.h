#ifndef traktor_hf_MaterialMask_H
#define traktor_hf_MaterialMask_H

#include "Core/Object.h"

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

class T_DLLCLASS MaterialMask : public Object
{
	T_RTTI_CLASS;

public:
	MaterialMask(uint32_t size);

	virtual ~MaterialMask();

	uint32_t getSize() const;

	uint8_t getMaterial(int x, int y) const;

private:
	friend class MaterialMaskFactory;

	uint32_t m_size;
	uint8_t* m_data;
};

	}
}

#endif	// traktor_hf_MaterialMask_H
