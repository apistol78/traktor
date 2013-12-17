#include "Amalgam/Classes/I18NClasses.h"
#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

i18n::Format::Argument convertArgument(const script::Any& value)
{
	if (value.isBoolean())
		return i18n::Format::Argument(value.getBoolean() ? L"true" : L"false");
	else if (value.isInteger())
		return i18n::Format::Argument(value.getInteger());
	else if (value.isFloat())
		return i18n::Format::Argument(value.getFloat());
	else if (value.isString())
		return i18n::Format::Argument(value.getWideString());
	else
		return i18n::Format::Argument();
}

class I18N : public Object
{
	T_RTTI_CLASS;

public:
	static std::wstring text(const std::wstring& id)
	{
		return i18n::Text(id);
	}

	static std::wstring format_1(const std::wstring& id, const script::Any& value1)
	{
		return i18n::Format(id, convertArgument(value1));
	}

	static std::wstring format_2(const std::wstring& id, const script::Any& value1, const script::Any& value2)
	{
		return i18n::Format(id, convertArgument(value1), convertArgument(value2));
	}

	static std::wstring format_3(const std::wstring& id, const script::Any& value1, const script::Any& value2, const script::Any& value3)
	{
		return i18n::Format(id, convertArgument(value1), convertArgument(value2), convertArgument(value3));
	}

	static std::wstring format_4(const std::wstring& id, const script::Any& value1, const script::Any& value2, const script::Any& value3, const script::Any& value4)
	{
		return i18n::Format(id, convertArgument(value1), convertArgument(value2), convertArgument(value3), convertArgument(value4));
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.I18N", I18N, Object)

		}

void registerI18NClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< I18N > > classI18N = new script::AutoScriptClass< I18N >();
	classI18N->addStaticMethod("text", &I18N::text);
	classI18N->addStaticMethod("format", &I18N::format_1);
	classI18N->addStaticMethod("format", &I18N::format_2);
	classI18N->addStaticMethod("format", &I18N::format_3);
	classI18N->addStaticMethod("format", &I18N::format_4);
	scriptManager->registerClass(classI18N);
}

	}
}
