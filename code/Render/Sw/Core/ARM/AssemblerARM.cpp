#include "Render/Sw/Core/ARM/AssemblerARM.h"

namespace traktor
{
	namespace render
	{

Register::Register(int index)
:	m_index(index)
{
}

int Register::getIndex() const
{
	return m_index;
}

void AssemblerARM::mov(const Register& dst, const Register& src)
{
}

	}
}
