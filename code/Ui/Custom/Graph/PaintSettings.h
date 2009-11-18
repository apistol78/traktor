#ifndef traktor_ui_custom_PaintSettings_H
#define traktor_ui_custom_PaintSettings_H

#include "Core/Object.h"
#include "Core/Math/Color.h"
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

	void setGridBackground(const Color& gridBackground);

	const Color& getGridBackground() const;

	void setNodeShadow(const Color& nodeShadow);

	const Color& getNodeShadow() const;

	void setNodeForeground(const Color& nodeForeground);

	const Color& getNodeForeground() const;

	void setNodeBackgroundTop(const Color& nodeBackgroundTop);

	const Color& getNodeBackgroundTop() const;

	void setNodeBackgroundBottom(const Color& nodeBackgroundBottom);

	const Color& getNodeBackgroundBottom() const;

	void setNodeText(const Color& nodeText);

	const Color& getNodeText() const;

	void setNodeTextInfo(const Color& nodeTextInfo);

	const Color& getNodeTextInfo() const;

	void setNodeTextUnconnected(const Color& nodeTextUnconnected);

	const Color& getNodeTextUnconnected() const;

	void setNodeHalo(const Color& nodeHalo);

	const Color& getNodeHalo() const;

	void setEdge(const Color&  edge);

	const Color& getEdge() const;

	void setEdgeSelected(const Color& edgeSelected);

	const Color& getEdgeSelected() const;

	void setEdgeCursor(const Color& edgeCursor);

	const Color& getEdgeCursor() const;

	void setSelectionCursor(const Color& selectionCursor);

	const Color& getSelectionCursor() const;

	void setGuideCursor(const Color& guideCursor);

	const Color& getGuideCursor() const;

	void setSmoothSpline(bool smoothSpline);

	bool getSmoothSpline() const;
	
private:
	Size m_gridSize;
	ui::Font m_font;
	ui::Font m_fontBold;
	ui::Font m_fontUnderline;
	Color m_gridBackground;
	Color m_nodeShadow;
	Color m_nodeForeground;
	Color m_nodeBackgroundTop;
	Color m_nodeBackgroundBottom;
	Color m_nodeText;
	Color m_nodeTextInfo;
	Color m_nodeTextUnconnected;
	Color m_nodeHalo;
	Color m_edge;
	Color m_edgeSelected;
	Color m_edgeCursor;
	Color m_selectionCursor;
	Color m_guideCursor;
	bool m_smoothSpline;
};
		
		}
	}
}

#endif	// traktor_ui_custom_PaintSettings_H
