#include <sstream>
#include "Render/Ps3/CgType.h"

namespace traktor
{
	namespace render
	{

std::wstring cg_channel_name(DataUsage usage, int index)
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
	const char* c[] =
	{
		"void",
		"float",
		"float2",
		"float3",
		"float4",
		"float4x4"
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
