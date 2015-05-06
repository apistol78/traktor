#ifndef traktor_ui_StyleSheet_H
#define traktor_ui_StyleSheet_H

#include <vector>
#include "Core/Math/Color4ub.h"
#include "Core/Serialization/ISerializable.h"

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

class Widget;

/*! \brief Widget style sheet.
 * \ingroup UI
 */
class T_DLLCLASS StyleSheet : public ISerializable
{
	T_RTTI_CLASS;

public:
	Color4ub getColor(const Widget* widget, const wchar_t* const element) const;

	virtual void serialize(ISerializer& s);

private:
	struct Group
	{
		std::wstring type;
		std::wstring element;
		Color4ub color;

		void serialize(ISerializer& s);
	};

	std::vector< Group > m_groups;
};

	}
}

#endif	// traktor_ui_StyleSheet_H
