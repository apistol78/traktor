#include <cstring>
#include "Core/Class/IRuntimeClass.h"
#include "Core/Class/IRuntimeClassFactory.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Editor/App/GenerateClassListTool.h"
#include "I18N/Text.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

class CollectClassRegistrar : public IRuntimeClassRegistrar
{
public:
	CollectClassRegistrar(OutputStream& os)
	:	m_os(os)
	{
	}

	virtual void registerClass(IRuntimeClass* runtimeClass)
	{
		const wchar_t* signature[IRuntimeClass::MaxSignatures];

		if (runtimeClass->getExportType().getSuper())
			m_os << runtimeClass->getExportType().getName() << L" : " << runtimeClass->getExportType().getSuper()->getName() << Endl;
		else
			m_os << runtimeClass->getExportType().getName() << Endl;

		m_os << IncreaseIndent;
		if (runtimeClass->haveConstructor())
			m_os << L"(ctor)" << Endl;
		for (uint32_t i = 0; i < runtimeClass->getConstantCount(); ++i)
			m_os << mbstows(runtimeClass->getConstantName(i)) << L" = " << runtimeClass->getConstantValue(i).getWideString() << Endl;
		for (uint32_t i = 0; i < runtimeClass->getMethodCount(); ++i)
		{
			std::memset(signature, 0, sizeof(signature));
			runtimeClass->getMethodSignature(i, signature);
			m_os << (signature[0] ? signature[0] : L"void") << L" " << mbstows(runtimeClass->getMethodName(i)) << L"(";
			for (uint32_t j = 1; signature[j]; ++j)
				m_os << (j > 1 ? L", " : L"") << signature[j];
			m_os << L")" << Endl;
		}
		for (uint32_t i = 0; i < runtimeClass->getStaticMethodCount(); ++i)
		{
			std::memset(signature, 0, sizeof(signature));
			runtimeClass->getStaticMethodSignature(i, signature);
			m_os << L"static " << (signature[0] ? signature[0] : L"void") << L" " << mbstows(runtimeClass->getStaticMethodName(i)) << L"(";
			for (uint32_t j = 1; signature[j]; ++j)
				m_os << (j > 1 ? L", " : L"") << signature[j];
			m_os << L")" << Endl;
		}
		m_os << DecreaseIndent;

		m_os << Endl;
	}

private:
	OutputStream& m_os;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.GenerateClassListTool", 0, GenerateClassListTool, IEditorTool)

std::wstring GenerateClassListTool::getDescription() const
{
	return i18n::Text(L"EDITOR_GENERATE_CLASS_LIST");
}

bool GenerateClassListTool::launch(ui::Widget* parent, IEditor* editor)
{
	StringOutputStream ss;
	CollectClassRegistrar registrar(ss);

	std::set< const TypeInfo* > runtimeClassFactoryTypes;
	type_of< IRuntimeClassFactory >().findAllOf(runtimeClassFactoryTypes, false);
	for (std::set< const TypeInfo* >::const_iterator i = runtimeClassFactoryTypes.begin(); i != runtimeClassFactoryTypes.end(); ++i)
	{
		Ref< IRuntimeClassFactory > runtimeClassFactory = dynamic_type_cast< IRuntimeClassFactory* >((*i)->createInstance());
		if (runtimeClassFactory)
			runtimeClassFactory->createClasses(&registrar);
	}

	log::info << ss.str();
	return true;
}

	}
}
