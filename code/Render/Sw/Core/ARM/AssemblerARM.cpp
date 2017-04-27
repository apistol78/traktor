/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
