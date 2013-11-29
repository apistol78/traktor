#include "Render/Dx11/Profiler.h"

namespace traktor
{
	namespace render
	{

Profiler::Profiler()
:	m_count(0)
,	m_us(0)
{
}

bool Profiler::create(ID3D11Device* d3dDevice)
{
	D3D11_QUERY_DESC dqd;

	dqd.Query = D3D11_QUERY_TIMESTAMP;
	dqd.MiscFlags = 0;

	for (int32_t i = 0; i < sizeof_array(m_timeQueries); ++i)
		d3dDevice->CreateQuery(&dqd, &m_timeQueries[i].getAssign());

	dqd.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	dqd.MiscFlags = 0;

	for (int32_t i = 0; i < sizeof_array(m_disjointQueries); ++i)
		d3dDevice->CreateQuery(&dqd, &m_disjointQueries[i].getAssign());

	return true;
}

void Profiler::begin(ID3D11DeviceContext* d3dContext)
{
	int32_t current = m_count % QueryCount;
	d3dContext->Begin(m_disjointQueries[current]);
	d3dContext->End(m_timeQueries[current * 2 + 0]);
}

void Profiler::end(ID3D11DeviceContext* d3dContext)
{
	int32_t current = m_count % QueryCount;
	d3dContext->End(m_timeQueries[current * 2 + 1]);
	d3dContext->End(m_disjointQueries[current]);

	if (m_count >= QueryCount - 1)
	{
		int32_t finished = (m_count - QueryCount + 1) % QueryCount;

		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT dqdtd;
		d3dContext->GetData(m_disjointQueries[finished], &dqdtd, sizeof(dqdtd), 0);

		if (!dqdtd.Disjoint)
		{
			UINT64 timeStampStart;
			d3dContext->GetData(m_timeQueries[finished * 2 + 0], &timeStampStart, sizeof(timeStampStart), 0);

			UINT64 timeStampEnd;
			d3dContext->GetData(m_timeQueries[finished * 2 + 1], &timeStampEnd, sizeof(timeStampEnd), 0);

			m_us = ((timeStampEnd - timeStampStart) * 1000000) / dqdtd.Frequency;
		}
	}

	++m_count;
}

	}
}
