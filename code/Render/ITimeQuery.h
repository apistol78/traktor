#pragma once

#include "Core/Object.h"

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

/*! \brief GPU time stamp recording query.
 * \ingroup Render
 */
class T_DLLCLASS ITimeQuery : public Object
{
	T_RTTI_CLASS;

public:
	/*! Begin recording stamps. */
	virtual void begin() = 0;

	/*! Insert a "stamp" into command buffer, return opaque handle of pending stamp. */
	virtual int32_t stamp() = 0;

	/*! End recording stamps. */
	virtual void end() = 0;

	/*! Check if stamps has been recorded and thus safe to access using get(stamp). */
	virtual bool ready() const = 0;

	/*! Get result of "stamp" by opaque handle. */
	virtual uint64_t get(int32_t index) const = 0;
};

	}
}

