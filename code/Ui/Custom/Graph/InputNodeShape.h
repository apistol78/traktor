#ifndef traktor_ui_custom_InputNodeShape_H
#define traktor_ui_custom_InputNodeShape_H

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

/*! \brief Input only node shape.
 * \ingroup UIC
 */
class T_DLLCLASS InputNodeShape : public NodeShape
{
	T_RTTI_CLASS(InputNodeShape)

public:
	InputNodeShape(GraphControl* graphControl);

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

#endif	// traktor_ui_custom_InputNodeShape_H
