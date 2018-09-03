#pragma once

#include <string>
#include "Core/Object.h"
#include "Ui/Size.h"

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

class IFontMetric;

/*! \brief
 * \ingroup UI
 */
class T_DLLCLASS FontMetric : public Object
{
	T_RTTI_CLASS;

public:
	explicit FontMetric(const IFontMetric* metric);

	/*! \brief Distance from top to base line. */
	int32_t getAscent() const;

	/*! \brief Distance from base line to bottom. */
	int32_t getDescent() const;

	/*! \brief Distance from top to bottom. */
	int32_t getHeight() const;

	/*! \brief Advancement to next character. */
	int32_t getAdvance(wchar_t ch, wchar_t next) const;

	/*! \brief Line spacing. */
	int32_t getLineSpacing() const;

	/*! \brief Extent of text. */
	Size getExtent(const std::wstring& text) const;

private:
	const IFontMetric* m_metric;
};
	
	}
}
