#ifndef traktor_render_TimeQueryPs4_H
#define traktor_render_TimeQueryPs4_H

#include "Render/ITimeQuery.h"

namespace traktor
{
	namespace render
	{

class ContextPs4;

/*! \brief GPU time stamp recording query.
 * \ingroup Render
 */
class TimeQueryPs4 : public ITimeQuery
{
	T_RTTI_CLASS;

public:
	TimeQueryPs4(ContextPs4* context);

	bool create();

	virtual void begin() T_OVERRIDE T_FINAL;

	virtual int32_t stamp() T_OVERRIDE T_FINAL;

	virtual void end() T_OVERRIDE T_FINAL;	

	virtual bool ready() const T_OVERRIDE T_FINAL;

	virtual uint64_t get(int32_t index) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_TimeQueryPs4_H
