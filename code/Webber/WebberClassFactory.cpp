/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Core/Class/IRuntimeObject.h"
#include "Core/Io/IStream.h"
#include "Html/Element.h"
#include "Webber/Button.h"
#include "Webber/Canvas.h"
#include "Webber/Container.h"
#include "Webber/Event.h"
#include "Webber/Float.h"
#include "Webber/HttpRequestListener.h"
#include "Webber/IApplication.h"
#include "Webber/Splitter.h"
#include "Webber/Static.h"
#include "Webber/TreeView.h"
#include "Webber/TreeViewItem.h"
#include "Webber/TextArea.h"
#include "Webber/WebberClassFactory.h"

namespace traktor
{
	namespace wbr
	{
		namespace
		{

class Application : public IApplication
{
public:
	Application(IRuntimeObject* app)
	:	m_app(app)
	,	m_methodIdGetRootWidget(~0U)
	,	m_methodIdResolve(~0U)
	{
	}

	virtual bool create() T_OVERRIDE T_FINAL
	{
		if (!(m_appClass = m_app->getRuntimeClass()))
			return false;

		m_methodIdGetRootWidget = findRuntimeClassMethodId(m_appClass, "getRootWidget");
		m_methodIdResolve = findRuntimeClassMethodId(m_appClass, "resolve");

		return true;
	}

	virtual void destroy() T_OVERRIDE T_FINAL
	{
		m_appClass = 0;
	}

	virtual Widget* getRootWidget() const T_OVERRIDE T_FINAL
	{
		if (!m_appClass || m_methodIdGetRootWidget == ~0U)
			return 0;

		return CastAny< Widget* >::get(
			m_appClass->invoke(m_app, m_methodIdGetRootWidget, 0, 0)
		);
	}

	virtual Ref< IStream > resolve(const std::wstring& uri) const T_OVERRIDE T_FINAL
	{
		if (!m_appClass || m_methodIdResolve == ~0U)
			return 0;

		Any argv[] =
		{
			CastAny< const std::wstring& >::set(uri)
		};

		return CastAny< IStream* >::get(
			m_appClass->invoke(m_app, m_methodIdResolve, sizeof_array(argv), argv)
		);
	}

private:
	Ref< IRuntimeObject > m_app;
	Ref< const IRuntimeClass > m_appClass;
	uint32_t m_methodIdGetRootWidget;
	uint32_t m_methodIdResolve;
};

Event* Button_get_clicked(Button* self)
{
	return &self->clicked();
}

Ref< Widget > Container_addChild(Container* self, Widget* child)
{
	return self->addChild(child);
}

void Event_add(Event* self, Ref< IRuntimeDelegate > event)
{
	*self += [=]() { event->call(0, 0); };
}

Ref< HttpRequestListener > HttpRequestListener_constructor(Ref< IRuntimeDelegate > newapp, bool verbose)
{
	return new HttpRequestListener(
		[=] () -> Ref< wbr::IApplication > {
			if (!newapp)
				return 0;

			Ref< IRuntimeObject > app = CastAny< IRuntimeObject* >::get(newapp->call(0, 0));
			if (!app)
				return 0;

			return new Application(app);
		},
		verbose
	);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.wbr.WebberClassFactory", 0, WebberClassFactory, IRuntimeClassFactory)

void WebberClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< Button > > classButton = new AutoRuntimeClass< Button >();
	classButton->addConstructor< const std::wstring& >();
	classButton->addProperty< Event* >("clicked", 0, &Button_get_clicked);
	registrar->registerClass(classButton);

	Ref< AutoRuntimeClass< Canvas > > classCanvas = new AutoRuntimeClass< Canvas >();
	classCanvas->addConstructor();
	registrar->registerClass(classCanvas);

	Ref< AutoRuntimeClass< Container > > classContainer = new AutoRuntimeClass< Container >();
	classContainer->addConstructor();
	classContainer->addMethod("setLayout", &Container::setLayout);
	classContainer->addMethod("addChild", &Container_addChild);
	classContainer->addMethod("removeChild", &Container::removeChild);
	classContainer->addMethod("getChildren", &Container::getChildren);
	registrar->registerClass(classContainer);

	Ref< AutoRuntimeClass< Event > > classEvent = new AutoRuntimeClass< Event >();
	classEvent->addMethod("add", &Event_add);
	registrar->registerClass(classEvent);

	Ref< AutoRuntimeClass< Float > > classFloat = new AutoRuntimeClass< Float >();
	classFloat->addConstructor();
	registrar->registerClass(classFloat);

	Ref< AutoRuntimeClass< HttpRequestListener > > classHttpRequestListener = new AutoRuntimeClass< HttpRequestListener >();
	classHttpRequestListener->addConstructor< Ref< IRuntimeDelegate >, bool >(&HttpRequestListener_constructor);
	registrar->registerClass(classHttpRequestListener);

	Ref< AutoRuntimeClass< IApplication > > classIApplication = new AutoRuntimeClass< IApplication >();
	registrar->registerClass(classIApplication);

	Ref< AutoRuntimeClass< ILayout > > classILayout = new AutoRuntimeClass< ILayout >();
	registrar->registerClass(classILayout);

	Ref< AutoRuntimeClass< Splitter > > classSplitter = new AutoRuntimeClass< Splitter >();
	registrar->registerClass(classSplitter);

	Ref< AutoRuntimeClass< Static > > classStatic = new AutoRuntimeClass< Static >();
	classStatic->addConstructor< const std::wstring& >();
	classStatic->addProperty("text", &Static::setText, &Static::getText);
	registrar->registerClass(classStatic);

	Ref< AutoRuntimeClass< TreeView > > classTreeView = new AutoRuntimeClass< TreeView >();
	classTreeView->addConstructor();
	classTreeView->addMethod("addChild", &TreeView::addChild);
	classTreeView->addMethod("removeChild", &TreeView::removeChild);
	classTreeView->addMethod("getChildren", &TreeView::getChildren);
	registrar->registerClass(classTreeView);

	Ref< AutoRuntimeClass< TreeViewItem > > classTreeViewItem = new AutoRuntimeClass< TreeViewItem >();
	classTreeViewItem->addConstructor< const std::wstring& >();
	classTreeViewItem->addProperty("text", &TreeViewItem::setText, &TreeViewItem::getText);
	classTreeViewItem->addProperty("expanded", &TreeViewItem::setExpanded, &TreeViewItem::isExpanded);
	classTreeViewItem->addMethod("addChild", &TreeViewItem::addChild);
	classTreeViewItem->addMethod("removeChild", &TreeViewItem::removeChild);
	classTreeViewItem->addMethod("getChildren", &TreeViewItem::getChildren);
	registrar->registerClass(classTreeViewItem);

	Ref< AutoRuntimeClass< TextArea > > classTextArea = new AutoRuntimeClass< TextArea >();
	classTextArea->addConstructor();
	classTextArea->addProperty("text", &TextArea::setText, &TextArea::getText);
	registrar->registerClass(classTextArea);

	Ref< AutoRuntimeClass< Widget > > classWidget = new AutoRuntimeClass< Widget >();
	classWidget->addProperty< int32_t >("id", 0, &Widget::getId);
	classWidget->addMethod("build", &Widget::build);
	classWidget->addMethod("consume", &Widget::consume);
	registrar->registerClass(classWidget);
}

	}
}
