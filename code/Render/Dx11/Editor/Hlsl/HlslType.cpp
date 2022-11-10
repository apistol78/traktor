/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Render/Dx11/Editor/Hlsl/HlslType.h"

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
	return str(L"%ls%d", s[(int)usage], index);
}

std::wstring hlsl_type_name(HlslType type)
{
	const wchar_t* c[] =
	{
		L"void",
		L"bool",
		L"int",
		L"int2",
		L"int3",
		L"int4",
		L"float",
		L"float2",
		L"float3",
		L"float4",
		L"float4x4",
		L"Texture2D",
		L"Texture3D",
		L"TextureCube"
	};
	T_FATAL_ASSERT(type < sizeof_array(c));
	return c[type];
}

int32_t hlsl_type_width(HlslType type)
{
	const int32_t w[] =
	{
		0,	// HtVoid
		0,	// HtBoolean
		1,	// HtInteger
		2,	// HtInteger2
		3,	// HtInteger3
		4,	// HtInteger4
		1,	// HtFloat
		2,	// HtFloat2
		3,	// HtFloat3
		4,	// HtFloat4
		16,	// HtFloat4x4
		0,	// HtTexture2D
		0,	// HtTexture3D
		0,	// HtTextureCube
		0	// HtStructBuffer
	};
	T_ASSERT(type < sizeof_array(w));
	return w[type];
}

HlslType hlsl_promote_to_float(HlslType type)
{
	const HlslType w[] =
	{
		HtVoid,		// HtVoid
		HtFloat,	// HtBoolean
		HtFloat,	// HtInteger
		HtFloat2,	// HtInteger2
		HtFloat3,	// HtInteger3
		HtFloat4,	// HtInteger4
		HtFloat,	// HtFloat
		HtFloat2,	// HtFloat2
		HtFloat3,	// HtFloat3
		HtFloat4,	// HtFloat4
		HtFloat4x4,	// HtFloat4x4
		HtVoid,		// HtTexture2D
		HtVoid,		// HtTexture3D
		HtVoid,		// HtTextureCube
		HtVoid		// HtStructBuffer
	};
	T_FATAL_ASSERT(type < sizeof_array(w));
	return w[type];
}

HlslType hlsl_degrade_to_integer(HlslType type)
{
	const HlslType w[] =
	{
		HtVoid,		// HtVoid
		HtInteger,	// HtBoolean
		HtInteger,	// HtInteger
		HtInteger2,	// HtInteger2
		HtInteger3,	// HtInteger3
		HtInteger4,	// HtInteger4
		HtInteger,	// HtFloat
		HtInteger2,	// HtFloat2
		HtInteger3,	// HtFloat3
		HtInteger4,	// HtFloat4
		HtVoid,		// HtFloat4x4
		HtVoid,		// HtTexture2D
		HtVoid,		// HtTexture3D
		HtVoid,		// HtTextureCube
		HtVoid		// HtStructBuffer
	};
	T_FATAL_ASSERT(type < sizeof_array(w));
	return w[type];	
}

HlslType hlsl_precedence(HlslType typeA, HlslType typeB)
{
	bool intA = (typeA >= HtBoolean && typeA <= HtInteger4);
	bool intB = (typeB >= HtBoolean && typeB <= HtInteger4);

	if (intA && !intB)
		typeA = hlsl_promote_to_float(typeA);
	if (!intA && intB)
		typeB = hlsl_promote_to_float(typeB);

	return std::max(typeA, typeB);
}

HlslType hlsl_from_data_type(DataType type)
{
	const HlslType c[] =
	{
		HtFloat,	// DtFloat1
		HtFloat2,	// DtFloat2
		HtFloat3,	// DtFloat3
		HtFloat4,	// DtFloat4
		HtInteger4,	// DtByte4
		HtFloat4,	// DtByte4N
		HtInteger2,	// DtShort2
		HtInteger4,	// DtShort4
		HtFloat2,	// DtShort2N
		HtFloat4,	// DtShort4N
		HtFloat2,	// DtHalf2
		HtFloat4,	// DtHalf4,
		HtInteger,	// DtInteger1
		HtInteger2,	// DtInteger2
		HtInteger3,	// DtInteger3
		HtInteger4	// DtInteger4
	};
	T_ASSERT(type < sizeof_array(c));
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
		HtTextureCube,
		HtStructBuffer
	};
	T_ASSERT((int32_t)type < sizeof_array(c));
	return c[(int32_t)type];
}

std::wstring hlsl_storage_type(DataType type)
{
	const wchar_t* c[] =
	{
		L"float",
		L"float2",
		L"float3",
		L"float4",
		L"",
		L"",
		L"short2",
		L"short4",
		L"",
		L"",
		L"uint",	// Two packed 16 bit.
		L"uint2",	// Four packed 16 bit.
		L"int",
		L"int2",
		L"int3",
		L"int4"
	};
	T_FATAL_ASSERT(type < sizeof_array(c));
	return c[type];
}

	}
}
