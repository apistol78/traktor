#include <sstream>
#include "Render/Ps3/CgType.h"

namespace traktor
{
	namespace render
	{

std::wstring cg_semantic(DataUsage usage, int index)
{
	const char* s[] =
	{
		"Position",
		"Normal",
		"Tangent",
		"Binormal",
		"Color",
		"Custom"
	};
	std::wstringstream ss;
	ss << s[usage] << index;
	return ss.str();
}

std::wstring cg_type_name(CgType type)
{
	const wchar_t* c[] =
	{
		L"void",
		L"float",
		L"float2",
		L"float3",
		L"float4",
		L"float4x4"
	};
	return c[type];
}

CgType cg_from_data_type(DataType type)
{
	const CgType c[] =
	{
		CtFloat,
		CtFloat2,
		CtFloat3,
		CtFloat4,
		CtFloat4,
		CtFloat4,
		CtFloat2,
		CtFloat4,
		CtFloat2,
		CtFloat4
	};
	return c[type];
}

	}
}
