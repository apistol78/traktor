#include "Spark/Editor/Shape/SvgDocument.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SvgDocument", SvgDocument, SvgShape)

SvgDocument::SvgDocument()
:	m_size(0.0f, 0.0f)
{
}

void SvgDocument::setViewBox(const Aabb2& viewBox)
{
	m_viewBox = viewBox;
}

const Aabb2& SvgDocument::getViewBox() const
{
	return m_viewBox;
}

void SvgDocument::setSize(const Vector2& size)
{
	m_size = size;
}

const Vector2& SvgDocument::getSize() const
{
	return m_size;
}

	}
}
