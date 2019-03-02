#include "Terrain/Terrain.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.Terrain", Terrain, Object)

Terrain::Terrain()
:	m_detailSkip(0)
,	m_patchDim(0)
{
}

	}
}
