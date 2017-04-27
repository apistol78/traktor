/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
,	m_fontProbe(font.getFace(), font.getSize(), false, false, false)
,	m_smoothSpline(false)
{
	m_gridBackground = Color4ub(220, 220, 220);
	m_nodeShadow = Color4ub(100, 100, 100);
	m_nodeForeground = Color4ub(90, 90, 90);
	m_nodeBackgroundTop = Color4ub(230, 230, 230);
	m_nodeBackgroundBottom = Color4ub(198, 198, 198);
	m_nodeText = Color4ub(20, 30, 20);
	m_nodeTextInfo = Color4ub(80, 90, 80);
	m_nodeTextUnconnected = Color4ub(250, 40, 40);
	m_nodeHalo = Color4ub(80, 120, 200);
	m_edge = Color4ub(255, 255, 255);
	m_edgeSelected = Color4ub(80, 90, 255);
	m_edgeCursor = Color4ub(110, 110, 110);
	m_selectionCursor = Color4ub(0, 0, 0, 64);
	m_guideCursor = Color4ub(180, 180, 140);
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

void PaintSettings::setFontProbe(const ui::Font& fontProbe)
{
	m_fontProbe = fontProbe;
}

const ui::Font& PaintSettings::getFontProbe() const
{
	return m_fontProbe;
}

void PaintSettings::setGridBackground(const Color4ub& gridBackground)
{
	m_gridBackground = gridBackground;
}

const Color4ub& PaintSettings::getGridBackground() const
{
	return m_gridBackground;
}

void PaintSettings::setNodeShadow(const Color4ub& nodeShadow)
{
	m_nodeShadow = nodeShadow;
}

const Color4ub& PaintSettings::getNodeShadow() const
{
	return m_nodeShadow;
}

void PaintSettings::setNodeForeground(const Color4ub& nodeForeground)
{
	m_nodeForeground = nodeForeground;
}

const Color4ub& PaintSettings::getNodeForeground() const
{
	return m_nodeForeground;
}

void PaintSettings::setNodeBackgroundTop(const Color4ub& nodeBackgroundTop)
{
	m_nodeBackgroundTop = nodeBackgroundTop;
}

const Color4ub& PaintSettings::getNodeBackgroundTop() const
{
	return m_nodeBackgroundTop;
}

void PaintSettings::setNodeBackgroundBottom(const Color4ub& nodeBackgroundBottom)
{
	m_nodeBackgroundBottom = nodeBackgroundBottom;
}

const Color4ub& PaintSettings::getNodeBackgroundBottom() const
{
	return m_nodeBackgroundBottom;
}

void PaintSettings::setNodeText(const Color4ub& nodeText)
{
	m_nodeText = nodeText;
}

const Color4ub& PaintSettings::getNodeText() const
{
	return m_nodeText;
}

void PaintSettings::setNodeTextInfo(const Color4ub& nodeTextInfo)
{
	m_nodeTextInfo = nodeTextInfo;
}

const Color4ub& PaintSettings::getNodeTextInfo() const
{
	return m_nodeTextInfo;
}

void PaintSettings::setNodeTextUnconnected(const Color4ub& nodeTextUnconnected)
{
	m_nodeTextUnconnected = nodeTextUnconnected;
}

const Color4ub& PaintSettings::getNodeTextUnconnected() const
{
	return m_nodeTextUnconnected;
}

void PaintSettings::setNodeHalo(const Color4ub& nodeHalo)
{
	m_nodeHalo = nodeHalo;
}

const Color4ub& PaintSettings::getNodeHalo() const
{
	return m_nodeHalo;
}

void PaintSettings::setEdge(const Color4ub&  edge)
{
	m_edge = edge;
}

const Color4ub& PaintSettings::getEdge() const
{
	return m_edge;
}

void PaintSettings::setEdgeSelected(const Color4ub& edgeSelected)
{
	m_edgeSelected = edgeSelected;
}

const Color4ub& PaintSettings::getEdgeSelected() const
{
	return m_edgeSelected;
}

void PaintSettings::setEdgeCursor(const Color4ub& edgeCursor)
{
	m_edgeCursor = edgeCursor;
}

const Color4ub& PaintSettings::getEdgeCursor() const
{
	return m_edgeCursor;
}

void PaintSettings::setSelectionCursor(const Color4ub& selectionCursor)
{
	m_selectionCursor = selectionCursor;
}

const Color4ub& PaintSettings::getSelectionCursor() const
{
	return m_selectionCursor;
}

void PaintSettings::setGuideCursor(const Color4ub& guideCursor)
{
	m_guideCursor = guideCursor;
}

const Color4ub& PaintSettings::getGuideCursor() const
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
