#include "Core/Functor/Functor.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Database/Database.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Render/IProgramCompiler.h"
#include "Render/Editor/Shader/ShaderGraphCombinations.h"
#include "Render/Editor/Shader/ShaderGraphOptimizer.h"
#include "Render/Editor/Shader/ShaderGraphStatic.h"
#include "Render/Editor/Shader/ShaderGraphTechniques.h"
#include "Render/Editor/Shader/ShaderViewer.h"
#include "Render/Resource/FragmentLinker.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader/ShaderGraph.h"
#include "Ui/Application.h"
#include "Ui/CheckBox.h"
#include "Ui/DropDown.h"
#include "Ui/FloodLayout.h"
#include "Ui/Static.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageHlsl.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxRichEdit.h"

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
		m_reflectJob = 0;
	}
	ui::Container::destroy();
}

bool ShaderViewer::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,*,100%", 0, 0)))
		return false;

	setText(i18n::Text(L"SHADERGRAPH_VIEWER"));

	Ref< ui::Container > containerDrops = new ui::Container();
	containerDrops->create(this, ui::WsNone, new ui::TableLayout(L"*,100%", L"*", ui::scaleBySystemDPI(4), ui::scaleBySystemDPI(4)));

	Ref< ui::Static > staticCompiler = new ui::Static();
	staticCompiler->create(containerDrops, i18n::Text(L"SHADERGRAPH_VIEWER_COMPILER"));

	m_dropCompiler = new ui::DropDown();
	m_dropCompiler->create(containerDrops);
	m_dropCompiler->addEventHandler< ui::SelectionChangeEvent >(this, &ShaderViewer::eventCompilerChange);

	Ref< ui::Static > staticTechnique = new ui::Static();
	staticTechnique->create(containerDrops, i18n::Text(L"SHADERGRAPH_VIEWER_TECHNIQUE"));

	m_dropTechniques = new ui::DropDown();
	m_dropTechniques->create(containerDrops);
	m_dropTechniques->addEventHandler< ui::SelectionChangeEvent >(this, &ShaderViewer::eventTechniqueChange);

	m_containerCombinations = new ui::Container();
	m_containerCombinations->create(this, ui::WsNone, new ui::TableLayout(L"*", L"*", ui::scaleBySystemDPI(4), ui::scaleBySystemDPI(4)));

	std::set< const TypeInfo* > programCompilerTypes;
	type_of< IProgramCompiler >().findAllOf(programCompilerTypes, false);

	std::wstring programCompilerTypeName = m_editor->getSettings()->getProperty< PropertyString >(L"ShaderPipeline.ProgramCompiler");
	int32_t compilerIndex = 0;
	for (std::set< const TypeInfo* >::const_iterator i = programCompilerTypes.begin(); i != programCompilerTypes.end(); ++i)
	{
		Ref< IProgramCompiler > compiler = mandatory_non_null_type_cast< IProgramCompiler* >((*i)->createInstance());
		if (programCompilerTypeName == (*i)->getName())
			compilerIndex = m_dropCompiler->add(compiler->getPlatformSignature(), compiler);
		else
			m_dropCompiler->add(compiler->getPlatformSignature(), compiler);
	}
	m_dropCompiler->select(compilerIndex);

	Ref< ui::Tab > tab = new ui::Tab();
	tab->create(this, ui::Tab::WsBottom);

	Ref< ui::TabPage > tabPageVertex = new ui::TabPage();
	tabPageVertex->create(tab, i18n::Text(L"SHADERGRAPH_VIEWER_VERTEX"), new ui::FloodLayout());
	tab->addPage(tabPageVertex);

	Ref< ui::TabPage > tabPagePixel = new ui::TabPage();
	tabPagePixel->create(tab, i18n::Text(L"SHADERGRAPH_VIEWER_PIXEL"), new ui::FloodLayout());
	tab->addPage(tabPagePixel);

	tab->setActivePage(tabPageVertex);

	// Create read-only syntax rich editors.
	m_shaderEditVertex = new ui::custom::SyntaxRichEdit();
	m_shaderEditVertex->create(tabPageVertex, L"", ui::WsDoubleBuffer);
	m_shaderEditVertex->setLanguage(new ui::custom::SyntaxLanguageHlsl());

	m_shaderEditPixel = new ui::custom::SyntaxRichEdit();
	m_shaderEditPixel->create(tabPagePixel, L"", ui::WsDoubleBuffer);
	m_shaderEditPixel->setLanguage(new ui::custom::SyntaxLanguageHlsl());

	std::wstring font = m_editor->getSettings()->getProperty< PropertyString >(L"Editor.Font", L"Consolas");
	int32_t fontSize = m_editor->getSettings()->getProperty< PropertyInteger >(L"Editor.FontSize", 14);
	m_shaderEditVertex->setFont(ui::Font(font, fontSize));
	m_shaderEditPixel->setFont(ui::Font(font, fontSize));

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
		std::wstring font = m_editor->getSettings()->getProperty< PropertyString >(L"Editor.Font", L"Consolas");
		int32_t fontSize = m_editor->getSettings()->getProperty< PropertyInteger >(L"Editor.FontSize", 14);
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
	// Update techniques drop, keep current selected technique if still exist.
	std::wstring currentTechnique = m_dropTechniques->getSelectedItem();
	m_dropTechniques->removeAll();
	for (std::map< std::wstring, TechniqueInfo >::const_iterator i = m_techniques.begin(); i != m_techniques.end(); ++i)
		m_dropTechniques->add(i->first);
	m_dropTechniques->select(currentTechnique);
}

void ShaderViewer::updateCombinations()
{
	// Remove all previous checkboxes.
	for (RefArray< ui::CheckBox >::iterator i = m_checkCombinations.begin(); i != m_checkCombinations.end(); ++i)
		(*i)->destroy();
	m_checkCombinations.clear();

	// Create checkboxes for combination in selected technique.
	std::wstring techniqueName = m_dropTechniques->getSelectedItem();
	std::map< std::wstring, TechniqueInfo >::const_iterator i = m_techniques.find(techniqueName);
	if (i != m_techniques.end())
	{
		for (std::vector< std::wstring >::const_iterator j = i->second.parameters.begin(); j != i->second.parameters.end(); ++j)
		{
			Ref< ui::CheckBox > checkCombination = new ui::CheckBox();
			checkCombination->create(m_containerCombinations, *j);
			checkCombination->addEventHandler< ui::ButtonClickEvent >(this, &ShaderViewer::eventCombinationClick);
			m_checkCombinations.push_back(checkCombination);
		}
	}

	update();
}

void ShaderViewer::updateShaders()
{
	uint32_t value = 0;

	// Calculate combination value.
	for (uint32_t i = 0; i < m_checkCombinations.size(); ++i)
	{
		if (m_checkCombinations[i]->isChecked())
			value |= 1 << i;
	}

	int32_t vertexOffset = m_shaderEditVertex->getScrollLine();
	int32_t pixelOffset = m_shaderEditPixel->getScrollLine();

	m_shaderEditVertex->setText(L"");
	m_shaderEditPixel->setText(L"");

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
				break;
			}
		}
	}

	m_shaderEditVertex->scrollToLine(vertexOffset);
	m_shaderEditPixel->scrollToLine(pixelOffset);

	m_shaderEditVertex->update();
	m_shaderEditPixel->update();
}

void ShaderViewer::eventCompilerChange(ui::SelectionChangeEvent* event)
{
	if (!m_pendingShaderGraph)
		reflect(m_lastShaderGraph);
}

void ShaderViewer::eventTechniqueChange(ui::SelectionChangeEvent* event)
{
	updateCombinations();
	updateShaders();
}

void ShaderViewer::eventCombinationClick(ui::ButtonClickEvent* event)
{
	updateShaders();
}

void ShaderViewer::eventTimer(ui::TimerEvent* event)
{
	if (m_reflectJob)
	{
		if (m_reflectJob->wait(0))
		{
			// Save reflected techniques into main thread copy.
			m_techniques = m_reflectedTechniques;
			m_reflectJob = 0;

			// Update user interface.
			updateTechniques();
			updateCombinations();
			updateShaders();
		}
	}
	else if (m_pendingShaderGraph)
	{
		Ref< const IProgramCompiler > compiler = m_dropCompiler->getSelectedData< IProgramCompiler >();
		T_ASSERT (compiler);

		m_reflectJob = JobManager::getInstance().add(makeFunctor(this, &ShaderViewer::jobReflect, m_pendingShaderGraph, compiler));

		m_lastShaderGraph = m_pendingShaderGraph;
		m_pendingShaderGraph = 0;
	}
}

void ShaderViewer::jobReflect(Ref< ShaderGraph > shaderGraph, Ref< const IProgramCompiler > compiler)
{
	std::vector< Guid > textureIds;
	StringOutputStream ssv, ssp;

	m_reflectedTechniques.clear();

	// Extract platform permutation.
	const wchar_t* platformSignature = compiler->getPlatformSignature();
	T_ASSERT (platformSignature);

	// Link shader fragments.
	FragmentReaderAdapter fragmentReader(m_editor->getSourceDatabase());
	if ((shaderGraph = FragmentLinker(fragmentReader).resolve(shaderGraph, true)) == 0)
		return;

	// Get platform shader permutation.
	shaderGraph = ShaderGraphStatic(shaderGraph).getPlatformPermutation(platformSignature);
	T_ASSERT (shaderGraph);

	// Remove unused branches from shader graph.
	shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches();
	T_ASSERT (shaderGraph);

	// Get all techniques.
	ShaderGraphTechniques techniques(shaderGraph);
	std::set< std::wstring > techniqueNames = techniques.getNames();
	for (std::set< std::wstring >::iterator i = techniqueNames.begin(); i != techniqueNames.end(); ++i)
	{
		TechniqueInfo& ti = m_reflectedTechniques[*i];

		Ref< const ShaderGraph > shaderGraphTechnique = techniques.generate(*i);
		T_ASSERT (shaderGraphTechnique);

		ShaderGraphCombinations combinations(shaderGraphTechnique);
		ti.parameters = combinations.getParameterNames();

		uint32_t combinationCount = combinations.getCombinationCount();
		ti.combinations.resize(combinationCount);

		for (uint32_t combination = 0; combination < combinationCount; ++combination)
		{
			CombinationInfo& ci = ti.combinations[combination];
			ci.mask = combinations.getCombinationMask(combination);
			ci.value = combinations.getCombinationValue(combination);

			Ref< const ShaderGraph > combinationGraph = combinations.getCombinationShaderGraph(combination);
			if (!combinationGraph)
				continue;

			Ref< ShaderGraph > programGraph = ShaderGraphStatic(combinationGraph).getVariableResolved();

			if (programGraph)
				programGraph = ShaderGraphStatic(programGraph).getConnectedPermutation();

			if (programGraph)
				programGraph = ShaderGraphStatic(programGraph).getTypePermutation();
			
			if (programGraph)
				programGraph = ShaderGraphStatic(programGraph).getConstantFolded();

			if (programGraph)
				programGraph = ShaderGraphStatic(programGraph).getStateResolved();

			if (programGraph)
				programGraph = ShaderGraphOptimizer(programGraph).mergeBranches();
			
			if (programGraph)
				programGraph = ShaderGraphOptimizer(programGraph).insertInterpolators(false);

			if (programGraph)
				programGraph = ShaderGraphStatic(programGraph).getSwizzledPermutation();

			if (programGraph)
				programGraph = ShaderGraphStatic(programGraph).cleanupRedundantSwizzles();

			if (programGraph)
			{
				// Replace texture nodes with uniforms.
				RefArray< Texture > textureNodes;
				programGraph->findNodesOf< Texture >(textureNodes);

				for (RefArray< Texture >::iterator j = textureNodes.begin(); j != textureNodes.end(); ++j)
				{
					const Guid& textureGuid = (*j)->getExternal();
					int32_t textureIndex;

					std::vector< Guid >::iterator it = std::find(textureIds.begin(), textureIds.end(), textureGuid);
					if (it != textureIds.end())
						textureIndex = std::distance(textureIds.begin(), it);
					else
					{
						textureIndex = int32_t(textureIds.size());
						textureIds.push_back(textureGuid);
					}

					Ref< Uniform > textureUniform = new Uniform(
						getParameterNameFromTextureReferenceIndex(textureIndex),
						(*j)->getParameterType(),
						UfOnce
					);

					const OutputPin* textureUniformOutput = textureUniform->getOutputPin(0);
					T_ASSERT (textureUniformOutput);

					const OutputPin* textureNodeOutput = (*j)->getOutputPin(0);
					T_ASSERT (textureNodeOutput);

					programGraph->rewire(textureNodeOutput, textureUniformOutput);
					programGraph->addNode(textureUniform);
				}

				// Finally ready to compile program graph.
				std::wstring vertexShader, pixelShader;
				if (compiler->generate(programGraph, m_editor->getSettings(), 0, vertexShader, pixelShader))
				{
					ci.vertexShader = vertexShader;
					ci.pixelShader = pixelShader;
				}
			}
		}
	}
}

	}
}
