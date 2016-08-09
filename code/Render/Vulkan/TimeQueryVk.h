#ifndef traktor_render_TimeQueryVk_H
#define traktor_render_TimeQueryVk_H

#include "Render/ITimeQuery.h"

namespace traktor
{
	namespace render
	{

/*! \brief GPU time stamp recording query.
 * \ingroup Render
 */
class TimeQueryVk : public ITimeQuery
{
	T_RTTI_CLASS;

public:
	TimeQueryVk();

	bool create();

	virtual void begin() T_OVERRIDE T_FINAL;

	virtual int32_t stamp() T_OVERRIDE T_FINAL;

	virtual void end() T_OVERRIDE T_FINAL;	

	virtual bool ready() const T_OVERRIDE T_FINAL;

	virtual uint64_t get(int32_t index) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_render_TimeQueryVk_H
