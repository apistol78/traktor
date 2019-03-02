#pragma once

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

	virtual void begin() override final;

	virtual int32_t stamp() override final;

	virtual void end() override final;

	virtual bool ready() const override final;

	virtual uint64_t get(int32_t index) const override final;
};

	}
}

