#pragma once

#include "Ui/Graph/INodeShape.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class IBitmap;

/*! Input/output node shape.
 * \ingroup UI
 */
class T_DLLCLASS InOutNodeShape : public INodeShape
{
	T_RTTI_CLASS;

public:
	enum Style
	{
		StDefault = 0,
		StUniform = 1,
		StVariable = 2
	};

	explicit InOutNodeShape(Style style);

	virtual Point getPinPosition(GraphControl* graph, const Node* node, const Pin* pin) const override final;

	virtual Pin* getPinAt(GraphControl* graph, const Node* node, const Point& pt) const override final;

	virtual void paint(GraphControl* graph, const Node* node, GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const override final;

	virtual Size calculateSize(GraphControl* graph, const Node* node) const override final;

private:
	Ref< IBitmap > m_imageNode[4];
	Ref< IBitmap > m_imagePin;
	Ref< IBitmap > m_imagePinHot;
};

	}
}

