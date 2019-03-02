#include "Core/Reflection/RfmArray.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.RfmArray", RfmArray, RfmCompound)

RfmArray::RfmArray(const wchar_t* name)
:	RfmCompound(name)
{
}

}
