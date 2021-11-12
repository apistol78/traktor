#include "Store/Editor/PublishWizardDialog.h"
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/EditList.h"
#include "Ui/EditListEditEvent.h"
#include "Ui/RichEdit/RichEdit.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor
{
    namespace store
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.store.PublishWizardDialog", PublishWizardDialog, ui::ConfigDialog)

PublishWizardDialog::PublishWizardDialog()
{
}

bool PublishWizardDialog::create(ui::Widget* parent, const std::wstring& instanceName)
{
    if (!ui::ConfigDialog::create(
        parent,
        L"Publish package",
        ui::dpi96(600),
        ui::dpi96(600),
        ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable | ui::ConfigDialog::WsOkCancelButtons,
        new ui::TableLayout(L"100%", L"100%", 0, 0)
    ))
        return false;

    Ref< ui::Container > containerMeta = new ui::Container();
    containerMeta->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*,*,*,*,*,100%,100%", ui::dpi96(8), ui::dpi96(8)));

    // Package name
    Ref< ui::Static > staticName = new ui::Static();
    staticName->create(containerMeta, L"Package name");

    m_editName = new ui::Edit();
    m_editName->create(containerMeta, instanceName);

    // Author name
    Ref< ui::Static > staticAuthor = new ui::Static();
    staticAuthor->create(containerMeta, L"Author");

    m_editAuthor = new ui::Edit();
    m_editAuthor->create(containerMeta);

    // Author e-mail
    Ref< ui::Static > staticEMail = new ui::Static();
    staticEMail->create(containerMeta, L"E-Mail");

    m_editEMail = new ui::Edit();
    m_editEMail->create(containerMeta);

    // Author phone
    Ref< ui::Static > staticPhone = new ui::Static();
    staticPhone->create(containerMeta, L"Phone");

    m_editPhone = new ui::Edit();
    m_editPhone->create(containerMeta);

    // Author site
    Ref< ui::Static > staticSite = new ui::Static();
    staticSite->create(containerMeta, L"Site");

    m_editSite = new ui::Edit();
    m_editSite->create(containerMeta);

    // Package description
    Ref< ui::Static > staticDescription = new ui::Static();
    staticDescription->create(containerMeta, L"Description");

    m_editDescription = new ui::RichEdit();
    m_editDescription->create(containerMeta);

    // Package tags.
    Ref< ui::Static > staticTags = new ui::Static();
    staticTags->create(containerMeta, L"Tags");

    m_editListTags = new ui::EditList();
    m_editListTags->create(containerMeta, ui::ListBox::WsSingle | ui::EditList::WsAutoAdd | ui::EditList::WsAutoRemove);
	m_editListTags->addEventHandler< ui::EditListEditEvent >(this, &PublishWizardDialog::eventTagEdit);

    update();
    return true;
}

std::wstring PublishWizardDialog::getName() const
{
    return m_editName->getText();
}

std::wstring PublishWizardDialog::getAuthor() const
{
    return m_editAuthor->getText();
}

std::wstring PublishWizardDialog::getEMail() const
{
    return m_editEMail->getText();
}

std::wstring PublishWizardDialog::getPhone() const
{
    return m_editPhone->getText();
}

std::wstring PublishWizardDialog::getSite() const
{
    return m_editSite->getText();
}

std::wstring PublishWizardDialog::getDescription() const
{
    return m_editDescription->getText();
}

std::set< std::wstring > PublishWizardDialog::getTags() const
{
    std::set< std::wstring > tags;
    for (int32_t i = 0; i < m_editListTags->count(); ++i)
        tags.insert(m_editListTags->getItem(i));
    return tags;
}

void PublishWizardDialog::eventTagEdit(ui::EditListEditEvent* event)
{
    // Consume event, always accept changes to tags list.
    event->consume();
}

    }
}