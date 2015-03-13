#ifndef traktor_ui_custom_PaintSettings_H
#define traktor_ui_custom_PaintSettings_H

#include "Core/Object.h"
#include "Core/Math/Color4ub.h"
#include "Ui/Size.h"
#include "Ui/Point.h"
#include "Ui/Font.h"

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
		namespace custom
		{

/*! \brief Graph visual settings.
 * \ingroup UIC
 */
class T_DLLCLASS PaintSettings : public Object
{
	T_RTTI_CLASS;

public:
	PaintSettings(const ui::Font& font);

	void setFont(const ui::Font& font);

	const ui::Font& getFont() const;

	void setFontBold(const ui::Font& fontBold);

	const ui::Font& getFontBold() const;

	void setFontUnderline(const ui::Font& fontUnderline);

	const ui::Font& getFontUnderline() const;

	void setFontProbe(const ui::Font& fontProbe);

	const ui::Font& getFontProbe() const;

	void setGridBackground(const Color4ub& gridBackground);

	const Color4ub& getGridBackground() const;

	void setNodeShadow(const Color4ub& nodeShadow);

	const Color4ub& getNodeShadow() const;

	void setNodeForeground(const Color4ub& nodeForeground);

	const Color4ub& getNodeForeground() const;

	void setNodeBackgroundTop(const Color4ub& nodeBackgroundTop);

	const Color4ub& getNodeBackgroundTop() const;

	void setNodeBackgroundBottom(const Color4ub& nodeBackgroundBottom);

	const Color4ub& getNodeBackgroundBottom() const;

	void setNodeText(const Color4ub& nodeText);

	const Color4ub& getNodeText() const;

	void setNodeTextInfo(const Color4ub& nodeTextInfo);

	const Color4ub& getNodeTextInfo() const;

	void setNodeTextUnconnected(const Color4ub& nodeTextUnconnected);

	const Color4ub& getNodeTextUnconnected() const;

	void setNodeHalo(const Color4ub& nodeHalo);

	const Color4ub& getNodeHalo() const;

	void setEdge(const Color4ub&  edge);

	const Color4ub& getEdge() const;

	void setEdgeSelected(const Color4ub& edgeSelected);

	const Color4ub& getEdgeSelected() const;

	void setEdgeCursor(const Color4ub& edgeCursor);

	const Color4ub& getEdgeCursor() const;

	void setSelectionCursor(const Color4ub& selectionCursor);

	const Color4ub& getSelectionCursor() const;

	void setGuideCursor(const Color4ub& guideCursor);

	const Color4ub& getGuideCursor() const;

	void setSmoothSpline(bool smoothSpline);

	bool getSmoothSpline() const;
	
private:
	Size m_gridSize;
	ui::Font m_font;
	ui::Font m_fontBold;
	ui::Font m_fontUnderline;
	ui::Font m_fontProbe;
	Color4ub m_gridBackground;
	Color4ub m_nodeShadow;
	Color4ub m_nodeForeground;
	Color4ub m_nodeBackgroundTop;
	Color4ub m_nodeBackgroundBottom;
	Color4ub m_nodeText;
	Color4ub m_nodeTextInfo;
	Color4ub m_nodeTextUnconnected;
	Color4ub m_nodeHalo;
	Color4ub m_edge;
	Color4ub m_edgeSelected;
	Color4ub m_edgeCursor;
	Color4ub m_selectionCursor;
	Color4ub m_guideCursor;
	bool m_smoothSpline;
};
		
		}
	}
}

#endif	// traktor_ui_custom_PaintSettings_H
