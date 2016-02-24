#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Clipboard.h"
#include "Ui/Form.h"
#include "Ui/StyleSheet.h"
#include "Ui/TableLayout.h"
#include "Ui/UiClassFactory.h"

namespace traktor
{
	namespace ui
	{

class BoxedSize : public Object
{
	T_RTTI_CLASS;

public:
	BoxedSize()
	{
	}

	BoxedSize(const Size& sz)
	:	m_sz(sz)
	{
	}

	const Size& unbox() const { return m_sz; }

private:
	Size m_sz;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Size", BoxedSize, Object)

class BoxedPoint : public Object
{
	T_RTTI_CLASS;

public:
	BoxedPoint()
	{
	}

	BoxedPoint(const Point& pnt)
	:	m_pnt(pnt)
	{
	}

	const Point& unbox() const { return m_pnt; }

private:
	Point m_pnt;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Point", BoxedPoint, Object)

class BoxedRect : public Object
{
	T_RTTI_CLASS;

public:
	BoxedRect()
	{
	}

	BoxedRect(const Rect& rc)
	:	m_rc(rc)
	{
	}

	const Rect& unbox() const { return m_rc; }

private:
	Rect m_rc;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Rect", BoxedRect, Object)

	}

template < >
struct CastAny < ui::Size, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.ui.Size";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< ui::BoxedSize >(value.getObjectUnsafe());
	}
	static Any set(const ui::Size& value) {
		return Any::fromObject(new ui::BoxedSize(value));
	}
	static const ui::Size& get(const Any& value) {
		return mandatory_non_null_type_cast< ui::BoxedSize* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const ui::Size&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.ui.Size&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< ui::BoxedSize >(value.getObjectUnsafe());
	}
	static Any set(const ui::Size& value) {
		return Any::fromObject(new ui::BoxedSize(value));
	}
	static const ui::Size& get(const Any& value) {
		return mandatory_non_null_type_cast< ui::BoxedSize* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < ui::Point, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.ui.Point";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< ui::BoxedPoint >(value.getObjectUnsafe());
	}
	static Any set(const ui::Point& value) {
		return Any::fromObject(new ui::BoxedPoint(value));
	}
	static const ui::Point& get(const Any& value) {
		return mandatory_non_null_type_cast< ui::BoxedPoint* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const ui::Point&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.ui.Point&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< ui::BoxedPoint >(value.getObjectUnsafe());
	}
	static Any set(const ui::Point& value) {
		return Any::fromObject(new ui::BoxedPoint(value));
	}
	static const ui::Point& get(const Any& value) {
		return mandatory_non_null_type_cast< ui::BoxedPoint* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < ui::Rect, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"traktor.ui.Rect";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< ui::BoxedRect >(value.getObjectUnsafe());
	}
	static Any set(const ui::Rect& value) {
		return Any::fromObject(new ui::BoxedRect(value));
	}
	static const ui::Rect& get(const Any& value) {
		return mandatory_non_null_type_cast< ui::BoxedRect* >(value.getObject())->unbox();
	}
};

template < >
struct CastAny < const ui::Rect&, false >
{
	static OutputStream& typeName(OutputStream& ss) {
		return ss << L"const traktor.ui.Rect&";
	}
	static bool accept(const Any& value) {
		return value.isObject() && is_a< ui::BoxedRect >(value.getObjectUnsafe());
	}
	static Any set(const ui::Rect& value) {
		return Any::fromObject(new ui::BoxedRect(value));
	}
	static const ui::Rect& get(const Any& value) {
		return mandatory_non_null_type_cast< ui::BoxedRect* >(value.getObject())->unbox();
	}
};

	namespace ui
	{
		namespace
		{

class RuntimeEventHandler : public RefCountImpl< EventSubject::IEventHandler >
{
public:
	RuntimeEventHandler(IRuntimeDelegate* runtimeDelegate)
	:	m_runtimeDelegate(runtimeDelegate)
	{
	}

	virtual void notify(Event* event)
	{
		Any argv[] =
		{
			Any::fromObject(event)
		};
		m_runtimeDelegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< IRuntimeDelegate > m_runtimeDelegate;
};

void EventSubject_addEventHandler(EventSubject* self, const TypeInfo& eventType, IRuntimeDelegate* eventHandler)
{
	self->addEventHandler(eventType, new RuntimeEventHandler(eventHandler));
}

bool Form_create_3(Form* self, const std::wstring& text, int width, int height)
{
	return self->create(text, width, height);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ui.UiClassFactory", 0, UiClassFactory, IRuntimeClassFactory)

void UiClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< Application > > classApplication = new AutoRuntimeClass< Application >();
	classApplication->addStaticMethod("getInstance", &Application::getInstance);
	//classApplication->addMethod("initialize", &Application::initialize);
	classApplication->addMethod("finalize", &Application::finalize);
	classApplication->addMethod("process", &Application::process);
	classApplication->addMethod("execute", &Application::execute);
	classApplication->addMethod("exit", &Application::exit);
	//classApplication->addMethod("getEventLoop", &Application::getEventLoop);
	//classApplication->addMethod("getWidgetFactory", &Application::getWidgetFactory);
	classApplication->addMethod("getClipboard", &Application::getClipboard);
	classApplication->addMethod("getStyleSheet", &Application::getStyleSheet);
	//classApplication->addMethod("translateVirtualKey", &Application::translateVirtualKey);
	//classApplication->addMethod("translateVirtualKey", &Application::translateVirtualKey);
	registrar->registerClass(classApplication);

	Ref< AutoRuntimeClass< Event > > classEvent = new AutoRuntimeClass< Event >();
	classEvent->addConstructor< EventSubject* >();
	classEvent->addMethod("getSender", &Event::getSender);
	classEvent->addMethod("getKeyState", &Event::getKeyState);
	classEvent->addMethod("consume", &Event::consume);
	classEvent->addMethod("consumed", &Event::consumed);
	registrar->registerClass(classEvent);

	Ref< AutoRuntimeClass< EventSubject > > classEventSubject = new AutoRuntimeClass< EventSubject >();
	classEventSubject->addMethod("raiseEvent", &EventSubject::raiseEvent);
	classEventSubject->addMethod("addEventHandler", &EventSubject_addEventHandler);
	//classEventSubject->addMethod("removeEventHandler", &EventSubject_removeEventHandler);
	//classEventSubject->addMethod("hasEventHandler", &EventSubject_hasEventHandler);
	registrar->registerClass(classEventSubject);

	Ref< AutoRuntimeClass< Layout > > classLayout = new AutoRuntimeClass< Layout >();
	registrar->registerClass(classLayout);

	Ref< AutoRuntimeClass< TableLayout > > classTableLayout = new AutoRuntimeClass< TableLayout >();
	classTableLayout->addConstructor< const std::wstring&, const std::wstring&, int, int >();
	registrar->registerClass(classTableLayout);

	Ref< AutoRuntimeClass< Widget > > classWidget = new AutoRuntimeClass< Widget >();
	//classWidget->addMethod("create", &Widget_create_1);
	//classWidget->addMethod("create", &Widget_create_2);
	classWidget->addMethod("destroy", &Widget::destroy);
	classWidget->addMethod("setText", &Widget::setText);
	classWidget->addMethod("getText", &Widget::getText);
	classWidget->addMethod("setToolTipText", &Widget::setToolTipText);
	classWidget->addMethod("setForeground", &Widget::setForeground);
	classWidget->addMethod("isForeground", &Widget::isForeground);
	classWidget->addMethod("setVisible", &Widget::setVisible);
	classWidget->addMethod("isVisible", &Widget::isVisible);
	classWidget->addMethod("setEnable", &Widget::setEnable);
	classWidget->addMethod("isEnable", &Widget::isEnable);
	classWidget->addMethod("hasFocus", &Widget::hasFocus);
	classWidget->addMethod("containFocus", &Widget::containFocus);
	classWidget->addMethod("setFocus", &Widget::setFocus);
	classWidget->addMethod("setRect", &Widget::setRect);
	classWidget->addMethod("getRect", &Widget::getRect);
	classWidget->addMethod("getInnerRect", &Widget::getInnerRect);
	classWidget->addMethod("getNormalRect", &Widget::getNormalRect);
	classWidget->addMethod("getTextExtent", &Widget::getTextExtent);
	//classWidget->addMethod("setFont", &Widget::setFont);
	//classWidget->addMethod("getFont", &Widget::getFont);
	//classWidget->addMethod("setCursor", &Widget::setCursor);
	//classWidget->addMethod("resetCursor", &Widget::resetCursor);
	//classWidget->addMethod("update", &Widget_update_0);
	//classWidget->addMethod("update", &Widget_update_1);
	//classWidget->addMethod("update", &Widget_update_2);
	classWidget->addMethod("show", &Widget::show);
	classWidget->addMethod("hide", &Widget::hide);
	classWidget->addMethod("raise", &Widget::raise);
	classWidget->addMethod("lower", &Widget::lower);
	classWidget->addMethod("hasCapture", &Widget::hasCapture);
	classWidget->addMethod("setCapture", &Widget::setCapture);
	classWidget->addMethod("releaseCapture", &Widget::releaseCapture);
	//classWidget->addMethod("startTimer", &Widget_startTimer_1);
	//classWidget->addMethod("startTimer", &Widget_startTimer_2);
	//classWidget->addMethod("stopTimer", &Widget_stopTimer_0);
	//classWidget->addMethod("stopTimer", &Widget_stopTimer_1);
	//classWidget->addMethod("setOutline", &Widget::setOutline);
	//classWidget->addMethod("getMousePosition", &Widget_getMousePosition_0);
	//classWidget->addMethod("getMousePosition", &Widget_getMousePosition_1);
	classWidget->addMethod("screenToClient", &Widget::screenToClient);
	classWidget->addMethod("clientToScreen", &Widget::clientToScreen);
	classWidget->addMethod("hitTest", &Widget::hitTest);
	//classWidget->addMethod("setChildRects", &Widget::setChildRects);
	classWidget->addMethod("getMinimumSize", &Widget::getMinimumSize);
	classWidget->addMethod("getPreferedSize", &Widget::getPreferedSize);
	classWidget->addMethod("getMaximumSize", &Widget::getMaximumSize);
	classWidget->addMethod("acceptLayout", &Widget::acceptLayout);
	classWidget->addMethod("link", &Widget::link);
	classWidget->addMethod("unlink", &Widget::unlink);
	classWidget->addMethod("setParent", &Widget::setParent);
	classWidget->addMethod("getParent", &Widget::getParent);
	classWidget->addMethod("getPreviousSibling", &Widget::getPreviousSibling);
	classWidget->addMethod("getNextSibling", &Widget::getNextSibling);
	classWidget->addMethod("getFirstChild", &Widget::getFirstChild);
	classWidget->addMethod("getLastChild", &Widget::getLastChild);
	//classWidget->addMethod("removeAllData", &Widget_removeAllData);
	//classWidget->addMethod("setData", &Widget_setData);
	//classWidget->addMethod("getData", &Widget_getData);
	registrar->registerClass(classWidget);

	Ref< AutoRuntimeClass< Container > > classContainer = new AutoRuntimeClass< Container >();
	classContainer->addConstructor();
	classContainer->addMethod("fit", &Container::fit);
	classContainer->addMethod("getLayout", &Container::getLayout);
	classContainer->addMethod("setLayout", &Container::setLayout);
	registrar->registerClass(classContainer);

	Ref< AutoRuntimeClass< Form > > classForm = new AutoRuntimeClass< Form >();
	classForm->addConstructor();
	classForm->addMethod("create", &Form_create_3);
	classForm->addMethod("setIcon", &Form::setIcon);
	classForm->addMethod("maximize", &Form::maximize);
	classForm->addMethod("minimize", &Form::minimize);
	classForm->addMethod("restore", &Form::restore);
	classForm->addMethod("isMaximized", &Form::isMaximized);
	classForm->addMethod("isMinimized", &Form::isMinimized);
	classForm->addMethod("setIcon", &Form::setIcon);
	registrar->registerClass(classForm);
}

	}
}
