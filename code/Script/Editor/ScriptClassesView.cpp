/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Core/Class/IRuntimeClass.h"
#include "Core/Class/IRuntimeClassFactory.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/TString.h"
#include "Script/Editor/ScriptClassesView.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/TreeView/TreeView.h"

namespace traktor
{
	namespace script
	{
		namespace
		{

class CollectClassRegistrar : public IRuntimeClassRegistrar
{
public:
	CollectClassRegistrar(ui::custom::TreeView* treeClasses)
	:	m_treeClasses(treeClasses)
	{
	}

	virtual void registerClass(IRuntimeClass* runtimeClass)
	{
		StringOutputStream ss;

		std::vector< std::wstring > qname;
		Split< std::wstring >::any(
			runtimeClass->getExportType().getName(),
			L".",
			qname
		);

		for (int32_t i = 0; i < qname.size() - 1; ++i)
		{
			if (!m_namespaceItems[qname[i]])
				m_namespaceItems[qname[i]] = m_treeClasses->createItem(
					(i > 0) ? m_namespaceItems[qname[i - 1]] : 0,
					qname[i],
					0
				);
		}

		if (runtimeClass->getExportType().getSuper())
			ss << qname.back() << L" : " << runtimeClass->getExportType().getSuper()->getName();
		else
			ss << qname.back();

		Ref< ui::custom::TreeViewItem > classItem = m_treeClasses->createItem(
			(qname.size() >= 2) ? m_namespaceItems[qname[qname.size() - 2]] : 0,
			ss.str(),
			0
		);

		if (runtimeClass->getConstructorDispatch())
		{
			ss.reset();
			runtimeClass->getConstructorDispatch()->signature(ss);

			std::vector< std::wstring > polys;
			Split< std::wstring >::any(ss.str(), L";", polys, true);

			for (const auto& poly : polys)
			{
				std::vector< std::wstring > args;
				Split< std::wstring >::any(poly, L",", args, true);

				ss.reset();
				ss << L"(";

				for (size_t i = 0; i < args.size(); ++i)
				{
					if (i > 0)
						ss << L", ";
					ss << args[i];
				}

				ss << L")";

				m_treeClasses->createItem(classItem, ss.str(), 0);
			}
		}

		for (uint32_t i = 0; i < runtimeClass->getConstantCount(); ++i)
		{
			ss.reset();
			ss << mbstows(runtimeClass->getConstantName(i)) << L" = " << runtimeClass->getConstantValue(i).getWideString();
			m_treeClasses->createItem(classItem, ss.str(), 0);
		}

		for (uint32_t i = 0; i < runtimeClass->getPropertiesCount(); ++i)
		{
			ss.reset();
			if (runtimeClass->getPropertyGetDispatch(i))
				runtimeClass->getPropertyGetDispatch(i)->signature(ss);
			else if (runtimeClass->getPropertySetDispatch(i))
				runtimeClass->getPropertySetDispatch(i)->signature(ss);

			std::vector< std::wstring > s;
			Split< std::wstring >::any(ss.str(), L",", s, true);
			T_FATAL_ASSERT(s.size() >= 1);

			ss.reset();
			ss << s[0] << L" " << mbstows(runtimeClass->getPropertyName(i));

			m_treeClasses->createItem(classItem, ss.str(), 0);
		}

		for (uint32_t i = 0; i < runtimeClass->getMethodCount(); ++i)
		{
			ss.reset();
			runtimeClass->getMethodDispatch(i)->signature(ss);

			std::vector< std::wstring > s;
			Split< std::wstring >::any(ss.str(), L",", s, true);
			T_FATAL_ASSERT (s.size() >= 1);

			ss.reset();
			ss << s[0] << L" " << mbstows(runtimeClass->getMethodName(i)) << L"(";
			for (size_t j = 1; j < s.size(); ++j)
				ss << (j > 1 ? L", " : L"") << s[j];
			ss << L")";

			m_treeClasses->createItem(classItem, ss.str(), 0);
		}

		for (uint32_t i = 0; i < runtimeClass->getStaticMethodCount(); ++i)
		{
			ss.reset();
			runtimeClass->getStaticMethodDispatch(i)->signature(ss);

			std::vector< std::wstring > s;
			Split< std::wstring >::any(ss.str(), L",", s, true);
			T_FATAL_ASSERT (s.size() >= 1);

			ss.reset();
			ss << L"static " << s[0] << L" " << mbstows(runtimeClass->getStaticMethodName(i)) << L"(";
			for (size_t j = 1; j < s.size(); ++j)
				ss << (j > 1 ? L", " : L"") << s[j];
			ss << L")";

			m_treeClasses->createItem(classItem, ss.str(), 0);
		}
	}

private:
	ui::custom::TreeView* m_treeClasses;
	std::map< std::wstring, Ref< ui::custom::TreeViewItem > > m_namespaceItems;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptClassesView", ScriptClassesView, ui::Container)

bool ScriptClassesView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0)))
		return false;

	m_treeClasses = new ui::custom::TreeView();
	m_treeClasses->create(this, ui::WsDoubleBuffer);

	CollectClassRegistrar registrar(m_treeClasses);

	std::set< const TypeInfo* > runtimeClassFactoryTypes;
	type_of< IRuntimeClassFactory >().findAllOf(runtimeClassFactoryTypes, false);
	for (std::set< const TypeInfo* >::const_iterator i = runtimeClassFactoryTypes.begin(); i != runtimeClassFactoryTypes.end(); ++i)
	{
		Ref< IRuntimeClassFactory > runtimeClassFactory = dynamic_type_cast< IRuntimeClassFactory* >((*i)->createInstance());
		if (runtimeClassFactory)
			runtimeClassFactory->createClasses(&registrar);
	}

	return true;
}

void ScriptClassesView::destroy()
{
	safeDestroy(m_treeClasses);
	ui::Container::destroy();
}

	}
}
