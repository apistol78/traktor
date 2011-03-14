#ifndef traktor_render_HlslType_H
#define traktor_render_HlslType_H

#include <string>
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup DX11
 */
enum HlslType
{
	HtVoid,
	HtBoolean,
	HtFloat,
	HtFloat2,
	HtFloat3,
	HtFloat4,
	HtFloat4x4,
	HtTexture
};

/*!
 * \ingroup DX11
 */
std::wstring hlsl_semantic(DataUsage usage, int index);

/*!
 * \ingroup DX11
 */
std::wstring hlsl_type_name(HlslType type);

/*!
 * \ingroup DX11
 */
HlslType hlsl_from_data_type(DataType type);

	}
}

#endif	// traktor_render_HlslType_H
