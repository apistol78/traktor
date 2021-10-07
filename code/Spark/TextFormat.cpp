#include "Spark/TextFormat.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.TextFormat", TextFormat, Object)

TextFormat::TextFormat(float letterSpacing, SwfTextAlignType align, float size)
:	m_letterSpacing(letterSpacing)
,	m_align(align)
,	m_size(size)
{
}

	}
}
