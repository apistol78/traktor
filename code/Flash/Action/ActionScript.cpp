#include "Flash/Action/ActionScript.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionScript", ActionScript, Object)

ActionScript::ActionScript(uint32_t codeSize)
:	m_code(new uint8_t [codeSize])
,	m_codeSize(codeSize)
{
}

ActionScript::~ActionScript()
{
	delete[] m_code;
}

uint8_t* ActionScript::getCode() const
{
	return m_code;
}

uint32_t ActionScript::getCodeSize() const
{
	return m_codeSize;
}

	}
}
