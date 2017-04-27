/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <sstream>
#include "Render/Dx11/Hlsl/HlslType.h"

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

std::wstring hlsl_type_name(HlslType type, bool lowPrecision)
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

int32_t hlsl_type_width(HlslType type)
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
	T_ASSERT (type < sizeof_array(c));
	return c[type];
}

HlslType hlsl_from_parameter_type(ParameterType type)
{
	const HlslType c[] =
	{
		HtFloat,
		HtFloat4,
		HtFloat4x4,
		HtTexture2D,
		HtTexture3D,
		HtTextureCube
	};
	T_ASSERT (type < sizeof_array(c));
	return c[type];
}

	}
}
