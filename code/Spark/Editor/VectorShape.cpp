#include "Spark/Editor/VectorShape.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

class FillStylePred
{
public:
	FillStylePred(const VectorShape::FillStyle& fs)
	:	m_fs(fs)
	{
	}

	bool operator () (const VectorShape::FillStyle& rh) const
	{
		return rh.color == m_fs.color;
	}

private:
	const VectorShape::FillStyle& m_fs;
};

class LineStylePred
{
public:
	LineStylePred(const VectorShape::LineStyle& ls)
	:	m_ls(ls)
	{
	}

	bool operator () (const VectorShape::LineStyle& rh) const
	{
		return rh.color == m_ls.color && rh.width == m_ls.width;
	}

private:
	const VectorShape::LineStyle& m_ls;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.VectorShape", VectorShape, Object)

void VectorShape::setBounds(const Aabb2& bounds)
{
	m_bounds = bounds;
}

const Aabb2& VectorShape::getBounds() const
{
	return m_bounds;
}

int32_t VectorShape::addFillStyle(const Color4f& color)
{
	FillStyle fs = { color };

	// Check if style already added.
	AlignedVector< FillStyle >::iterator i = std::find_if(m_fillStyles.begin(), m_fillStyles.end(), FillStylePred(fs));
	if (i != m_fillStyles.end())
		return int32_t(std::distance(m_fillStyles.begin(), i));

	m_fillStyles.push_back(fs);
	return int32_t(m_fillStyles.size() - 1);
}

int32_t VectorShape::addLineStyle(const Color4f& color, float width)
{
	LineStyle ls = { color, width };

	// Check if style already added.
	AlignedVector< LineStyle >::iterator i = std::find_if(m_lineStyles.begin(), m_lineStyles.end(), LineStylePred(ls));
	if (i != m_lineStyles.end())
		return int32_t(std::distance(m_lineStyles.begin(), i));

	m_lineStyles.push_back(ls);
	return int32_t(m_lineStyles.size() - 1);
}

void VectorShape::fill(const Path& path, int32_t fillStyle)
{
	PathWithStyle pws = { path, fillStyle, -1 };
	m_paths.push_back(pws);
}

void VectorShape::stroke(const Path& path, int32_t lineStyle)
{
	PathWithStyle pws = { path, -1, lineStyle };
	m_paths.push_back(pws);
}

	}
}
