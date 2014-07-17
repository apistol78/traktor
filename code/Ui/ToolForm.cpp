#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/ToolForm.h"
#include "Ui/Itf/IToolForm.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolForm", ToolForm, Container)

bool ToolForm::create(Widget* parent, const std::wstring& text, int width, int height, int style, Layout* layout)
{
	IToolForm* toolForm = Application::getInstance()->getWidgetFactory()->createToolForm(this);
	if (!toolForm)
	{
		log::error << L"Failed to create native widget peer (ToolForm)" << Endl;
		return false;
	}

	Ref< Layout > refLayout = layout;

	if (!toolForm->create(parent ? parent->getIWidget() : 0, text, width, height, style))
	{
		toolForm->destroy();
		return false;
	}

	m_widget = toolForm;

	return Container::create(parent, style, refLayout);
}

void ToolForm::center()
{
	static_cast< IToolForm* >(m_widget)->center();
}

bool ToolForm::acceptLayout() const
{
	return false;
}

	}
}
