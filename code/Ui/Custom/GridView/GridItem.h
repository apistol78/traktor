#ifndef traktor_ui_GridItem_H
#define traktor_ui_GridItem_H

#include "Core/Object.h"
#include "Ui/Associative.h"

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

/*! \brief Grid item.
 * \ingroup UIC
 */
class T_DLLCLASS GridItem
:	public Object
,	public Associative
{
	T_RTTI_CLASS(GridItem)

public:
	GridItem();

	GridItem(const std::wstring& text);

	GridItem(const std::wstring& text, int32_t image);

	GridItem(const std::wstring& text, int32_t image, int32_t expandedImage);

	void setText(const std::wstring& text);

	const std::wstring& getText() const { return m_text; }

	void setImage(int32_t image);

	int32_t getImage() const { return m_image; }

	void setExpandedImage(int expandedImage);

	int32_t getExpandedImage() const { return m_expandedImage; }

private:
	std::wstring m_text;
	int32_t m_image;
	int32_t m_expandedImage;
};

		}
	}
}

#endif	// traktor_ui_GridItem_H
