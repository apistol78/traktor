#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Spark/Editor/VectorShape.h"
#include "Spark/Editor/Svg/SvgDocument.h"
#include "Spark/Editor/Svg/SvgParser.h"
#include "Spark/Editor/Svg/SvgPathShape.h"
#include "Spark/Editor/Svg/SvgShape.h"
#include "Spark/Editor/Svg/SvgShapeFormat.h"
#include "Spark/Editor/Svg/SvgShapeVisitor.h"
#include "Spark/Editor/Svg/SvgStyle.h"
#include "Xml/Document.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

Color4f getSvgStyleFillColor(const SvgStyle* style)
{
	return Color4f(
		style->getFill().r / 255.0f,
		style->getFill().g / 255.0f,
		style->getFill().b / 255.0f,
		int32_t(style->getOpacity() * 255)
	);
}

Color4f getSvgStyleStrokeColor(const SvgStyle* style)
{
	return Color4f(
		style->getStroke().r / 255.0f,
		style->getStroke().g / 255.0f,
		style->getStroke().b / 255.0f,
		int32_t(style->getOpacity() * 255)
	);
}

class VectorShapeGenerator : public SvgShapeVisitor
{
public:
	VectorShapeGenerator(VectorShape* shape)
	:	m_shape(shape)
	,	m_viewBox(Vector2(0.0f, 0.0f), Vector2(100.0f, 100.0f))
	,	m_size(100.0f, 100.0f)
	{
		m_transformStack.push_back(Matrix33::identity());
	}

	virtual void enter(SvgShape* shape)
	{
		m_transformStack.push_back(m_transformStack.back() * shape->getTransform());
		const Matrix33& T = m_transformStack.back();

		SvgDocument* document = dynamic_type_cast< SvgDocument* >(shape);
		if (document)
		{
			m_viewBox = document->getViewBox();
			m_size = document->getSize();
			m_shape->setBounds(m_viewBox);
		}

		if (shape->getStyle())
			m_styleStack.push_back(shape->getStyle());

		SvgPathShape* pathShape = dynamic_type_cast< SvgPathShape* >(shape);
		if (pathShape)
		{
			if (m_styleStack.empty())
				return;

			// Transform path by shape's transform and fit into view.
			Path path = pathShape->getPath();
			path.transform(
				T *
				translate(m_viewBox.mn.x, m_viewBox.mn.y) *
				scale(m_size.x / (m_viewBox.mx.x - m_viewBox.mn.y), m_size.y / (m_viewBox.mx.y - m_viewBox.mn.y))
			);

			if (m_styleStack.back()->getFillEnable())
			{
				int32_t fillStyle = m_shape->addFillStyle(getSvgStyleFillColor(m_styleStack.back()));
				if (fillStyle >= 0)
					m_shape->fill(path, fillStyle);
			}
			if (m_styleStack.back()->getStrokeEnable())
			{
				int32_t lineStyle = m_shape->addLineStyle(getSvgStyleStrokeColor(m_styleStack.back()), m_styleStack.back()->getStrokeWidth());
				if (lineStyle >= 0)
					m_shape->stroke(path, lineStyle);
			}
		}
	}

	virtual void leave(SvgShape* shape)
	{
		m_transformStack.pop_back();
		if (shape->getStyle())
			m_styleStack.pop_back();
	}

private:
	Ref< VectorShape > m_shape;
	Aabb2 m_viewBox;
	Vector2 m_size;
	RefArray< const SvgStyle > m_styleStack;
	AlignedVector< Matrix33 > m_transformStack;
	Matrix33 m_currentTransform;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.SvgShapeFormat", 0, SvgShapeFormat, VectorShapeFormat)

void SvgShapeFormat::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Scalable Vector Graphics";
	outExtensions.push_back(L"svg");
}

bool SvgShapeFormat::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase< std::wstring >(extension, L"svg") == 0;
}

Ref< VectorShape > SvgShapeFormat::read(IStream* stream) const
{
	// Parse XML document containing SVG.
	xml::Document document;
	if (!document.loadFromStream(stream))
	{
		log::error << L"Unable to parse XML file." << Endl;
		return 0;
	}

	// Parse SVG into intermediate shape.
	Ref< SvgShape > svgShape = SvgParser().parse(&document);
	if (!svgShape)
	{
		log::error << L"Unable to parse SVG document." << Endl;
		return 0;
	}

	// Convert SVG shape into our intermediate shape representation.
	Ref< VectorShape > outputShape = new VectorShape();
	VectorShapeGenerator shapeGenerator(outputShape);
	svgShape->visit(&shapeGenerator);

	return outputShape;
}

	}
}
