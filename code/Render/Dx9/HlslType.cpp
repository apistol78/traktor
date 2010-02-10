#include <sstream>
#include "Render/Dx9/HlslType.h"

namespace traktor
{
	namespace render
	{

std::wstring hlsl_semantic(DataUsage usage, int index)
{
	const wchar_t* s[] =
	{
		L"POSITION",
		L"NORMAL",
		L"TANGENT",
		L"BINORMAL",
		L"COLOR",
		L"TEXCOORD",
		L""
	};
	std::wstringstream ss;
	ss << s[usage] << index;
	return ss.str();
}

std::wstring hlsl_type_name(HlslType type)
{
	const wchar_t* c[] =
	{
		L"void",
		L"bool",
		L"float",
		L"float2",
		L"float3",
		L"float4",
		L"float4x4",
		L"texture"
	};
	return c[type];
}

HlslType hlsl_from_data_type(DataType type)
{
	const HlslType c[] =
	{
		HtFloat,
		HtFloat2,
		HtFloat3,
		HtFloat4,
		HtFloat4,
		HtFloat4,
		HtFloat2,
		HtFloat4,
		HtFloat2,
		HtFloat4,
		HtFloat2,
		HtFloat4
	};
	return c[type];
}

	}
}
