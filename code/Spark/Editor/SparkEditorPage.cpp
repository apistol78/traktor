#include "Core/Class/IRuntimeClassFactory.h"
#include "Core/Class/OrderedClassRegistrar.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/ResourceManager.h"
#include "Spark/CharacterResourceFactory.h"
#include "Spark/FontResourceFactory.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"
#include "Spark/ShapeResourceFactory.h"
#include "Spark/Editor/SparkEditControl.h"
#include "Spark/Editor/SparkEditorPage.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

const struct ViewSize
{
	int32_t width;
	int32_t height;
	const wchar_t* description;
}
c_viewSizes[] =
{
	{ 320, 480, L"320 * 480 (iPhone)" },
	{ 640, 960, L"640 * 960 (iPhone Retina)" },
	{ 640, 1136, L"640 * 1136 (iPhone 5)" },
	{ 750, 1334, L"750 * 1334 (iPhone 6)" },
	{ 1080, 1920, L"1080 * 1920 (iPhone 6+)" },
	{ 768, 1024, L"768 * 1024 (iPad)" },
	{ 1536, 2048, L"1536 * 2048 (iPad Retina)" }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SparkEditorPage", SparkEditorPage, editor::IEditorPage)

SparkEditorPage::SparkEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document)
:	m_editor(editor)
,	m_site(site)
,	m_document(document)
{
}

bool SparkEditorPage::create(ui::Container* parent)
{
	Ref< render::IRenderSystem > renderSystem = m_editor->getStoreObject< render::IRenderSystem >(L"RenderSystem");
	if (!renderSystem)
		return false;

	Ref< db::Database > database = m_editor->getOutputDatabase();

	m_resourceManager = new resource::ResourceManager(true);
	m_resourceManager->addFactory(new render::ShaderFactory(database, renderSystem));
	m_resourceManager->addFactory(new render::TextureFactory(database, renderSystem, 0));
	m_resourceManager->addFactory(new CharacterResourceFactory(database));
	m_resourceManager->addFactory(new FontResourceFactory(database, renderSystem));
	m_resourceManager->addFactory(new ShapeResourceFactory(database, renderSystem));

	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(container, ui::WsNone);

	m_toolViewSize = new ui::custom::ToolBarDropDown(ui::Command(L"Spark.Editor.ViewSize"), ui::scaleBySystemDPI(220), i18n::Text(L"SPARK_EDITOR_VIEW_SIZE"));
	for (int32_t i = 0; i < sizeof_array(c_viewSizes); ++i)
		m_toolViewSize->add(c_viewSizes[i].description);
	m_toolViewSize->select(0);

	m_toolViewLandscape = new ui::custom::ToolBarButton(L"Landscape", ui::Command(L"Spark.Editor.ViewSize"), ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggle);

	m_toolBar->addItem(m_toolViewSize);
	m_toolBar->addItem(m_toolViewLandscape);

	m_toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &SparkEditorPage::eventToolClick);

	m_editControl = new SparkEditControl(m_editor, m_site);
	m_editControl->create(container, ui::WsNone, database, m_resourceManager, renderSystem);
	m_editControl->setViewSize(c_viewSizes[0].width, c_viewSizes[0].height);
	m_editControl->update();

	m_panelPlace = new ui::Container();
	m_panelPlace->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0));
	m_panelPlace->setText(i18n::Text(L"SPARK_EDITOR_CHILDREN"));

	m_gridPlace = new ui::custom::GridView();
	m_gridPlace->create(m_panelPlace, ui::WsDoubleBuffer);
	m_gridPlace->addColumn(new ui::custom::GridColumn(i18n::Text(L"SPARK_EDITOR_PLACE_ITEM_NAME"), 200));
	m_gridPlace->addColumn(new ui::custom::GridColumn(L"", 30));
	m_gridPlace->addColumn(new ui::custom::GridColumn(L"", 30));

	m_site->createAdditionalPanel(m_panelPlace, ui::scaleBySystemDPI(300), false);

	m_panelLibrary = new ui::Container();
	m_panelLibrary->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0));
	m_panelLibrary->setText(i18n::Text(L"SPARK_EDITOR_LIBRARY"));

	m_gridLibrary = new ui::custom::GridView();
	m_gridLibrary->create(m_panelLibrary, ui::WsDoubleBuffer);
	m_gridLibrary->addColumn(new ui::custom::GridColumn(i18n::Text(L"SPARK_EDITOR_LIBRARY_ITEM_NAME"), 200));
	m_gridLibrary->addColumn(new ui::custom::GridColumn(L"", 30));

	m_site->createAdditionalPanel(m_panelLibrary, ui::scaleBySystemDPI(300), false);


	m_sprite = m_document->getObject< Sprite >(0);
	m_spriteInstance = m_sprite ? checked_type_cast< SpriteInstance* >(m_sprite->createInstance(0, m_resourceManager, 0, false)) : 0;

	if (m_sprite)
	{
		const AlignedVector< Sprite::Place >& place = m_sprite->getPlacements();
		for (AlignedVector< Sprite::Place >::const_iterator i = place.begin(); i != place.end(); ++i)
		{
			Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
			row->add(new ui::custom::GridItem(!i->name.empty() ? i->name : i18n::Text(L"SPARK_EDITOR_CHILD_UNNAMED")));
			m_gridPlace->addRow(row);
		}

		const SmallMap< std::wstring, Ref< Character > >& characters = m_sprite->getCharacters();
		for (SmallMap< std::wstring, Ref< Character > >::const_iterator i = characters.begin(); i != characters.end(); ++i)
		{
			Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
			row->add(new ui::custom::GridItem(i->first));
			m_gridLibrary->addRow(row);
		}
	}


	m_editControl->setSprite(m_sprite, m_spriteInstance);
	m_site->setPropertyObject(m_sprite);

	return true;
}

void SparkEditorPage::destroy()
{
	// Destroy panels.
	m_site->destroyAdditionalPanel(m_panelLibrary);
	m_site->destroyAdditionalPanel(m_panelPlace);

	// Destroy widgets.
	safeDestroy(m_panelLibrary);
	safeDestroy(m_panelPlace);
	safeDestroy(m_editControl);
	safeDestroy(m_resourceManager);
}

void SparkEditorPage::activate()
{
}

void SparkEditorPage::deactivate()
{
}

bool SparkEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false; //m_editControl->dropInstance(instance, position);
}

bool SparkEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.PropertiesChanged")
	{
		m_spriteInstance = m_sprite ? checked_type_cast< SpriteInstance* >(m_sprite->createInstance(0, m_resourceManager, 0, false)) : 0;
		m_editControl->setSprite(m_sprite, m_spriteInstance);
	}
	else
		return false;

	return true;
}

void SparkEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->reload(eventId, false);
}

void SparkEditorPage::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	if (event->getCommand() == L"Spark.Editor.ViewSize")
	{
		int32_t index = m_toolViewSize->getSelected();
		if (!m_toolViewLandscape->isToggled())
			m_editControl->setViewSize(c_viewSizes[index].width, c_viewSizes[index].height);
		else
			m_editControl->setViewSize(c_viewSizes[index].height, c_viewSizes[index].width);
	}
}

	}
}