#ifndef traktor_model_LwVMad_H
#define traktor_model_LwVMad_H

#include <map>
#include "Core/Object.h"

namespace traktor
{

class IStream;

	namespace model
	{

struct LwChunk;

class LwVMad : public Object
{
	T_RTTI_CLASS;

public:
	LwVMad();

	bool read(const LwChunk& chunk, IStream* stream);

private:
	uint32_t m_type;
	uint16_t m_dimension;
	std::wstring m_name;
	std::vector< float > m_values;
	std::map< uint32_t, uint32_t > m_vm;
};

	}
}

#endif	// traktor_model_LwVMad_H
