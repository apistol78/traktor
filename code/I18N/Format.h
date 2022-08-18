#pragma once

#include <string>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_I18N_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace i18n
	{

/*! Automatic text localization with formating.
 * \ingroup I18N
 *
 * Used as an implicit bridge from ID to localized text.
 * This class is more advanced than Text as it support
 * automatic string formating.
 */
class T_DLLCLASS Format : public Object
{
	T_RTTI_CLASS;

public:
	class T_DLLCLASS Argument
	{
	public:
		enum class Type
		{
			Void,
			Char,
			Int32,
			Float,
			Double,
			String
		};

		Argument(void);

		Argument(wchar_t c);

		Argument(int32_t i);

		Argument(float f);

		Argument(double d);

		Argument(const std::wstring& s);

		Argument(const wchar_t* s);

		Argument(const Argument& s);

		Argument(Argument&& s);

		virtual ~Argument();

	private:
		friend class Format;

		union Value
		{
			wchar_t c;
			int32_t i;
			float f;
			double d;
			wchar_t* s;
		};

		static Argument ms_void;
		Type m_type;
		Value m_value;

		std::wstring format() const;
	};

	Format(
		const std::wstring& id,
		const Argument& arg1 = Argument::ms_void,
		const Argument& arg2 = Argument::ms_void,
		const Argument& arg3 = Argument::ms_void,
		const Argument& arg4 = Argument::ms_void
	);

	const std::wstring& str() const;

	operator const std::wstring& () const;

private:
	std::wstring m_text;
};

	}
}

