#include "Ui/Custom/InputDialog.h"
#include "Ui/TableLayout.h"
#include "Ui/Static.h"
#include "Ui/Edit.h"
#include "Core/Heap/New.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.InputDialog", InputDialog, ConfigDialog)

InputDialog::InputDialog()
:	m_outFields(0)
{
}

bool InputDialog::create(
	Widget* parent,
	const std::wstring& title,
	const std::wstring& message,
	Field* outFields,
	uint32_t outFieldsCount
)
{
	if (!ConfigDialog::create(parent, title, 300, 180, ConfigDialog::WsDefaultFixed, gc_new< TableLayout >(L"100%", L"*,*", 4, 4)))
		return false;

	Ref< Static > labelMessage = gc_new< Static >();
	labelMessage->create(this, message);

	Ref< Container > container = gc_new< Container >();
	container->create(this, WsNone, gc_new< TableLayout >(L"*,100%", L"*", 0, 4));

	m_outFields = outFields;
	for (uint32_t i = 0; i < outFieldsCount; ++i)
	{
		Ref< Static > labelEdit = gc_new< Static >();
		labelEdit->create(container, m_outFields[i].title);

		Ref< Edit > edit = gc_new< Edit >();
		edit->create(container, m_outFields[i].value, WsClientBorder, m_outFields[i].validator);

		m_editFields.push_back(edit);
	}

	fit();

	return true;
}

int InputDialog::showModal()
{
	if (m_editFields.empty())
		return DrCancel;

	Ref< Edit > edit = m_editFields.front();
	edit->setFocus();
	edit->selectAll();

	int result = ConfigDialog::showModal();
	if (result == DrOk && m_outFields)
	{
		for (uint32_t i = 0; i < uint32_t(m_editFields.size()); ++i)
		{
			if (m_editFields[i])
				m_outFields[i].value = m_editFields[i]->getText();
		}
	}
	
	return result;
}

		}
	}
}
