#ifndef traktor_render_PsslType_H
#define traktor_render_PsslType_H

#include <string>
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup GNM
 */
enum PsslType
{
	PsslVoid,
	PsslBoolean,
	PsslFloat,
	PsslFloat2,
	PsslFloat3,
	PsslFloat4,
	PsslFloat4x4,
	PsslTexture2D,
	PsslTexture3D,
	PsslTextureCube
};

/*!
 * \ingroup GNM
 */
std::wstring pssl_semantic(DataUsage usage, int index);

/*!
 * \ingroup GNM
 */
std::wstring pssl_type_name(PsslType type, bool lowPrecision);

/*!
 * \ingroup GNM
 */
int32_t pssl_type_width(PsslType type);

/*!
 * \ingroup GNM
 */
PsslType pssl_from_data_type(DataType type);

/*!
 * \ingroup GNM
 */
PsslType pssl_from_parameter_type(ParameterType type);

	}
}

#endif	// traktor_render_PsslType_H
