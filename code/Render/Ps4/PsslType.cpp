#include <sstream>
#include "Render/Ps4/PsslType.h"

namespace traktor
{
	namespace render
	{

std::wstring pssl_semantic(DataUsage usage, int index)
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

std::wstring pssl_type_name(PsslType type, bool lowPrecision)
{
	const wchar_t* c_full[] =
	{
		L"void",
		L"bool",
		L"float",
		L"float2",
		L"float3",
		L"float4",
		L"float4x4",
		L"Texture2D",
		L"Texture3D",
		L"TextureCube"
	};
	const wchar_t* c_low[] =
	{
		L"void",
		L"bool",
		L"half",
		L"half2",
		L"half3",
		L"half4",
		L"half4x4",
		L"Texture2D",
		L"Texture3D",
		L"TextureCube"
	};
	return (lowPrecision ? c_low : c_full)[type];
}

int32_t pssl_type_width(PsslType type)
{
	const int32_t c[] =
	{
		0,
		0,
		1,
		2,
		3,
		4,
		0,
		0,
		0,
		0
	};
	T_ASSERT (type < sizeof_array(c));
	return c[type];
}

PsslType pssl_from_data_type(DataType type)
{
	const PsslType c[] =
	{
		PsslFloat,
		PsslFloat2,
		PsslFloat3,
		PsslFloat4,
		PsslFloat4,
		PsslFloat4,
		PsslFloat2,
		PsslFloat4,
		PsslFloat2,
		PsslFloat4,
		PsslFloat2,
		PsslFloat4
	};
	T_ASSERT (type < sizeof_array(c));
	return c[type];
}

PsslType pssl_from_parameter_type(ParameterType type)
{
	const PsslType c[] =
	{
		PsslFloat,
		PsslFloat4,
		PsslFloat4x4,
		PsslTexture2D,
		PsslTexture3D,
		PsslTextureCube
	};
	T_ASSERT (type < sizeof_array(c));
	return c[type];
}

	}
}
