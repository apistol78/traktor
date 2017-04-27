/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_AssemblerARM_H
#define traktor_render_AssemblerARM_H

#include <vector>

namespace traktor
{
	namespace render
	{

class Register
{
public:
	Register(int index);

	int getIndex() const;

private:
	int m_index;
};

class AssemblerARM
{
public:
	void mov(const Register& dst, const Register& src);
};

	}
}

#endif	// traktor_render_AssemblerARM_H
