#include "Spark/TextFormat.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.TextFormat", TextFormat, ActionObjectRelay)

TextFormat::TextFormat(float letterSpacing, SwfTextAlignType align, float size)
:	ActionObjectRelay("TextFormat")
,	m_letterSpacing(letterSpacing)
,	m_align(align)
,	m_size(size)
{
}

	}
}
