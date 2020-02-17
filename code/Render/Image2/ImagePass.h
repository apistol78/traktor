#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ImageStep;

/*!
 * \ingroup Render
 */
class T_DLLCLASS ImagePass : public Object
{
	T_RTTI_CLASS;

public:

private:
	friend class ImageGraph;
	friend class ImagePassData;

	std::wstring m_name;
	int32_t m_outputTargetSet;
	RefArray< const ImageStep > m_steps;
};

	}
}