#include "Spark/Editor/Shape/Document.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Document", Document, Shape)

Document::Document()
:	m_size(0.0f, 0.0f)
{
}

void Document::setViewBox(const Aabb2& viewBox)
{
	m_viewBox = viewBox;
}

const Aabb2& Document::getViewBox() const
{
	return m_viewBox;
}

void Document::setSize(const Vector2& size)
{
	m_size = size;
}

const Vector2& Document::getSize() const
{
	return m_size;
}

	}
}
