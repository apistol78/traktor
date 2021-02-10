#pragma once

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

class StyleBitmap;

/*! Widget style sheet.
 * \ingroup UI
 */
class T_DLLCLASS StyleSheet : public ISerializable
{
	T_RTTI_CLASS;

public:
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

	static Ref< StyleSheet > createDefault();

	void setColor(const std::wstring& typeName, const std::wstring& element, const Color4ub& color);

	Color4ub getColor(const std::wstring& typeName, const std::wstring& element) const;

	Color4ub getColor(const Object* widget, const std::wstring& element) const;

	std::wstring getValue(const std::wstring& name) const;

	/*! Merge this style sheet with another.
	 *
	 * Styles defined in right override existing styles.
	 * */
	Ref< StyleSheet > merge(const StyleSheet* right) const;

	virtual void serialize(ISerializer& s) override;

	const std::vector< Group >& getGroups() const { return m_groups; }

	const std::vector< Value >& getValues() const { return m_values; }

private:
	std::vector< Group > m_groups;
	std::vector< Value > m_values;

	void setColor(const wchar_t* const type, const wchar_t* const element, const Color4ub& color);

	void setValue(const wchar_t* const name, const wchar_t* const value);
};

	}
}

