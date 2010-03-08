#include "Ui/Custom/Graph/PaintSettings.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.PaintSettings", PaintSettings, Object)

PaintSettings::PaintSettings(const ui::Font& font)
:	m_font(font.getFace(), font.getSize(), false, false, false)
,	m_fontBold(font.getFace(), font.getSize(), true, false, false)
,	m_fontUnderline(font.getFace(), font.getSize(), false, false, true)
,	m_smoothSpline(false)
{
	m_gridBackground = Color(220, 220, 220);
	m_nodeShadow = Color(100, 100, 100);
	m_nodeForeground = Color(90, 90, 90);
	m_nodeBackgroundTop = Color(230, 230, 230);
	m_nodeBackgroundBottom = Color(198, 198, 198);
	m_nodeText = Color(20, 30, 20);
	m_nodeTextInfo = Color(80, 90, 80);
	m_nodeTextUnconnected = Color(250, 40, 40);
	m_nodeHalo = Color(80, 120, 200);
	m_edge = Color(120, 120, 120);
	m_edgeSelected = Color(80, 90, 220);
	m_edgeCursor = Color(110, 110, 110);
	m_selectionCursor = Color(0, 0, 0, 64);
	m_guideCursor = Color(180, 180, 140);
}

void PaintSettings::setFont(const ui::Font& font)
{
	m_font = font;
}

const ui::Font& PaintSettings::getFont() const
{
	return m_font;
}

void PaintSettings::setFontBold(const ui::Font& fontBold)
{
	m_fontBold = fontBold;
}

const ui::Font& PaintSettings::getFontBold() const
{
	return m_fontBold;
}

void PaintSettings::setFontUnderline(const ui::Font& fontUnderline)
{
	m_fontUnderline = fontUnderline;
}

const ui::Font& PaintSettings::getFontUnderline() const
{
	return m_fontUnderline;
}

void PaintSettings::setGridBackground(const Color& gridBackground)
{
	m_gridBackground = gridBackground;
}

const Color& PaintSettings::getGridBackground() const
{
	return m_gridBackground;
}

void PaintSettings::setNodeShadow(const Color& nodeShadow)
{
	m_nodeShadow = nodeShadow;
}

const Color& PaintSettings::getNodeShadow() const
{
	return m_nodeShadow;
}

void PaintSettings::setNodeForeground(const Color& nodeForeground)
{
	m_nodeForeground = nodeForeground;
}

const Color& PaintSettings::getNodeForeground() const
{
	return m_nodeForeground;
}

void PaintSettings::setNodeBackgroundTop(const Color& nodeBackgroundTop)
{
	m_nodeBackgroundTop = nodeBackgroundTop;
}

const Color& PaintSettings::getNodeBackgroundTop() const
{
	return m_nodeBackgroundTop;
}

void PaintSettings::setNodeBackgroundBottom(const Color& nodeBackgroundBottom)
{
	m_nodeBackgroundBottom = nodeBackgroundBottom;
}

const Color& PaintSettings::getNodeBackgroundBottom() const
{
	return m_nodeBackgroundBottom;
}

void PaintSettings::setNodeText(const Color& nodeText)
{
	m_nodeText = nodeText;
}

const Color& PaintSettings::getNodeText() const
{
	return m_nodeText;
}

void PaintSettings::setNodeTextInfo(const Color& nodeTextInfo)
{
	m_nodeTextInfo = nodeTextInfo;
}

const Color& PaintSettings::getNodeTextInfo() const
{
	return m_nodeTextInfo;
}

void PaintSettings::setNodeTextUnconnected(const Color& nodeTextUnconnected)
{
	m_nodeTextUnconnected = nodeTextUnconnected;
}

const Color& PaintSettings::getNodeTextUnconnected() const
{
	return m_nodeTextUnconnected;
}

void PaintSettings::setNodeHalo(const Color& nodeHalo)
{
	m_nodeHalo = nodeHalo;
}

const Color& PaintSettings::getNodeHalo() const
{
	return m_nodeHalo;
}

void PaintSettings::setEdge(const Color&  edge)
{
	m_edge = edge;
}

const Color& PaintSettings::getEdge() const
{
	return m_edge;
}

void PaintSettings::setEdgeSelected(const Color& edgeSelected)
{
	m_edgeSelected = edgeSelected;
}

const Color& PaintSettings::getEdgeSelected() const
{
	return m_edgeSelected;
}

void PaintSettings::setEdgeCursor(const Color& edgeCursor)
{
	m_edgeCursor = edgeCursor;
}

const Color& PaintSettings::getEdgeCursor() const
{
	return m_edgeCursor;
}

void PaintSettings::setSelectionCursor(const Color& selectionCursor)
{
	m_selectionCursor = selectionCursor;
}

const Color& PaintSettings::getSelectionCursor() const
{
	return m_selectionCursor;
}

void PaintSettings::setGuideCursor(const Color& guideCursor)
{
	m_guideCursor = guideCursor;
}

const Color& PaintSettings::getGuideCursor() const
{
	return m_guideCursor;
}

void PaintSettings::setSmoothSpline(bool smoothSpline)
{
	m_smoothSpline = smoothSpline;
}

bool PaintSettings::getSmoothSpline() const
{
	return m_smoothSpline;
}

		}
	}
}
