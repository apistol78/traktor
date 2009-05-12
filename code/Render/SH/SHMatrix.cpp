#include "Render/SH/SHMatrix.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.render.SHMatrix", SHMatrix, Serializable)

SHMatrix::SHMatrix()
:	m_rows(0)
,	m_columns(0)
{
}

SHMatrix::SHMatrix(int32_t rows, int32_t columns)
:	m_rows(rows)
,	m_columns(columns)
,	m_elements(rows * columns)
{
}

bool SHMatrix::serialize(Serializer& s)
{
	s >> Member< int32_t >(L"rows", m_rows);
	s >> Member< int32_t >(L"columns", m_columns);
	s >> MemberStlVector< float >(L"elements", m_elements);
	return true;
}

	}
}
