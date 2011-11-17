#ifndef traktor_hf_MaterialMask_H
#define traktor_hf_MaterialMask_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Misc/AutoPtr.h"

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

class MaterialParams;

class T_DLLCLASS MaterialMask : public Object
{
	T_RTTI_CLASS;

public:
	MaterialMask(uint32_t size, const RefArray< MaterialParams >& params);

	uint32_t getSize() const;

	uint8_t getId(int x, int y) const;

	const MaterialParams* getParams(uint8_t id) const;

	const MaterialParams* getParams(int x, int y) const;

private:
	friend class MaterialMaskFactory;

	uint32_t m_size;
	AutoArrayPtr< uint8_t > m_data;
	RefArray< MaterialParams > m_params;
};

	}
}

#endif	// traktor_hf_MaterialMask_H
