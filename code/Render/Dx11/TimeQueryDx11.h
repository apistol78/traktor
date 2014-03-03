#ifndef traktor_render_TimeQueryDx11_H
#define traktor_render_TimeQueryDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/ITimeQuery.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*! \brief GPU time stamp recording query.
 * \ingroup Render
 */
class TimeQueryDx11 : public ITimeQuery
{
	T_RTTI_CLASS;

public:
	TimeQueryDx11(ContextDx11* context);

	bool create();

	virtual void begin();

	virtual int32_t stamp();

	virtual void end();	

	virtual bool ready() const;

	virtual uint64_t get(int32_t index) const;

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

#endif	// traktor_render_TimeQueryDx11_H
