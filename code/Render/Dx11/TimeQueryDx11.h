#pragma once

#include "Core/Ref.h"
#include "Core/Misc/ComRef.h"
#include "Render/ITimeQuery.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*! GPU time stamp recording query.
 * \ingroup Render
 */
class TimeQueryDx11 : public ITimeQuery
{
	T_RTTI_CLASS;

public:
	TimeQueryDx11(ContextDx11* context);

	bool create();

	virtual void begin() override final;

	virtual int32_t stamp() override final;

	virtual void end() override final;

	virtual bool ready() const override final;

	virtual uint64_t get(int32_t index) const override final;

private:
	enum
	{
		MaxTimeQueries = 2000
	};

	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Query > m_disjointQuery;
	ComRef< ID3D11Query > m_timeQueries[MaxTimeQueries];
	int32_t m_current;
};

	}
}

