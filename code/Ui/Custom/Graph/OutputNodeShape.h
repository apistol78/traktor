#ifndef traktor_ui_custom_OutputNodeShape_H
#define traktor_ui_custom_OutputNodeShape_H

#include "Core/Heap/Ref.h"
#include "Ui/Custom/Graph/NodeShape.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Bitmap;

		namespace custom
		{

class GraphControl;

/*! \brief Output only node shape.
 * \ingroup UIC
 */
class T_DLLCLASS OutputNodeShape : public NodeShape
{
	T_RTTI_CLASS(OutputNodeShape)

public:
	OutputNodeShape(GraphControl* graphControl);

	virtual Point getPinPosition(const Node* node, const Pin* pin);

	virtual Pin* getPinAt(const Node* node, const Point& pt);

	virtual void paint(const Node* node, const PaintSettings* settings, Canvas* canvas, const Size& offset);

	virtual Size calculateSize(const Node* node);

private:
	Ref< GraphControl > m_graphControl;
	Ref< Bitmap > m_imageNode;
	Ref< Bitmap > m_imagePin;
};

		}
	}
}

#endif	// traktor_ui_custom_OutputNodeShape_H
