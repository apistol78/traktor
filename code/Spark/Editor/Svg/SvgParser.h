#ifndef traktor_spark_SvgParser_H
#define traktor_spark_SvgParser_H

#include <map>
#include "Core/Ref.h"
#include "Core/Object.h"
#include "Core/Math/Matrix33.h"

namespace traktor
{
	namespace xml
	{

class Document;
class Element;

	}

	namespace spark
	{

class SvgShape;
class SvgStyle;
class SvgGradient;

/*! \brief
 * \ingroup Spark
 */
class SvgParser : public Object
{
	T_RTTI_CLASS;

public:
	Ref< SvgShape > parse(xml::Document* doc);

private:
	std::map< std::wstring, Ref< SvgGradient > > m_gradients;

	Ref< SvgShape > traverse(xml::Element* elm);

	Ref< SvgShape > parseDocument(xml::Element* elm);

	Ref< SvgShape > parseGroup(xml::Element* elm);

	Ref< SvgShape > parseCircle(xml::Element* elm);

	Ref< SvgShape > parseRect(xml::Element* elm);

	Ref< SvgShape > parsePolygon(xml::Element* elm);

	Ref< SvgShape > parsePolyLine(xml::Element* elm);

	Ref< SvgShape > parsePath(xml::Element* elm);
	
	void parseDefs(xml::Element* elm);
	
	Ref< SvgStyle > parseStyle(xml::Element* elm);

	Matrix33 parseTransform(xml::Element* elm);

	float parseAttr(xml::Element* elm, const std::wstring& attrName, float defValue = 0.0f) const;
};

	}
}

#endif	// traktor_spark_SvgParser_H
