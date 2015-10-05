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

	virtual void begin() T_OVERRIDE T_FINAL;

	virtual int32_t stamp() T_OVERRIDE T_FINAL;

	virtual void end() T_OVERRIDE T_FINAL;	

	virtual bool ready() const T_OVERRIDE T_FINAL;

	virtual uint64_t get(int32_t index) const T_OVERRIDE T_FINAL;

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
