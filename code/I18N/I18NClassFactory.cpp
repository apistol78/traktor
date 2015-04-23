#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "I18N/Dictionary.h"
#include "I18N/Format.h"
#include "I18N/I18N.h"
#include "I18N/I18NClassFactory.h"
#include "I18N/Text.h"

namespace traktor
{
	namespace i18n
	{
		namespace
		{

Format::Argument convertArgument(const Any& value)
{
	if (value.isBoolean())
		return Format::Argument(value.getBoolean() ? L"true" : L"false");
	else if (value.isInteger())
		return Format::Argument(value.getInteger());
	else if (value.isFloat())
		return Format::Argument(value.getFloat());
	else if (value.isString())
		return Format::Argument(value.getWideString());
	else
		return Format::Argument();
}

std::wstring i18n_Dictionary_get(Dictionary* self, const std::wstring& id)
{
	std::wstring tmp;
	if (self->get(id, tmp))
		return tmp;
	else
		return L"";
}

class I18NWrapper : public Object
{
	T_RTTI_CLASS;

public:
	static void appendDictionary(const Dictionary* dictionary, bool overrideExisting)
	{
		i18n::I18N::getInstance().appendDictionary(dictionary, overrideExisting);
	}

	static std::wstring text(const std::wstring& id)
	{
		return Text(id);
	}

	static std::wstring format_1(const std::wstring& id, const Any& value1)
	{
		return Format(id, convertArgument(value1));
	}

	static std::wstring format_2(const std::wstring& id, const Any& value1, const Any& value2)
	{
		return Format(id, convertArgument(value1), convertArgument(value2));
	}

	static std::wstring format_3(const std::wstring& id, const Any& value1, const Any& value2, const Any& value3)
	{
		return Format(id, convertArgument(value1), convertArgument(value2), convertArgument(value3));
	}

	static std::wstring format_4(const std::wstring& id, const Any& value1, const Any& value2, const Any& value3, const Any& value4)
	{
		return Format(id, convertArgument(value1), convertArgument(value2), convertArgument(value3), convertArgument(value4));
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.I18N", I18NWrapper, Object)

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.i18n.I18NClassFactory", 0, I18NClassFactory, IRuntimeClassFactory)

void I18NClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< Dictionary > > classDictionary = new AutoRuntimeClass< Dictionary >();
	classDictionary->addConstructor();
	classDictionary->addMethod("has", &Dictionary::has);
	classDictionary->addMethod("set", &Dictionary::set);
	classDictionary->addMethod("get", &i18n_Dictionary_get);
	registrar->registerClass(classDictionary);

	Ref< AutoRuntimeClass< I18NWrapper > > classI18N = new AutoRuntimeClass< I18NWrapper >();
	classI18N->addStaticMethod("appendDictionary", &I18NWrapper::appendDictionary);
	classI18N->addStaticMethod("text", &I18NWrapper::text);
	classI18N->addStaticMethod("format", &I18NWrapper::format_1);
	classI18N->addStaticMethod("format", &I18NWrapper::format_2);
	classI18N->addStaticMethod("format", &I18NWrapper::format_3);
	classI18N->addStaticMethod("format", &I18NWrapper::format_4);
	registrar->registerClass(classI18N);
}

	}
}
