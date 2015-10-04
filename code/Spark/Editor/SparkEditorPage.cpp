#include "Core/Class/IRuntimeClassFactory.h"
#include "Core/Class/OrderedClassRegistrar.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
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
#include "Spark/Editor/CharacterAdapter.h"
#include "Spark/Editor/Context.h"
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
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

namespace traktor
{
	namespace spark
	{

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

	// Create resource manager.
	m_resourceManager = new resource::ResourceManager(true);
	m_resourceManager->addFactory(new render::ShaderFactory(database, renderSystem));
	m_resourceManager->addFactory(new render::TextureFactory(database, renderSystem, 0));
	m_resourceManager->addFactory(new CharacterResourceFactory(database));
	m_resourceManager->addFactory(new FontResourceFactory(database, renderSystem));
	m_resourceManager->addFactory(new ShapeResourceFactory(database, renderSystem));

	// Create editor context.
	m_context = new Context(m_resourceManager);
	if (!m_context->setSprite(m_document->getObject< Sprite >(0)))
		return false;

	// Create user interface.
	Ref< ui::Container > container = new ui::Container();
	container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(container, ui::WsNone);

	m_toolBar->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &SparkEditorPage::eventToolClick);

	m_editControl = new SparkEditControl(m_editor, m_site, m_context);
	m_editControl->create(container, ui::WsNone, m_resourceManager, renderSystem);
	m_editControl->update();

	m_panelPlace = new ui::Container();
	m_panelPlace->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0));
	m_panelPlace->setText(i18n::Text(L"SPARK_EDITOR_CHILDREN"));

	m_gridPlace = new ui::custom::GridView();
	m_gridPlace->create(m_panelPlace, ui::WsDoubleBuffer);
	m_gridPlace->addColumn(new ui::custom::GridColumn(i18n::Text(L"SPARK_EDITOR_PLACE_ITEM_NAME"), 200));
	m_gridPlace->addColumn(new ui::custom::GridColumn(L"", 30));
	m_gridPlace->addColumn(new ui::custom::GridColumn(L"", 30));
	m_gridPlace->addEventHandler< ui::SelectionChangeEvent >(this, &SparkEditorPage::eventGridAdapterSelectionChange);

	m_site->createAdditionalPanel(m_panelPlace, ui::scaleBySystemDPI(300), false);

	m_panelLibrary = new ui::Container();
	m_panelLibrary->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%", 0, 0));
	m_panelLibrary->setText(i18n::Text(L"SPARK_EDITOR_LIBRARY"));

	m_gridLibrary = new ui::custom::GridView();
	m_gridLibrary->create(m_panelLibrary, ui::WsDoubleBuffer);
	m_gridLibrary->addColumn(new ui::custom::GridColumn(i18n::Text(L"SPARK_EDITOR_LIBRARY_ITEM_NAME"), 200));
	m_gridLibrary->addColumn(new ui::custom::GridColumn(L"", 30));

	m_site->createAdditionalPanel(m_panelLibrary, ui::scaleBySystemDPI(300), false);

	updateAdaptersGrid();

	m_site->setPropertyObject(m_document->getObject< Sprite >(0));
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
		//m_spriteInstance = m_sprite ? checked_type_cast< SpriteInstance* >(m_characterBuilder->create(m_sprite, 0)) : 0;
		//m_editControl->setSprite(m_sprite, m_spriteInstance);
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

void SparkEditorPage::updateAdaptersGrid(ui::custom::GridRow* parentRow, CharacterAdapter* adapter)
{
	const RefArray< CharacterAdapter >& children = adapter->getChildren();
	for (RefArray< CharacterAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		Ref< ui::custom::GridRow > row = new ui::custom::GridRow(0);
		row->add(new ui::custom::GridItem(!(*i)->getName().empty() ? (*i)->getName() : L"Unnamed"));
		row->add(new ui::custom::GridItem(type_name((*i)->getCharacter())));
		row->setData(L"ADAPTER", *i);

		if (!(*i)->getChildren().empty())
			updateAdaptersGrid(row, *i);

		if (parentRow)
			parentRow->addChild(row);
		else
			m_gridPlace->addRow(row);
	}
}

void SparkEditorPage::updateAdaptersGrid()
{
	m_gridPlace->removeAllRows();
	updateAdaptersGrid(0, m_context->getRoot());
	m_gridPlace->update();
}

void SparkEditorPage::eventToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
}

void SparkEditorPage::eventGridAdapterSelectionChange(ui::SelectionChangeEvent* event)
{
}

	}
}