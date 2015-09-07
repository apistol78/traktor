#include "Core/Misc/String.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/Event.h"
#include "Ui/FloodLayout.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "World/PostProcess/PostProcessDefineTarget.h"
#include "World/PostProcess/PostProcessDefineTexture.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PostProcess/PostProcessStepBlur.h"
#include "World/PostProcess/PostProcessStepBokeh.h"
#include "World/PostProcess/PostProcessStepChain.h"
#include "World/PostProcess/PostProcessStepGodRay.h"
#include "World/PostProcess/PostProcessStepGrain.h"
#include "World/PostProcess/PostProcessStepLuminance.h"
#include "World/PostProcess/PostProcessStepRepeat.h"
#include "World/PostProcess/PostProcessStepSetTarget.h"
#include "World/PostProcess/PostProcessStepSimple.h"
#include "World/PostProcess/PostProcessStepSmProj.h"
#include "World/PostProcess/PostProcessStepSsao.h"
#include "World/PostProcess/PostProcessStepSwapTargets.h"
#include "World/Editor/PostProcess/PostProcessDefineTargetFacade.h"
#include "World/Editor/PostProcess/PostProcessDefineTextureFacade.h"
#include "World/Editor/PostProcess/PostProcessEditor.h"
#include "World/Editor/PostProcess/PostProcessStepBlurFacade.h"
#include "World/Editor/PostProcess/PostProcessStepBokehFacade.h"
#include "World/Editor/PostProcess/PostProcessStepChainFacade.h"
#include "World/Editor/PostProcess/PostProcessStepGodRayFacade.h"
#include "World/Editor/PostProcess/PostProcessStepGrainFacade.h"
#include "World/Editor/PostProcess/PostProcessStepLuminanceFacade.h"
#include "World/Editor/PostProcess/PostProcessStepRepeatFacade.h"
#include "World/Editor/PostProcess/PostProcessStepSetTargetFacade.h"
#include "World/Editor/PostProcess/PostProcessStepSimpleFacade.h"
#include "World/Editor/PostProcess/PostProcessStepSmProjFacade.h"
#include "World/Editor/PostProcess/PostProcessStepSsaoFacade.h"
#include "World/Editor/PostProcess/PostProcessStepSwapTargetsFacade.h"
#include "World/Editor/PostProcess/PostProcessProperties.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessEditor", PostProcessEditor, editor::IObjectEditor)

PostProcessEditor::PostProcessEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool PostProcessEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	if (!m_instance)
		return false;

	m_asset = dynamic_type_cast< PostProcessSettings* >(object);
	if (!m_asset)
		return false;

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(parent, true, ui::scaleBySystemDPI(400), false);

	Ref< ui::custom::Splitter > splitterView = new ui::custom::Splitter();
	splitterView->create(splitter, false, 60, true);

	Ref< ui::Container > containerSteps = new ui::Container();
	containerSteps->create(splitterView, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolBarSteps = new ui::custom::ToolBar();
	m_toolBarSteps->create(containerSteps);
	m_toolBarSteps->addItem(new ui::custom::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_ADD_STEP"), 0, ui::Command(L"World.PostProcess.AddStep"), ui::custom::ToolBarButton::BsText));
	m_toolBarSteps->addItem(new ui::custom::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_REMOVE_STEP"), 0, ui::Command(L"World.PostProcess.RemoveStep"), ui::custom::ToolBarButton::BsText));
	m_toolBarSteps->addItem(new ui::custom::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_MOVE_UP"), 0, ui::Command(L"World.PostProcess.MoveUp"), ui::custom::ToolBarButton::BsText));
	m_toolBarSteps->addItem(new ui::custom::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_MOVE_DOWN"), 0, ui::Command(L"World.PostProcess.MoveDown"), ui::custom::ToolBarButton::BsText));
	m_toolBarSteps->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &PostProcessEditor::eventStepToolClick);

	m_gridSteps = new ui::custom::GridView();
	m_gridSteps->create(containerSteps, ui::WsDoubleBuffer | ui::custom::GridView::WsColumnHeader);
	m_gridSteps->addColumn(new ui::custom::GridColumn(i18n::Text(L"POSTPROCESS_EDITOR_ORDER"), ui::scaleBySystemDPI(60)));
	m_gridSteps->addColumn(new ui::custom::GridColumn(i18n::Text(L"POSTPROCESS_EDITOR_DESCRIPTION"), ui::scaleBySystemDPI(300)));
	m_gridSteps->addEventHandler< ui::SelectionChangeEvent >(this, &PostProcessEditor::eventGridStepSelect);

	Ref< ui::Container > containerDefinitions = new ui::Container();
	containerDefinitions->create(splitterView, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolBarDefinitions = new ui::custom::ToolBar();
	m_toolBarDefinitions->create(containerDefinitions);
	m_toolBarDefinitions->addItem(new ui::custom::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_ADD_TARGET_DEFINITION"), 0, ui::Command(L"World.PostProcess.AddTargetDefinition"), ui::custom::ToolBarButton::BsText));
	m_toolBarDefinitions->addItem(new ui::custom::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_ADD_TEXTURE_DEFINITION"), 0, ui::Command(L"World.PostProcess.AddTextureDefinition"), ui::custom::ToolBarButton::BsText));
	m_toolBarDefinitions->addItem(new ui::custom::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_REMOVE_DEFINITION"), 0, ui::Command(L"World.PostProcess.RemoveDefinition"), ui::custom::ToolBarButton::BsText));
	m_toolBarDefinitions->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &PostProcessEditor::eventDefinitionToolClick);

	m_gridDefinitions = new ui::custom::GridView();
	m_gridDefinitions->create(containerDefinitions, ui::WsDoubleBuffer | ui::custom::GridView::WsColumnHeader);
	m_gridDefinitions->addColumn(new ui::custom::GridColumn(i18n::Text(L"POSTPROCESS_EDITOR_ID"), ui::scaleBySystemDPI(60)));
	m_gridDefinitions->addColumn(new ui::custom::GridColumn(i18n::Text(L"POSTPROCESS_EDITOR_DESCRIPTION"), ui::scaleBySystemDPI(300)));
	m_gridDefinitions->addEventHandler< ui::SelectionChangeEvent >(this, &PostProcessEditor::eventGridDefinitionSelect);

	Ref< ui::Container > container = new ui::Container();
	container->create(splitter, ui::WsNone, new ui::FloodLayout());

	m_properties = new PostProcessProperties(m_editor);
	m_properties->create(container);
	m_properties->addEventHandler< ui::ContentChangeEvent >(this, &PostProcessEditor::eventStepPropertiesChange);

	// Create define facades.
	m_postProcessDefineFacades[&type_of < PostProcessDefineTarget >()] = new PostProcessDefineTargetFacade();
	m_postProcessDefineFacades[&type_of < PostProcessDefineTexture >()] = new PostProcessDefineTextureFacade();

	// Create step facades.
	m_postProcessStepFacades[&type_of< PostProcessStepBlur >()] = new PostProcessStepBlurFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepBokeh >()] = new PostProcessStepBokehFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepChain >()] = new PostProcessStepChainFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepGodRay >()] = new PostProcessStepGodRayFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepGrain >()] = new PostProcessStepGrainFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepLuminance >()] = new PostProcessStepLuminanceFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepRepeat >()] = new PostProcessStepRepeatFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepSetTarget >()] = new PostProcessStepSetTargetFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepSimple >()] = new PostProcessStepSimpleFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepSmProj >()] = new PostProcessStepSmProjFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepSsao >()] = new PostProcessStepSsaoFacade();
	m_postProcessStepFacades[&type_of< PostProcessStepSwapTargets >()] = new PostProcessStepSwapTargetsFacade();

	updateViews();
	return true;
}

void PostProcessEditor::destroy()
{
}

void PostProcessEditor::apply()
{
	m_instance->setObject(m_asset);
}

ui::Size PostProcessEditor::getPreferredSize() const
{
	return ui::Size(ui::scaleBySystemDPI(900), ui::scaleBySystemDPI(600));
}

bool PostProcessEditor::handleCommand(const ui::Command& command)
{
	if (command == L"World.PostProcess.AddStep")
	{
		const TypeInfo* stepType = m_editor->browseType(&type_of< PostProcessStep >());
		if (stepType)
		{
			Ref< PostProcessStep > step = checked_type_cast< PostProcessStep*, false >(stepType->createInstance());
			RefArray< PostProcessStep > steps = m_asset->getSteps();
			steps.push_back(step);
			m_asset->setSteps(steps);
			updateViews();
		}
	}
	else if (command == L"World.PostProcess.RemoveStep")
	{
		ui::custom::GridRow* selectedRow = m_gridSteps->getSelectedRow();
		if (selectedRow)
		{
			if (selectedRow->getParent() == 0)
			{
				RefArray< PostProcessStep > steps = m_asset->getSteps();
				steps.remove(selectedRow->getData< PostProcessStep >(L"STEP"));
				m_asset->setSteps(steps);
			}
			else
			{

			}
			updateViews();
		}
	}
	else if (command == L"World.PostProcess.MoveUp")
	{
		ui::custom::GridRow* selectedRow = m_gridSteps->getSelectedRow();
		if (selectedRow)
		{
			if (selectedRow->getParent() == 0)
			{
				RefArray< PostProcessStep > steps = m_asset->getSteps();
				for (uint32_t i = 1; i < steps.size(); ++i)
				{
					if (steps[i] == selectedRow->getData< PostProcessStep >(L"STEP"))
					{
						Ref< PostProcessStep > r = steps[i];
						steps[i] = steps[i - 1];
						steps[i - 1] = r;
						break;
					}
				}
				m_asset->setSteps(steps);
			}
			else
			{

			}
			updateViews();
		}
	}
	else if (command == L"World.PostProcess.MoveDown")
	{
		ui::custom::GridRow* selectedRow = m_gridSteps->getSelectedRow();
		if (selectedRow)
		{
			if (selectedRow->getParent() == 0)
			{
				RefArray< PostProcessStep > steps = m_asset->getSteps();
				if (!steps.empty())
				{
					for (uint32_t i = 0; i < steps.size() - 1; ++i)
					{
						if (steps[i] == selectedRow->getData< PostProcessStep >(L"STEP"))
						{
							Ref< PostProcessStep > r = steps[i];
							steps[i] = steps[i + 1];
							steps[i + 1] = r;
							break;
						}
					}
					m_asset->setSteps(steps);
				}
			}
			else
			{

			}
			updateViews();
		}
	}
	else if (command == L"World.PostProcess.AddTargetDefinition")
	{
		Ref< PostProcessDefine > definition = new PostProcessDefineTarget();
		RefArray< PostProcessDefine > definitions = m_asset->getDefinitions();
		definitions.push_back(definition);
		m_asset->setDefinitions(definitions);
		updateViews();
	}
	else if (command == L"World.PostProcess.AddTextureDefinition")
	{
		Ref< PostProcessDefine > definition = new PostProcessDefineTexture();
		RefArray< PostProcessDefine > definitions = m_asset->getDefinitions();
		definitions.push_back(definition);
		m_asset->setDefinitions(definitions);
		updateViews();
	}
	else if (command == L"World.PostProcess.RemoveDefinition")
	{
		ui::custom::GridRow* selectedRow = m_gridDefinitions->getSelectedRow();
		if (selectedRow)
		{
			RefArray< PostProcessDefine > definitions = m_asset->getDefinitions();
			definitions.remove(selectedRow->getData< PostProcessDefine >(L"DEFINITION"));
			m_asset->setDefinitions(definitions);
			updateViews();
		}
	}
	else
		return false;

	return true;
}

void PostProcessEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void PostProcessEditor::updateStepView(ui::custom::GridRow* parentStepRow, const RefArray< PostProcessStep >& steps, int32_t& inoutOrder)
{
	for (RefArray< PostProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
	{
		const IPostProcessStepFacade* stepFacade = m_postProcessStepFacades[&type_of(*i)];
		if (!stepFacade)
			continue;

		Ref< ui::custom::GridRow > stepRow = new ui::custom::GridRow();
		stepRow->add(new ui::custom::GridItem(toString(inoutOrder++)/*stepFacade->getImage(*i)*/));
		stepRow->add(new ui::custom::GridItem(stepFacade->getText(*i)));
		stepRow->setData(L"STEP", *i);

		if (parentStepRow)
			parentStepRow->addChild(stepRow);
		else
			m_gridSteps->addRow(stepRow);

		RefArray< PostProcessStep > children;
		if (stepFacade->getChildren(*i, children))
			updateStepView(stepRow, children, inoutOrder);
	}
}

void PostProcessEditor::updateViews()
{
	m_gridSteps->removeAllRows();
	m_gridDefinitions->removeAllRows();

	const RefArray< PostProcessStep >& steps = m_asset->getSteps();
	int32_t order = 0;
	updateStepView(0, steps, order);

	// Add implicit, read-only, definitions.
	const wchar_t* c_implicitDefinitions[] = { L"Output", L"InputColor", L"InputDepth", L"InputShadowMask" };
	for (uint32_t i = 0; i < sizeof_array(c_implicitDefinitions); ++i)
	{
		Ref< ui::custom::GridRow > definitionRow = new ui::custom::GridRow();
		definitionRow->add(new ui::custom::GridItem(L"(Implicit)"));
		definitionRow->add(new ui::custom::GridItem(c_implicitDefinitions[i]));
		definitionRow->setData(L"DEFINITION", 0);
		m_gridDefinitions->addRow(definitionRow);
	}

	// Add user definitions.
	const RefArray< PostProcessDefine >& definitions = m_asset->getDefinitions();
	for (RefArray< PostProcessDefine >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
	{
		const IPostProcessDefineFacade* defineFacade = m_postProcessDefineFacades[&type_of(*i)];
		if (!defineFacade)
			continue;

		Ref< ui::custom::GridRow > definitionRow = new ui::custom::GridRow();
		definitionRow->add(new ui::custom::GridItem(L"(User)"));
		definitionRow->add(new ui::custom::GridItem(defineFacade->getText(*i)));
		definitionRow->setData(L"DEFINITION", *i);
		m_gridDefinitions->addRow(definitionRow);
	}
}

void PostProcessEditor::eventStepToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void PostProcessEditor::eventGridStepSelect(ui::SelectionChangeEvent* event)
{
	m_gridDefinitions->deselectAll();

	ui::custom::GridRow* selectedRow = m_gridSteps->getSelectedRow();
	if (selectedRow)
		m_properties->set(selectedRow->getData< PostProcessStep >(L"STEP"));
	else
		m_properties->set(0);
}

void PostProcessEditor::eventDefinitionToolClick(ui::custom::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void PostProcessEditor::eventGridDefinitionSelect(ui::SelectionChangeEvent* event)
{
	m_gridSteps->deselectAll();

	ui::custom::GridRow* selectedRow = m_gridDefinitions->getSelectedRow();
	if (selectedRow)
		m_properties->set(selectedRow->getData< PostProcessDefine >(L"DEFINITION"));
	else
		m_properties->set(0);
}

void PostProcessEditor::eventStepPropertiesChange(ui::ContentChangeEvent* event)
{
	updateViews();
}

	}
}
