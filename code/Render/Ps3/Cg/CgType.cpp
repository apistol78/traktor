#include <sstream>
#include "Render/Ps3/Cg/CgType.h"

namespace traktor
{
	namespace render
	{

int32_t cg_attr_index(DataUsage usage, int index)
{
	const int32_t base[] =
	{
		0,	// DuPosition
		1,	// DuNormal
		2,	// DuTangent
		3,	// DuBinormal
		4,	// DuColor
		5,	// DuCustom
		16	// Last attribute index
	};
	int32_t attr = base[int(usage)] + index;
	T_ASSERT (attr < base[int(usage) + 1]);
	return attr;
}

std::wstring cg_semantic(DataUsage usage, int32_t index)
{
	std::wstringstream ss;
	ss << L"ATTR" << cg_attr_index(usage, index);
	return ss.str();
}

std::wstring cg_type_name(CgType type)
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
		L"texture",
		L"texture",
		L"texture"
	};
	T_ASSERT (type < sizeof_array(c));
	return c[type];
}

int32_t cg_type_width(CgType type)
{
	const int32_t c_widths[] =
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
	T_ASSERT (type < sizeof_array(c_widths));
	return c_widths[type];
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
		CtFloat4,
		CtFloat2,
		CtFloat4
	};
	T_ASSERT (type < sizeof_array(c));
	return c[type];
}

CgType cg_from_parameter_type(ParameterType type)
{
	const CgType c[] =
	{
		CtFloat,
		CtFloat4,
		CtFloat4x4,
		CtTexture2D,
		CtTexture3D,
		CtTextureCube
	};
	T_ASSERT (type < sizeof_array(c));
	return c[type];

}

	}
}
