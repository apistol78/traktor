/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <map>
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
#include "Ui/TreeView/TreeView.h"
#include "Ui/TreeView/TreeViewItem.h"

namespace traktor::script
{
	namespace
	{

class CollectClassRegistrar : public IRuntimeClassRegistrar
{
public:
	CollectClassRegistrar(ui::TreeView* treeClasses)
	:	m_treeClasses(treeClasses)
	{
	}

	virtual void registerClass(IRuntimeClass* runtimeClass)
	{
		T_ANONYMOUS_VAR(Ref< IRuntimeClass >)(runtimeClass);
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
					(i > 0) ? m_namespaceItems[qname[i - 1]] : nullptr,
					qname[i],
					0
				);
		}

		if (runtimeClass->getExportType().getSuper())
		{
			std::vector< std::wstring > qnameSuper;
			Split< std::wstring >::any(
				runtimeClass->getExportType().getSuper()->getName(),
				L".",
				qnameSuper
			);
			ss << qname.back() << L" : " << qnameSuper.back();
		}
		else
			ss << qname.back();

		Ref< ui::TreeViewItem > classItem = m_treeClasses->createItem(
			(qname.size() >= 2) ? m_namespaceItems[qname[qname.size() - 2]] : nullptr,
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
	ui::TreeView* m_treeClasses;
	std::map< std::wstring, Ref< ui::TreeViewItem > > m_namespaceItems;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptClassesView", ScriptClassesView, ui::Container)

bool ScriptClassesView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0_ut, 0_ut)))
		return false;

	m_treeClasses = new ui::TreeView();
	m_treeClasses->create(this, ui::WsDoubleBuffer);
	
	CollectClassRegistrar registrar(m_treeClasses);
	for (const auto runtimeClassFactoryType : type_of< IRuntimeClassFactory >().findAllOf(false))
	{
		Ref< IRuntimeClassFactory > runtimeClassFactory = dynamic_type_cast< IRuntimeClassFactory* >(runtimeClassFactoryType->createInstance());
		if (runtimeClassFactory)
			runtimeClassFactory->createClasses(&registrar);
	}

	RefArray< ui::TreeViewItem > items;
	m_treeClasses->getItems(items, ui::TreeView::GfDefault);
	for (auto item : items)
		item->sort(
			true,
			[](const ui::TreeViewItem* item1, const ui::TreeViewItem* item2) -> bool {
				return item1->getText() < item2->getText();
			}
		);

	return true;
}

void ScriptClassesView::destroy()
{
	safeDestroy(m_treeClasses);
	ui::Container::destroy();
}

}
