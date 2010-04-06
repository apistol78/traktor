#include "Core/Misc/String.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Online/Editor/AchievementDesc.h"
#include "Online/Editor/AchievementsAsset.h"
#include "Online/Editor/AchievementsEditorPage.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/MethodHandler.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridItem.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.AchievementsEditorPage", AchievementsEditorPage, editor::IEditorPage)

AchievementsEditorPage::AchievementsEditorPage(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool AchievementsEditorPage::create(ui::Container* parent, editor::IEditorPageSite* site)
{
	m_gridAchievements = new ui::custom::GridView();
	m_gridAchievements->create(parent, ui::custom::GridView::WsColumnHeader | ui::WsDoubleBuffer);
	m_gridAchievements->addColumn(new ui::custom::GridColumn(L"Id", 150));
	m_gridAchievements->addColumn(new ui::custom::GridColumn(L"Image", 150));
	m_gridAchievements->addColumn(new ui::custom::GridColumn(L"Name", 200));
	m_gridAchievements->addColumn(new ui::custom::GridColumn(L"Description", 500));
	m_gridAchievements->addColumn(new ui::custom::GridColumn(L"Hidden", 50));
	return true;
}

void AchievementsEditorPage::destroy()
{
	m_gridAchievements->destroy();
}

void AchievementsEditorPage::activate()
{
}

void AchievementsEditorPage::deactivate()
{
}

bool AchievementsEditorPage::setDataObject(db::Instance* instance, Object* data)
{
	Ref< AchievementsAsset > asset = dynamic_type_cast< AchievementsAsset* >(data);
	if (!asset)
		return false;

	m_gridAchievements->removeAllRows();

	const RefArray< const AchievementDesc >& achievements = asset->get();
	for (RefArray< const AchievementDesc >::const_iterator i = achievements.begin(); i != achievements.end(); ++i)
	{
		Ref< ui::Bitmap > bitmap;

		Ref< drawing::Image > image = drawing::Image::load((*i)->getImagePath());
		if (image)
		{
			drawing::ScaleFilter scaleFilter(64, 64, drawing::ScaleFilter::MnAverage, drawing::ScaleFilter::MgLinear);
			image = image->applyFilter(&scaleFilter);
			bitmap = new ui::Bitmap(image);
		}

		Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
		row->add(new ui::custom::GridItem((*i)->getId()));
		row->add(new ui::custom::GridItem(bitmap));
		row->add(new ui::custom::GridItem((*i)->getName()));
		row->add(new ui::custom::GridItem((*i)->getDescription()));
		m_gridAchievements->addRow(row);
	}

	return true;
}

Ref< db::Instance > AchievementsEditorPage::getDataInstance()
{
	return 0;
}

Ref< Object > AchievementsEditorPage::getDataObject()
{
	return 0;
}

bool AchievementsEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool AchievementsEditorPage::handleCommand(const ui::Command& command)
{
	return false;
}

void AchievementsEditorPage::handleDatabaseEvent(const Guid& eventId)
{
}

	}
}
