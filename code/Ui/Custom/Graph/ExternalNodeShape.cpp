#include <algorithm>
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/Graph/ExternalNodeShape.h"
#include "Ui/Custom/Graph/GraphControl.h"
#include "Ui/Custom/Graph/Node.h"
#include "Ui/Custom/Graph/PaintSettings.h"
#include "Ui/Custom/Graph/Pin.h"

// Resources
#include "Resources/External.h"
#include "Resources/Pin.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_marginWidth = 8;	/*< Distance from image edge to "visual" edge. */
const int c_marginHeight = 8;
const int c_topMargin = 4;		/*< Distance from top to top of title. */
const int c_titlePad = 8;		/*< Padding between title (and info) from first pin. */
const int c_pinNamePad = 24;	/*< Distance between pin and pin's name. */
const int c_pinCenterPad = 16;	/*< Distance between input and output pin names. */
const int c_pinHitWidth = 14;	/*< Width of pin hit area from visual edge. */

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ExternalNodeShape", ExternalNodeShape, NodeShape)

ExternalNodeShape::ExternalNodeShape(GraphControl* graphControl)
:	m_graphControl(graphControl)
{
	m_imageNode = Bitmap::load(c_ResourceExternal, sizeof(c_ResourceExternal), L"png");
	m_imagePin = Bitmap::load(c_ResourcePin, sizeof(c_ResourcePin), L"png");
}

Point ExternalNodeShape::getPinPosition(const Node* node, const Pin* pin)
{
	Rect rc = node->calculateRect();
	
	int textHeight = m_graphControl->getPaintSettings()->getFont().getPixelSize() + ui::scaleBySystemDPI(4);	
	int top = c_marginHeight + c_topMargin + c_titlePad;
	if (!node->getTitle().empty())
		top += textHeight;
	if (!node->getInfo().empty())
		top += textHeight;
	if (node->getImage())
		top += node->getImage()->getSize().cy;

	Size pinSize = m_imagePin->getSize();

	int x = pin->getDirection() == Pin::DrInput ?
		-pinSize.cx / 2 + c_marginWidth :
		rc.getWidth() - pinSize.cx / 2 - c_marginWidth;

	const RefArray< Pin >& pins = (pin->getDirection() == Pin::DrInput) ? node->getInputPins() : node->getOutputPins();
	RefArray< Pin >::const_iterator i = std::find(pins.begin(), pins.end(), pin);

	top += int(std::distance(pins.begin(), i)) * textHeight;

	return Point(rc.left + x, rc.top + top);
}

Ref< Pin > ExternalNodeShape::getPinAt(const Node* node, const Point& pt)
{
	Rect rc = node->calculateRect();
	if (!rc.inside(pt))
		return 0;

	Point ptn(pt.x - rc.left, pt.y - rc.top);

	int textHeight = m_graphControl->getPaintSettings()->getFont().getPixelSize() + ui::scaleBySystemDPI(4);	
	int top = c_marginHeight + c_topMargin + c_titlePad;
	if (!node->getTitle().empty())
		top += textHeight;
	if (!node->getInfo().empty())
		top += textHeight;
	if (node->getImage())
		top += node->getImage()->getSize().cy;

	const RefArray< Pin >* pins = 0;
	if (ptn.x <= c_pinHitWidth + c_marginWidth)
		pins = &node->getInputPins();
	else if (ptn.x >= rc.getWidth() - c_pinHitWidth - c_marginWidth)
		pins = &node->getOutputPins();

	if (!pins)
		return 0;

	for (int i = 0; i < int(pins->size()); ++i)
	{
		if (ptn.y >= top + i * textHeight - textHeight / 2 && ptn.y <= top + i * textHeight + textHeight / 2)
			return (*pins)[i];
	}

	return 0;
}

void ExternalNodeShape::paint(const Node* node, const PaintSettings* settings, Canvas* canvas, const Size& offset)
{
	Rect rc = node->calculateRect().offset(offset);
	int textHeight = m_graphControl->getPaintSettings()->getFont().getPixelSize() + ui::scaleBySystemDPI(4);	

	int sx[] = { 0, 20, 76, 96 };
	int sy[] = { 0, 20, 76, 96 };
	int dx[] = { 0, 20, rc.getWidth() - 20, rc.getWidth() };
	int dy[] = { 0, 20, rc.getHeight() - 20, rc.getHeight() };

	int ofx = node->isSelected() ? 96 : 0;
	int ofy = node->getState() * 96;

	for (int iy = 0; iy < 3; ++iy)
	{
		for (int ix = 0; ix < 3; ++ix)
		{
			canvas->drawBitmap(
				rc.getTopLeft() + Size(dx[ix], dy[iy]),
				Size(dx[ix + 1] - dx[ix], dy[iy + 1] - dy[iy]),
				Point(sx[ix] + ofx, sy[iy] + ofy),
				Size(sx[ix + 1] - sx[ix], sy[iy + 1] - sy[iy]),
				m_imageNode,
				ui::BmAlpha | ui::BmModulate
			);
		}
	}

	int top = rc.top + c_marginHeight + c_topMargin;

	const std::wstring& title = node->getTitle();
	if (!title.empty())
	{
		canvas->setForeground(settings->getNodeText());
		canvas->setFont(settings->getFontBold());
		canvas->drawText(Rect(rc.left, top, rc.right, top + textHeight), title, AnCenter, AnCenter);
		canvas->setFont(settings->getFont());

		top += textHeight;
	}

	const std::wstring& info = node->getInfo();
	if (!info.empty())
	{
		canvas->setForeground(settings->getNodeTextInfo());
		canvas->drawText(Rect(rc.left, top, rc.right, top + textHeight), info, AnCenter, AnCenter);

		top += textHeight;
	}

	const std::wstring& comment = node->getComment();
	if (!comment.empty())
	{
		canvas->setForeground(settings->getNodeShadow());
		canvas->drawText(Rect(rc.left, rc.top - textHeight, rc.right, rc.top), comment, AnCenter, AnCenter);
	}

	canvas->setBackground(Color4ub(255, 255, 255));

	if (node->getImage())
	{
		canvas->drawBitmap(
			Point(rc.getCenter().x - node->getImage()->getSize().cx / 2, top),
			Point(0, 0),
			node->getImage()->getSize(),
			node->getImage()
		);
		top += node->getImage()->getSize().cy;
	}

	top += c_titlePad;

	const RefArray< Pin >& inputPins = node->getInputPins();
	const RefArray< Pin >& outputPins = node->getOutputPins();

	Size pinSize = m_imagePin->getSize();

	for (int i = 0; i < int(inputPins.size()); ++i)
	{
		Point pos(
			rc.left - pinSize.cx / 2 + c_marginWidth,
			top + i * textHeight - pinSize.cy / 2
		);

		canvas->drawBitmap(
			pos,
			Point(0, 0),
			pinSize,
			m_imagePin,
			ui::BmAlpha
		);
	}

	for (int i = 0; i < int(outputPins.size()); ++i)
	{
		Point pos(
			rc.right - pinSize.cx / 2 - c_marginWidth,
			top + i * textHeight - pinSize.cy / 2
		);

		canvas->drawBitmap(
			pos,
			Point(0, 0),
			pinSize,
			m_imagePin,
			ui::BmAlpha
		);
	}

	canvas->setForeground(settings->getNodeText());

	for (int i = 0; i < int(inputPins.size()); ++i)
	{
		const Pin* pin = inputPins[i];
		Point pos(
			rc.left,
			top + i * textHeight
		);

		const std::wstring& name = pin->getName();
		Size extent = canvas->getTextExtent(name);

		if (pin->isMandatory())
			canvas->setFont(settings->getFontUnderline());

		canvas->drawText(
			Point(pos.x + c_pinNamePad, pos.y - extent.cy / 2),
			name
		);

		canvas->setFont(settings->getFont());
	}

	for (int i = 0; i < int(outputPins.size()); ++i)
	{
		const Pin* pin = outputPins[i];
		Point pos(
			rc.right,
			top + i * textHeight
		);

		const std::wstring& name = pin->getName();
		Size extent = canvas->getTextExtent(name);

		canvas->drawText(
			Point(pos.x - extent.cx - c_pinNamePad, pos.y - extent.cy / 2),
			name
		);
	}
}

Size ExternalNodeShape::calculateSize(const Node* node)
{
	int textHeight = m_graphControl->getPaintSettings()->getFont().getPixelSize() + ui::scaleBySystemDPI(4);	
	int height = c_marginHeight * 2 + c_topMargin + c_titlePad;
	
	if (!node->getTitle().empty())
		height += textHeight;
	if (!node->getInfo().empty())
		height += textHeight;

	if (node->getImage())
		height += node->getImage()->getSize().cy;

	int pins = std::max< int >(
		int(node->getInputPins().size()),
		int(node->getOutputPins().size())
	);
	height += pins * textHeight;

	int maxWidthPins[2] = { 0, 0 };
	for (RefArray< Pin >::const_iterator i = node->getInputPins().begin(); i != node->getInputPins().end(); ++i)
		maxWidthPins[0] = std::max< int >(maxWidthPins[0], m_graphControl->getTextExtent((*i)->getName()).cx);
	for (RefArray< Pin >::const_iterator i = node->getOutputPins().begin(); i != node->getOutputPins().end(); ++i)
		maxWidthPins[1] = std::max< int >(maxWidthPins[1], m_graphControl->getTextExtent((*i)->getName()).cx);

	int width = maxWidthPins[0] + maxWidthPins[1];

	if (!node->getTitle().empty())
	{
		m_graphControl->setFont(m_graphControl->getPaintSettings()->getFontBold());
		int titleExtent = m_graphControl->getTextExtent(node->getTitle()).cx;
		width = std::max(width, titleExtent);
		m_graphControl->setFont(m_graphControl->getPaintSettings()->getFont());
	}
	if (!node->getInfo().empty())
	{
		int infoExtent = m_graphControl->getTextExtent(node->getInfo()).cx;
		width = std::max(width, infoExtent);
	}
	if (node->getImage())
	{
		int imageExtent = node->getImage()->getSize().cx;
		width = std::max(width, imageExtent);
	}

	width += c_marginWidth * 2 + c_pinCenterPad + c_pinNamePad * 2;

	return Size(width, height);
}

		}
	}
}
