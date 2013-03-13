#include "Editor/App/BuildView.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/BuildChart/BuildChartControl.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.BuildView", BuildView, ui::Container)

bool BuildView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0)))
		return false;

	m_chartControl = new ui::custom::BuildChartControl();
	m_chartControl->create(this);

	return true;
}

void BuildView::beginBuild()
{
	m_chartControl->begin();
}

void BuildView::endBuild()
{
	m_chartControl->end();
}

void BuildView::beginBuild(uint32_t core, const std::wstring& name)
{
	m_chartControl->beginTask(core, name);
}

void BuildView::endBuild(uint32_t core)
{
	m_chartControl->endTask(core);
}

	}
}
