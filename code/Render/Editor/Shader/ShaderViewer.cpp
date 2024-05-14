/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/Editor/IProgramCompiler.h"
#include "Render/Editor/Shader/FragmentLinker.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"
#include "Render/Editor/Shader/ShaderModule.h"
#include "Render/Editor/Shader/ShaderViewer.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphCombinations.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphStatic.h"
#include "Render/Editor/Shader/Algorithms/ShaderGraphTechniques.h"
#include "Ui/Application.h"
#include "Ui/CheckBox.h"
#include "Ui/Clipboard.h"
#include "Ui/FileDialog.h"
#include "Ui/FloodLayout.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/TableLayout.h"
#include "Ui/DropDown.h"
#include "Ui/SyntaxRichEdit/SyntaxLanguageGlsl.h"
#include "Ui/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class FragmentReaderAdapter : public FragmentLinker::IFragmentReader
{
public:
	FragmentReaderAdapter(db::Database* db)
	:	m_db(db)
	{
	}

	virtual Ref< const ShaderGraph > read(const Guid& fragmentGuid) const
	{
		return m_db->getObjectReadOnly< ShaderGraph >(fragmentGuid);
	}

private:
	Ref< db::Database > m_db;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderViewer", ShaderViewer, ui::Container)

ShaderViewer::ShaderViewer(editor::IEditor* editor)
:	m_editor(editor)
{
}

void ShaderViewer::destroy()
{
	if (m_reflectJob)
	{
		m_reflectJob->wait();
		m_reflectJob = nullptr;
	}
	ui::Container::destroy();
}

bool ShaderViewer::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,*,*,100%", 0_ut, 0_ut)))
		return false;

	setText(i18n::Text(L"SHADERGRAPH_VIEWER"));

	Ref< ui::Container > containerDrops = new ui::Container();
	containerDrops->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", 4_ut, 4_ut));

	Ref< ui::Static > staticCompiler = new ui::Static();
	staticCompiler->create(containerDrops, i18n::Text(L"SHADERGRAPH_VIEWER_COMPILER"));

	m_dropCompiler = new ui::DropDown();
	m_dropCompiler->create(containerDrops);
	m_dropCompiler->addEventHandler< ui::SelectionChangeEvent >([&](ui::SelectionChangeEvent*) {
		if (!m_pendingShaderGraph)
			reflect(m_lastShaderGraph);
	});

	Ref< ui::Static > staticTechnique = new ui::Static();
	staticTechnique->create(containerDrops, i18n::Text(L"SHADERGRAPH_VIEWER_TECHNIQUE"));

	m_dropTechniques = new ui::DropDown();
	m_dropTechniques->create(containerDrops);
	m_dropTechniques->addEventHandler< ui::SelectionChangeEvent >([&](ui::SelectionChangeEvent*) {
		updateCombinations();
		updateShaders();
	});

	Ref< ui::Static > staticCombinations = new ui::Static();
	staticCombinations->create(containerDrops, i18n::Text(L"SHADERGRAPH_VIEWER_COMBINATIONS"));

	m_dropCombinations = new ui::DropDown();
	m_dropCombinations->create(containerDrops, ui::DropDown::WsMultiple);
	m_dropCombinations->addEventHandler< ui::SelectionChangeEvent >([&](ui::SelectionChangeEvent*) {
		updateShaders();
	});

	Ref< ui::Static > staticLanguages = new ui::Static();
	staticLanguages->create(containerDrops, i18n::Text(L"SHADERGRAPH_VIEWER_LANGUAGES"));

	m_dropLanguages = new ui::DropDown();
	m_dropLanguages->create(containerDrops);
	m_dropLanguages->add(L"");
	m_dropLanguages->add(L"GLSL");
	m_dropLanguages->add(L"HLSL");
	m_dropLanguages->add(L"MSL");
	m_dropLanguages->add(L"SPIRV");
	m_dropLanguages->addEventHandler< ui::SelectionChangeEvent >([&](ui::SelectionChangeEvent*) {
		if (!m_pendingShaderGraph)
			reflect(m_lastShaderGraph);
	});

	m_checkRelaxed = new ui::CheckBox();
	m_checkRelaxed->create(this, i18n::Text(L"SHADERGRAPH_VIEWER_RELAXED"), false);
	m_checkRelaxed->addEventHandler< ui::ButtonClickEvent >([&](ui::ButtonClickEvent*) {
		if (!m_pendingShaderGraph)
			reflect(m_lastShaderGraph);
	});

	Ref< ui::ToolBar > toolBar = new ui::ToolBar();
	toolBar->create(this);
	toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Save"));
	toolBar->addImage(new ui::StyleBitmap(L"Editor.ToolBar.Copy"));
	toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_SAVE"), 0, ui::Command(L"Shader.Editor.Preview.Save")));
	toolBar->addItem(new ui::ToolBarButton(i18n::Text(L"TOOLBAR_COPY"), 1, ui::Command(L"Shader.Editor.Preview.Copy")));
	toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &ShaderViewer::eventToolBarClick);

	const std::wstring programCompilerTypeName = m_editor->getSettings()->getProperty< std::wstring >(L"ShaderPipeline.ProgramCompiler");
	int32_t compilerIndex = 0;
	for (const auto programCompilerType : type_of< IProgramCompiler >().findAllOf(false))
	{
		if (std::wstring(programCompilerType->getName()) == L"traktor.render.ProgramCompilerVrfy")
			continue;

		Ref< IProgramCompiler > compiler = dynamic_type_cast< IProgramCompiler* >(programCompilerType->createInstance());
		if (!compiler)
			continue;

		if (programCompilerTypeName == programCompilerType->getName())
			compilerIndex = m_dropCompiler->add(compiler->getRendererSignature(), compiler);
		else
			m_dropCompiler->add(compiler->getRendererSignature(), compiler);
	}
	m_dropCompiler->select(compilerIndex);

	m_tab = new ui::Tab();
	m_tab->create(this, ui::Tab::WsBottom);

	Ref< ui::TabPage > tabPageVertex = new ui::TabPage();
	tabPageVertex->create(m_tab, i18n::Text(L"SHADERGRAPH_VIEWER_VERTEX"), new ui::FloodLayout());
	m_tab->addPage(tabPageVertex);

	Ref< ui::TabPage > tabPagePixel = new ui::TabPage();
	tabPagePixel->create(m_tab, i18n::Text(L"SHADERGRAPH_VIEWER_PIXEL"), new ui::FloodLayout());
	m_tab->addPage(tabPagePixel);

	Ref< ui::TabPage > tabPageCompute = new ui::TabPage();
	tabPageCompute->create(m_tab, i18n::Text(L"SHADERGRAPH_VIEWER_COMPUTE"), new ui::FloodLayout());
	m_tab->addPage(tabPageCompute);

	m_tab->setActivePage(tabPageVertex);

	// Create read-only syntax rich editors.
	m_shaderEditVertex = new ui::SyntaxRichEdit();
	m_shaderEditVertex->create(tabPageVertex, L"", ui::WsDoubleBuffer);
	m_shaderEditVertex->setLanguage(new ui::SyntaxLanguageGlsl());

	m_shaderEditPixel = new ui::SyntaxRichEdit();
	m_shaderEditPixel->create(tabPagePixel, L"", ui::WsDoubleBuffer);
	m_shaderEditPixel->setLanguage(new ui::SyntaxLanguageGlsl());

	m_shaderEditCompute = new ui::SyntaxRichEdit();
	m_shaderEditCompute->create(tabPageCompute, L"", ui::WsDoubleBuffer);
	m_shaderEditCompute->setLanguage(new ui::SyntaxLanguageGlsl());

	const std::wstring font = m_editor->getSettings()->getProperty< std::wstring >(L"Editor.Font", L"Consolas");
	const ui::Unit fontSize = ui::Unit(m_editor->getSettings()->getProperty< int32_t >(L"Editor.FontSize", 11));
	m_shaderEditVertex->setFont(ui::Font(font, fontSize));
	m_shaderEditPixel->setFont(ui::Font(font, fontSize));
	m_shaderEditCompute->setFont(ui::Font(font, fontSize));

	addEventHandler< ui::TimerEvent >(this, &ShaderViewer::eventTimer);
	startTimer(200);

	return true;
}

void ShaderViewer::reflect(const ShaderGraph* shaderGraph)
{
	m_pendingShaderGraph = DeepClone(shaderGraph).create< ShaderGraph >();
}

bool ShaderViewer::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.SettingsChanged")
	{
		const std::wstring font = m_editor->getSettings()->getProperty< std::wstring >(L"Editor.Font", L"Consolas");
		const ui::Unit fontSize = ui::Unit(m_editor->getSettings()->getProperty< int32_t >(L"Editor.FontSize", 11));
		m_shaderEditVertex->setFont(ui::Font(font, fontSize));
		m_shaderEditPixel->setFont(ui::Font(font, fontSize));
		m_shaderEditVertex->update();
		m_shaderEditPixel->update();
	}
	else
		return false;

	return true;
}

void ShaderViewer::updateTechniques()
{
	const std::wstring currentTechnique = m_dropTechniques->getSelectedItem();

	// Update techniques drop.
	m_dropTechniques->removeAll();
	for (auto it : m_techniques)
		m_dropTechniques->add(it.first);

	// Select previous technique. 
	if (m_dropTechniques->select(currentTechnique))
		return;

	// Select single technique.
	if (m_techniques.size() == 1)
		m_dropTechniques->select(0);

	// Last fallback to Default as it's the most common.
	m_dropTechniques->select(L"Default");
}

void ShaderViewer::updateCombinations()
{
	// Remove all previous checkboxes.
	m_dropCombinations->removeAll();

	// Create checkboxes for combination in selected technique.
	const std::wstring techniqueName = m_dropTechniques->getSelectedItem();
	const auto it = m_techniques.find(techniqueName);
	if (it != m_techniques.end())
	{
		for (const auto& parameter : it->second.parameters)
			m_dropCombinations->add(parameter);
	}

	update();
}

void ShaderViewer::updateShaders()
{
	uint32_t value = 0;

	// Calculate combination value.
	std::vector< int32_t > selectedCombinations;
	m_dropCombinations->getSelected(selectedCombinations);
	for (auto index : selectedCombinations)
		value |= 1 << index;

	const int32_t vertexOffset = m_shaderEditVertex->getScrollLine();
	const int32_t pixelOffset = m_shaderEditPixel->getScrollLine();
	const int32_t computeOffset = m_shaderEditCompute->getScrollLine();

	m_shaderEditVertex->setText(L"");
	m_shaderEditPixel->setText(L"");
	m_shaderEditCompute->setText(L"");

	// Find matching shader combination.
	std::wstring techniqueName = m_dropTechniques->getSelectedItem();
	std::map< std::wstring, TechniqueInfo >::const_iterator i = m_techniques.find(techniqueName);
	if (i != m_techniques.end())
	{
		for (std::vector< CombinationInfo >::const_iterator j = i->second.combinations.begin(); j != i->second.combinations.end(); ++j)
		{
			if ((j->mask & value) == j->value)
			{
				m_shaderEditVertex->setText(j->vertexShader);
				m_shaderEditPixel->setText(j->pixelShader);
				m_shaderEditCompute->setText(j->computeShader);
				break;
			}
		}
	}

	m_shaderEditVertex->scrollToLine(vertexOffset);
	m_shaderEditPixel->scrollToLine(pixelOffset);
	m_shaderEditCompute->scrollToLine(computeOffset);

	m_shaderEditVertex->update();
	m_shaderEditPixel->update();
	m_shaderEditCompute->update();
}

void ShaderViewer::eventToolBarClick(ui::ToolBarButtonClickEvent* event)
{
	const auto activeTabPage = m_tab->getActivePage();
	T_ASSERT(activeTabPage != nullptr);

	const auto edit = mandatory_non_null_type_cast< ui::SyntaxRichEdit* >(activeTabPage->getFirstChild());
	const std::wstring shader = edit->getText();

	if (event->getCommand() == L"Shader.Editor.Preview.Save")
	{
		Path filePath;

		ui::FileDialog fileDialog;
		fileDialog.create(this, type_name(this), L"Save shader as", L"Shader;*.*", L"", true);
		bool cancelled = !(fileDialog.showModal(filePath) == ui::DialogResult::Ok);
		fileDialog.destroy();

		if (!cancelled)
		{
			Ref< IStream > fs = FileSystem::getInstance().open(filePath, File::FmWrite);
			if (fs)
				FileOutputStream(fs, new Utf8Encoding()) << shader;
		}
	}
	else if (event->getCommand() == L"Shader.Editor.Preview.Copy")
		ui::Application::getInstance()->getClipboard()->setText(shader);
}

void ShaderViewer::eventTimer(ui::TimerEvent* event)
{
	if (m_reflectJob)
	{
		if (m_reflectJob->wait(0))
		{
			// Save reflected techniques into main thread copy.
			m_techniques = m_reflectedTechniques;
			m_reflectJob = nullptr;

			// Update user interface.
			updateTechniques();
			updateCombinations();
			updateShaders();
		}
	}
	else if (m_pendingShaderGraph)
	{
		if (!isVisible(true))
			return;

		Ref< const IProgramCompiler > compiler = m_dropCompiler->getSelectedData< IProgramCompiler >();
		if (!compiler)
			return;

		Ref< ShaderGraph > shaderGraph = m_pendingShaderGraph;
		m_reflectJob = JobManager::getInstance().add([=, this](){
			jobReflect(shaderGraph, compiler);
		});

		m_lastShaderGraph = m_pendingShaderGraph;
		m_pendingShaderGraph = nullptr;
	}
}

void ShaderViewer::jobReflect(Ref< ShaderGraph > shaderGraph, Ref< const IProgramCompiler > compiler)
{
	std::vector< Guid > textureIds;
	StringOutputStream ssv, ssp;

	m_reflectedTechniques.clear();

	// Create a copy of current settings and add our language of choice.
	Ref< PropertyGroup > settings = PropertyGroup::get(m_editor->getSettings());
	settings->setProperty< PropertyString >(L"Glsl.Vulkan.CrossDialect", m_dropLanguages->getSelectedItem());
	settings->setProperty< PropertyBoolean >(L"Glsl.Vulkan.ConvertRelaxedToHalf", m_checkRelaxed->isChecked());

	// Extract renderer permutation.
	const wchar_t* rendererSignature = compiler->getRendererSignature();
	T_ASSERT(rendererSignature);

	// Resolve all variables.
	shaderGraph = ShaderGraphStatic(shaderGraph, Guid()).getVariableResolved();
	if (!shaderGraph)
	{
		log::error << L"Shader viewer failed; unable to resolve variables." << Endl;
		return;
	}

	// Link shader fragments.
	FragmentReaderAdapter fragmentReader(m_editor->getSourceDatabase());
	if ((shaderGraph = FragmentLinker(fragmentReader).resolve(shaderGraph, true)) == 0)
		return;

	// Resolve all bundles.
	shaderGraph = render::ShaderGraphStatic(shaderGraph, Guid()).getBundleResolved();
	if (!shaderGraph)
	{
		log::error << L"Shader viewer failed; unable to resolve bundles." << Endl;
		return;
	}

	// Get connected permutation.
	shaderGraph = render::ShaderGraphStatic(shaderGraph, Guid()).getConnectedPermutation();
	if (!shaderGraph)
	{
		log::error << L"Shader viewer failed; unable to resolve connected permutation." << Endl;
		return;
	}

	// Get platform shader permutation.
	shaderGraph = ShaderGraphStatic(shaderGraph, Guid()).getPlatformPermutation(L"Other");
	if (!shaderGraph)
	{
		log::error << L"Shader viewer failed; unable to create platform permutation." << Endl;
		return;
	}

	// Get renderer shader permutation.
	shaderGraph = ShaderGraphStatic(shaderGraph, Guid()).getRendererPermutation(rendererSignature);
	if (!shaderGraph)
	{
		log::error << L"Shader viewer failed; unable to create renderer permutation." << Endl;
		return;
	}

	// Remove unused branches from shader graph.
	shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches(false);
	T_ASSERT(shaderGraph);

	// Get all techniques.
	ShaderGraphTechniques techniques(shaderGraph, Guid());
	for (auto techniqueName : techniques.getNames())
	{
		TechniqueInfo& ti = m_reflectedTechniques[techniqueName];

		Ref< const ShaderGraph > shaderGraphTechnique = techniques.generate(techniqueName);
		T_ASSERT(shaderGraphTechnique);

		ShaderGraphCombinations combinations(shaderGraphTechnique, Guid());
		ti.parameters = combinations.getParameterNames();

		const uint32_t combinationCount = combinations.getCombinationCount();
		ti.combinations.resize(combinationCount);

		for (uint32_t combination = 0; combination < combinationCount; ++combination)
		{
			CombinationInfo& ci = ti.combinations[combination];
			ci.mask = combinations.getCombinationMask(combination);
			ci.value = combinations.getCombinationValue(combination);

			Ref< const ShaderGraph > combinationGraph = combinations.getCombinationShaderGraph(combination);
			if (!combinationGraph)
				continue;

			Ref< ShaderGraph > programGraph = ShaderGraphStatic(combinationGraph, Guid()).getConnectedPermutation();

			if (programGraph)
				programGraph = ShaderGraphStatic(programGraph, Guid()).getTypePermutation();

			if (programGraph)
				programGraph = ShaderGraphStatic(programGraph, Guid()).getConstantFolded();

			if (programGraph)
				programGraph = ShaderGraphStatic(programGraph, Guid()).getStateResolved();

			if (programGraph)
				programGraph = ShaderGraphOptimizer(programGraph).mergeBranches();

			if (programGraph)
				programGraph = ShaderGraphOptimizer(programGraph).insertInterpolators();

			if (programGraph)
				programGraph = ShaderGraphStatic(programGraph, Guid()).getSwizzledPermutation();

			if (programGraph)
				programGraph = ShaderGraphStatic(programGraph, Guid()).cleanupRedundantSwizzles();

			if (programGraph)
			{
				// Replace texture nodes with uniforms.
				for (auto textureNode : programGraph->findNodesOf< Texture >())
				{
					const Guid& textureGuid = textureNode->getExternal();
					int32_t textureIndex;

					const auto it = std::find(textureIds.begin(), textureIds.end(), textureGuid);
					if (it != textureIds.end())
						textureIndex = (int32_t)std::distance(textureIds.begin(), it);
					else
					{
						textureIndex = (int32_t)textureIds.size();
						textureIds.push_back(textureGuid);
					}

					Ref< Uniform > textureUniform = new Uniform(
						getParameterNameFromTextureReferenceIndex(textureIndex),
						textureNode->getParameterType(),
						UpdateFrequency::Once
					);

					const OutputPin* textureUniformOutput = textureUniform->getOutputPin(0);
					T_ASSERT(textureUniformOutput);

					const OutputPin* textureNodeOutput = textureNode->getOutputPin(0);
					T_ASSERT(textureNodeOutput);

					programGraph->addNode(textureUniform);
					programGraph->rewire(textureNodeOutput, textureUniformOutput);
				}

				auto reolveModule = [&](const Guid& moduleId) -> std::wstring {
					Ref< const ShaderModule > shaderModule = m_editor->getSourceDatabase()->getObjectReadOnly< ShaderModule >(moduleId);
					if (shaderModule)
						return shaderModule->escape([](const Guid& id) -> std::wstring {
							return id.format();
						});
					else
						return L"";
				};

				// Finally ready to compile program graph.
				std::wstring vertexShader, pixelShader, computeShader;
				if (compiler->generate(programGraph, settings, techniqueName, reolveModule, vertexShader, pixelShader, computeShader))
				{
					ci.vertexShader = vertexShader;
					ci.pixelShader = pixelShader;
					ci.computeShader = computeShader;
				}
				else
				{
					ci.vertexShader = L"Failed to generate vertex shader!";
					ci.pixelShader = L"Failed to generate pixel shader!";;
					ci.computeShader = L"Failed to generate compute shader!";
				}
			}
		}
	}
}

	}
}
