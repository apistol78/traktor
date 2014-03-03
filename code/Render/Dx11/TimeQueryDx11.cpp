#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/TimeQueryDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.TimeQueryDx11", TimeQueryDx11, ITimeQuery)

TimeQueryDx11::TimeQueryDx11(ContextDx11* context)
:	m_context(context)
,	m_current(0)
{
}

bool TimeQueryDx11::create()
{
	D3D11_QUERY_DESC dqd;

	dqd.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	dqd.MiscFlags = 0;

	m_context->getD3DDevice()->CreateQuery(&dqd, &m_disjointQuery.getAssign());

	dqd.Query = D3D11_QUERY_TIMESTAMP;
	dqd.MiscFlags = 0;

	for (int32_t i = 0; i < sizeof_array(m_timeQueries); ++i)
		m_context->getD3DDevice()->CreateQuery(&dqd, &m_timeQueries[i].getAssign());

	return true;
}

void TimeQueryDx11::begin()
{
	m_context->getD3DDeviceContext()->Begin(m_disjointQuery);
}

int32_t TimeQueryDx11::stamp()
{
	int32_t index = m_current;
	m_context->getD3DDeviceContext()->End(m_timeQueries[index]);
	m_current = (m_current + 1) % sizeof_array(m_timeQueries);
	return index;
}

void TimeQueryDx11::end()
{
	m_context->getD3DDeviceContext()->End(m_disjointQuery);
}

bool TimeQueryDx11::ready() const
{
	return m_context->getD3DDeviceContext()->GetData(m_disjointQuery, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK;
}

uint64_t TimeQueryDx11::get(int32_t index) const
{
	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
	UINT64 timeStamp;

	m_context->getD3DDeviceContext()->GetData(m_disjointQuery, &disjointData, sizeof(disjointData), 0);
	m_context->getD3DDeviceContext()->GetData(m_timeQueries[index], &timeStamp, sizeof(timeStamp), 0);

	return (timeStamp * 1000000) / disjointData.Frequency;
}

	}
}
