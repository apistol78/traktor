#pragma once

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_VIDEO_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace video
	{

/*! \brief
 * \ingroup Video
 */
class T_DLLCLASS VideoAsset : public editor::Asset
{
	T_RTTI_CLASS;
};

	}
}
