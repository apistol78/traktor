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

/*! \brief Widget style sheet.
 * \ingroup UI
 */
class T_DLLCLASS StyleSheet : public ISerializable
{
	T_RTTI_CLASS;

public:
	static Ref< StyleSheet > createDefault();

	Color4ub getColor(const Object* widget, const wchar_t* const element) const;

	std::wstring getValue(const wchar_t* const name) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE;

private:
	struct Group
	{
		std::wstring type;
		std::wstring element;
		Color4ub color;

		void serialize(ISerializer& s);
	};

	struct Value
	{
		std::wstring name;
		std::wstring value;

		void serialize(ISerializer& s);
	};

	std::vector< Group > m_groups;
	std::vector< Value > m_values;

	void setColor(const wchar_t* const type, const wchar_t* const element, const Color4ub& color);
};

	}
}

#endif	// traktor_ui_StyleSheet_H
