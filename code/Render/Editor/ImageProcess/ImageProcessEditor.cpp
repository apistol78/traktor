/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/String.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/Event.h"
#include "Ui/FloodLayout.h"
#include "Ui/TableLayout.h"
#include "Ui/Splitter.h"
#include "Ui/GridView/GridColumn.h"
#include "Ui/GridView/GridItem.h"
#include "Ui/GridView/GridRow.h"
#include "Ui/GridView/GridView.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Render/ImageProcess/ImageProcessDefineTarget.h"
#include "Render/ImageProcess/ImageProcessDefineTexture.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Render/ImageProcess/ImageProcessStepBlur.h"
#include "Render/ImageProcess/ImageProcessStepBokeh.h"
#include "Render/ImageProcess/ImageProcessStepChain.h"
#include "Render/ImageProcess/ImageProcessStepGodRay.h"
#include "Render/ImageProcess/ImageProcessStepGrain.h"
#include "Render/ImageProcess/ImageProcessStepLuminance.h"
#include "Render/ImageProcess/ImageProcessStepRepeat.h"
#include "Render/ImageProcess/ImageProcessStepSetTarget.h"
#include "Render/ImageProcess/ImageProcessStepSimple.h"
#include "Render/ImageProcess/ImageProcessStepSmProj.h"
#include "Render/ImageProcess/ImageProcessStepSsao.h"
#include "Render/ImageProcess/ImageProcessStepSwapTargets.h"
#include "Render/ImageProcess/ImageProcessStepTemporal.h"
#include "Render/Editor/ImageProcess/ImageProcessDefineTargetFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessDefineTextureFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessEditor.h"
#include "Render/Editor/ImageProcess/ImageProcessStepBlurFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessStepBokehFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessStepChainFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessStepGodRayFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessStepGrainFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessStepLuminanceFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessStepRepeatFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessStepSetTargetFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessStepSimpleFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessStepSmProjFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessStepSsaoFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessStepSwapTargetsFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessStepTemporalFacade.h"
#include "Render/Editor/ImageProcess/ImageProcessProperties.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageProcessEditor", ImageProcessEditor, editor::IObjectEditor)

ImageProcessEditor::ImageProcessEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool ImageProcessEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	if (!m_instance)
		return false;

	m_asset = dynamic_type_cast< ImageProcessSettings* >(object);
	if (!m_asset)
		return false;

	Ref< ui::Splitter > splitter = new ui::Splitter();
	splitter->create(parent, true, ui::dpi96(400), false);

	Ref< ui::Splitter > splitterView = new ui::Splitter();
	splitterView->create(splitter, false, 60, true);

	Ref< ui::Container > containerSteps = new ui::Container();
	containerSteps->create(splitterView, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolBarSteps = new ui::ToolBar();
	m_toolBarSteps->create(containerSteps);
	m_toolBarSteps->addItem(new ui::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_ADD_STEP"), 0, ui::Command(L"Render.ImageProcess.AddStep"), ui::ToolBarButton::BsText));
	m_toolBarSteps->addItem(new ui::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_REMOVE_STEP"), 0, ui::Command(L"Render.ImageProcess.RemoveStep"), ui::ToolBarButton::BsText));
	m_toolBarSteps->addItem(new ui::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_MOVE_UP"), 0, ui::Command(L"Render.ImageProcess.MoveUp"), ui::ToolBarButton::BsText));
	m_toolBarSteps->addItem(new ui::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_MOVE_DOWN"), 0, ui::Command(L"Render.ImageProcess.MoveDown"), ui::ToolBarButton::BsText));
	m_toolBarSteps->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ImageProcessEditor::eventStepToolClick);

	m_gridSteps = new ui::GridView();
	m_gridSteps->create(containerSteps, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader);
	m_gridSteps->addColumn(new ui::GridColumn(i18n::Text(L"POSTPROCESS_EDITOR_ORDER"), ui::dpi96(60)));
	m_gridSteps->addColumn(new ui::GridColumn(i18n::Text(L"POSTPROCESS_EDITOR_DESCRIPTION"), ui::dpi96(300)));
	m_gridSteps->addEventHandler< ui::SelectionChangeEvent >(this, &ImageProcessEditor::eventGridStepSelect);

	Ref< ui::Container > containerDefinitions = new ui::Container();
	containerDefinitions->create(splitterView, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolBarDefinitions = new ui::ToolBar();
	m_toolBarDefinitions->create(containerDefinitions);
	m_toolBarDefinitions->addItem(new ui::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_ADD_TARGET_DEFINITION"), 0, ui::Command(L"Render.ImageProcess.AddTargetDefinition"), ui::ToolBarButton::BsText));
	m_toolBarDefinitions->addItem(new ui::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_ADD_TEXTURE_DEFINITION"), 0, ui::Command(L"Render.ImageProcess.AddTextureDefinition"), ui::ToolBarButton::BsText));
	m_toolBarDefinitions->addItem(new ui::ToolBarButton(i18n::Text(L"POSTPROCESS_EDITOR_REMOVE_DEFINITION"), 0, ui::Command(L"Render.ImageProcess.RemoveDefinition"), ui::ToolBarButton::BsText));
	m_toolBarDefinitions->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ImageProcessEditor::eventDefinitionToolClick);

	m_gridDefinitions = new ui::GridView();
	m_gridDefinitions->create(containerDefinitions, ui::WsDoubleBuffer | ui::GridView::WsColumnHeader);
	m_gridDefinitions->addColumn(new ui::GridColumn(i18n::Text(L"POSTPROCESS_EDITOR_ID"), ui::dpi96(60)));
	m_gridDefinitions->addColumn(new ui::GridColumn(i18n::Text(L"POSTPROCESS_EDITOR_DESCRIPTION"), ui::dpi96(300)));
	m_gridDefinitions->addEventHandler< ui::SelectionChangeEvent >(this, &ImageProcessEditor::eventGridDefinitionSelect);

	Ref< ui::Container > container = new ui::Container();
	container->create(splitter, ui::WsNone, new ui::FloodLayout());

	m_properties = new ImageProcessProperties(m_editor);
	m_properties->create(container);
	m_properties->addEventHandler< ui::ContentChangeEvent >(this, &ImageProcessEditor::eventStepPropertiesChange);

	// Create define facades.
	m_imageProcessDefineFacades[&type_of < ImageProcessDefineTarget >()] = new ImageProcessDefineTargetFacade();
	m_imageProcessDefineFacades[&type_of < ImageProcessDefineTexture >()] = new ImageProcessDefineTextureFacade();

	// Create step facades.
	m_imageProcessStepFacades[&type_of< ImageProcessStepBlur >()] = new ImageProcessStepBlurFacade();
	m_imageProcessStepFacades[&type_of< ImageProcessStepBokeh >()] = new ImageProcessStepBokehFacade();
	m_imageProcessStepFacades[&type_of< ImageProcessStepChain >()] = new ImageProcessStepChainFacade();
	m_imageProcessStepFacades[&type_of< ImageProcessStepGodRay >()] = new ImageProcessStepGodRayFacade();
	m_imageProcessStepFacades[&type_of< ImageProcessStepGrain >()] = new ImageProcessStepGrainFacade();
	m_imageProcessStepFacades[&type_of< ImageProcessStepLuminance >()] = new ImageProcessStepLuminanceFacade();
	m_imageProcessStepFacades[&type_of< ImageProcessStepRepeat >()] = new ImageProcessStepRepeatFacade();
	m_imageProcessStepFacades[&type_of< ImageProcessStepSetTarget >()] = new ImageProcessStepSetTargetFacade();
	m_imageProcessStepFacades[&type_of< ImageProcessStepSimple >()] = new ImageProcessStepSimpleFacade();
	m_imageProcessStepFacades[&type_of< ImageProcessStepSmProj >()] = new ImageProcessStepSmProjFacade();
	m_imageProcessStepFacades[&type_of< ImageProcessStepSsao >()] = new ImageProcessStepSsaoFacade();
	m_imageProcessStepFacades[&type_of< ImageProcessStepSwapTargets >()] = new ImageProcessStepSwapTargetsFacade();
	m_imageProcessStepFacades[&type_of< ImageProcessStepTemporal >()] = new ImageProcessStepTemporalFacade();

	updateViews();
	return true;
}

void ImageProcessEditor::destroy()
{
}

void ImageProcessEditor::apply()
{
	m_instance->setObject(m_asset);
}

ui::Size ImageProcessEditor::getPreferredSize() const
{
	return ui::Size(ui::dpi96(900), ui::dpi96(600));
}

bool ImageProcessEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Render.ImageProcess.AddStep")
	{
		const TypeInfo* stepType = m_editor->browseType(makeTypeInfoSet< ImageProcessStep >());
		if (stepType)
		{
			Ref< ImageProcessStep > step = checked_type_cast< ImageProcessStep*, false >(stepType->createInstance());
			RefArray< ImageProcessStep > steps = m_asset->getSteps();
			steps.push_back(step);
			m_asset->setSteps(steps);
			updateViews();
		}
	}
	else if (command == L"Render.ImageProcess.RemoveStep")
	{
		ui::GridRow* selectedRow = m_gridSteps->getSelectedRow();
		if (selectedRow)
		{
			if (selectedRow->getParent() == 0)
			{
				RefArray< ImageProcessStep > steps = m_asset->getSteps();
				steps.remove(selectedRow->getData< ImageProcessStep >(L"STEP"));
				m_asset->setSteps(steps);
			}
			else
			{

			}
			updateViews();
		}
	}
	else if (command == L"Render.ImageProcess.MoveUp")
	{
		ui::GridRow* selectedRow = m_gridSteps->getSelectedRow();
		if (selectedRow)
		{
			if (selectedRow->getParent() == 0)
			{
				RefArray< ImageProcessStep > steps = m_asset->getSteps();
				for (uint32_t i = 1; i < steps.size(); ++i)
				{
					if (steps[i] == selectedRow->getData< ImageProcessStep >(L"STEP"))
					{
						Ref< ImageProcessStep > r = steps[i];
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
	else if (command == L"Render.ImageProcess.MoveDown")
	{
		ui::GridRow* selectedRow = m_gridSteps->getSelectedRow();
		if (selectedRow)
		{
			if (selectedRow->getParent() == 0)
			{
				RefArray< ImageProcessStep > steps = m_asset->getSteps();
				if (!steps.empty())
				{
					for (uint32_t i = 0; i < steps.size() - 1; ++i)
					{
						if (steps[i] == selectedRow->getData< ImageProcessStep >(L"STEP"))
						{
							Ref< ImageProcessStep > r = steps[i];
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
	else if (command == L"Render.ImageProcess.AddTargetDefinition")
	{
		Ref< ImageProcessDefine > definition = new ImageProcessDefineTarget();
		RefArray< ImageProcessDefine > definitions = m_asset->getDefinitions();
		definitions.push_back(definition);
		m_asset->setDefinitions(definitions);
		updateViews();
	}
	else if (command == L"Render.ImageProcess.AddTextureDefinition")
	{
		Ref< ImageProcessDefine > definition = new ImageProcessDefineTexture();
		RefArray< ImageProcessDefine > definitions = m_asset->getDefinitions();
		definitions.push_back(definition);
		m_asset->setDefinitions(definitions);
		updateViews();
	}
	else if (command == L"Render.ImageProcess.RemoveDefinition")
	{
		ui::GridRow* selectedRow = m_gridDefinitions->getSelectedRow();
		if (selectedRow)
		{
			RefArray< ImageProcessDefine > definitions = m_asset->getDefinitions();
			definitions.remove(selectedRow->getData< ImageProcessDefine >(L"DEFINITION"));
			m_asset->setDefinitions(definitions);
			updateViews();
		}
	}
	else
		return false;

	return true;
}

void ImageProcessEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

void ImageProcessEditor::updateStepView(ui::GridRow* parentStepRow, const RefArray< ImageProcessStep >& steps, int32_t& inoutOrder)
{
	for (RefArray< ImageProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
	{
		const IImageProcessStepFacade* stepFacade = m_imageProcessStepFacades[&type_of(*i)];
		if (!stepFacade)
			continue;

		Ref< ui::GridRow > stepRow = new ui::GridRow();
		stepRow->add(new ui::GridItem(toString(inoutOrder++)/*stepFacade->getImage(*i)*/));
		stepRow->add(new ui::GridItem(stepFacade->getText(m_editor, *i)));
		stepRow->setData(L"STEP", *i);

		if (parentStepRow)
			parentStepRow->addChild(stepRow);
		else
			m_gridSteps->addRow(stepRow);

		RefArray< ImageProcessStep > children;
		if (stepFacade->getChildren(*i, children))
			updateStepView(stepRow, children, inoutOrder);
	}
}

void ImageProcessEditor::updateViews()
{
	m_gridSteps->removeAllRows();
	m_gridDefinitions->removeAllRows();

	const RefArray< ImageProcessStep >& steps = m_asset->getSteps();
	int32_t order = 0;
	updateStepView(0, steps, order);

	// Add implicit, read-only, definitions.
	const wchar_t* c_implicitDefinitions[] = { L"Output", L"InputColor", L"InputDepth", L"InputNormal", L"InputVelocity", L"InputShadowMask" };
	for (uint32_t i = 0; i < sizeof_array(c_implicitDefinitions); ++i)
	{
		Ref< ui::GridRow > definitionRow = new ui::GridRow();
		definitionRow->add(new ui::GridItem(L"(Implicit)"));
		definitionRow->add(new ui::GridItem(c_implicitDefinitions[i]));
		definitionRow->setData(L"DEFINITION", 0);
		m_gridDefinitions->addRow(definitionRow);
	}

	// Add user definitions.
	const RefArray< ImageProcessDefine >& definitions = m_asset->getDefinitions();
	for (RefArray< ImageProcessDefine >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
	{
		const IImageProcessDefineFacade* defineFacade = m_imageProcessDefineFacades[&type_of(*i)];
		if (!defineFacade)
			continue;

		Ref< ui::GridRow > definitionRow = new ui::GridRow();
		definitionRow->add(new ui::GridItem(L"(User)"));
		definitionRow->add(new ui::GridItem(defineFacade->getText(m_editor, *i)));
		definitionRow->setData(L"DEFINITION", *i);
		m_gridDefinitions->addRow(definitionRow);
	}
}

void ImageProcessEditor::eventStepToolClick(ui::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void ImageProcessEditor::eventGridStepSelect(ui::SelectionChangeEvent* event)
{
	m_gridDefinitions->deselectAll();

	ui::GridRow* selectedRow = m_gridSteps->getSelectedRow();
	if (selectedRow)
		m_properties->set(selectedRow->getData< ImageProcessStep >(L"STEP"));
	else
		m_properties->set(0);
}

void ImageProcessEditor::eventDefinitionToolClick(ui::ToolBarButtonClickEvent* event)
{
	handleCommand(event->getCommand());
}

void ImageProcessEditor::eventGridDefinitionSelect(ui::SelectionChangeEvent* event)
{
	m_gridSteps->deselectAll();

	ui::GridRow* selectedRow = m_gridDefinitions->getSelectedRow();
	if (selectedRow)
		m_properties->set(selectedRow->getData< ImageProcessDefine >(L"DEFINITION"));
	else
		m_properties->set(0);
}

void ImageProcessEditor::eventStepPropertiesChange(ui::ContentChangeEvent* event)
{
	updateViews();
}

	}
}
