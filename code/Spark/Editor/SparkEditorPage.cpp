#pragma optimize( "", off )

#include "Core/Misc/SafeDestroy.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Render/IRenderSystem.h"
#include "Render/Resource/ShaderFactory.h"
#include "Resource/ResourceManager.h"
#include "Spark/Character.h"
#include "Spark/ShapeResourceFactory.h"
#include "Spark/Editor/SparkEditControl.h"
#include "Spark/Editor/SparkEditorPage.h"
#include "Ui/Container.h"

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

	m_resourceManager = new resource::ResourceManager(true);
	m_resourceManager->addFactory(new render::ShaderFactory(database, renderSystem));
	m_resourceManager->addFactory(new ShapeResourceFactory(database, renderSystem));

	m_editControl = new SparkEditControl(m_editor);
	m_editControl->create(parent, ui::WsNone, database, m_resourceManager, renderSystem);
	m_editControl->update();

	Ref< Character > character = m_document->getObject< Character >(0);
	m_editControl->setRootCharacter(character);

	m_site->setPropertyObject(character);
	return true;
}

void SparkEditorPage::destroy()
{
	safeDestroy(m_editControl);
}

void SparkEditorPage::activate()
{
}

void SparkEditorPage::deactivate()
{
}

bool SparkEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	return false;
}

bool SparkEditorPage::handleCommand(const ui::Command& command)
{
	return false;
}

void SparkEditorPage::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->reload(eventId, false);

	m_editControl->update();
}

	}
}