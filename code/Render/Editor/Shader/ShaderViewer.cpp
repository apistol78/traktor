#include "Core/Io/StringOutputStream.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
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
#include "Ui/TableLayout.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageHlsl.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"

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
		Ref< const ShaderGraph > shaderGraph = m_db->getObjectReadOnly< ShaderGraph >(fragmentGuid);
		if (!shaderGraph)
			return 0;

		shaderGraph = ShaderGraphStatic(shaderGraph).getVariableResolved();
		if (!shaderGraph)
			return 0;

		return shaderGraph;
	}

private:
	Ref< db::Database > m_db;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderViewer", ShaderViewer, ui::Container)

ShaderViewer::ShaderViewer(editor::IEditor* editor)
:	m_editor(editor)
,	m_reflectThread(0)
{
}

void ShaderViewer::destroy()
{
	if (m_reflectThread)
	{
		m_reflectThread->stop();
		ThreadManager::getInstance().destroy(m_reflectThread);
		m_reflectThread = 0;
	}
	ui::Container::destroy();
}

bool ShaderViewer::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0)))
		return false;

	setText(i18n::Text(L"SHADERGRAPH_VIEWER"));

	m_shaderTools = new ui::custom::ToolBar();
	m_shaderTools->create(this);

	// Create drop down of all loaded program compilers.
	m_compilerTool = new ui::custom::ToolBarDropDown(ui::Command(L"ShaderGraph.Editor.ProgramCompiler"), 300, L"");

	std::set< const TypeInfo* > programCompilerTypes;
	type_of< IProgramCompiler >().findAllOf(programCompilerTypes, false);

	std::wstring programCompilerTypeName = m_editor->getSettings()->getProperty< PropertyString >(L"ShaderPipeline.ProgramCompiler");
	int compilerIndex = 0;
	for (std::set< const TypeInfo* >::const_iterator i = programCompilerTypes.begin(); i != programCompilerTypes.end(); ++i)
	{
		if (programCompilerTypeName == (*i)->getName())
			compilerIndex = m_compilerTool->add((*i)->getName());
		else
			m_compilerTool->add((*i)->getName());
	}
	m_compilerTool->select(compilerIndex);

	m_shaderTools->addItem(m_compilerTool);
	m_shaderTools->addEventHandler< ui::custom::ToolBarButtonClickEvent >(this, &ShaderViewer::eventShaderToolsClick);

	// Create read-only syntax rich editor.
	m_shaderEdit = new ui::custom::SyntaxRichEdit();
	m_shaderEdit->create(this, L"", ui::WsDoubleBuffer);
	m_shaderEdit->setLanguage(new ui::custom::SyntaxLanguageHlsl());
#if defined(__APPLE__)
	m_shaderEdit->setFont(ui::Font(L"Courier New", 14));
#elif defined(__LINUX__)
	m_shaderEdit->setFont(ui::Font(L"Courier New", 14));
#else
	m_shaderEdit->setFont(ui::Font(L"Consolas", 14));
#endif

	// Create reflector thread.
	m_reflectThread = ThreadManager::getInstance().create(makeFunctor(this, &ShaderViewer::threadReflect), L"Shader reflector");
	T_ASSERT (m_reflectThread);
	m_reflectThread->start();

	return true;
}

void ShaderViewer::reflect(const ShaderGraph* shaderGraph)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_reflectLock);
	m_reflectShaderGraph = DeepClone(shaderGraph).create< ShaderGraph >();
	T_ASSERT (m_reflectShaderGraph);
}

void ShaderViewer::threadReflect()
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (!currentThread->stopped())
	{
		// Don't do anything while the viewer is hidden.
		if (!isVisible(true))
		{
			currentThread->sleep(250);
			continue;
		}

		// Grab enqueued shader graph for reflection.
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_reflectLock);
			
			if (!m_reflectShaderGraph)
				continue;

			m_shaderGraph = m_reflectShaderGraph;
			m_reflectShaderGraph = 0;
		}

		// In case no new graph for reflection.
		if (!m_shaderGraph)
		{
			currentThread->sleep(250);
			continue;
		}

		// Link shader fragments.
		FragmentReaderAdapter fragmentReader(m_editor->getSourceDatabase());
		m_shaderGraph = FragmentLinker(fragmentReader).resolve(m_shaderGraph, true);

		threadUpdateViews();
	}
}

void ShaderViewer::threadUpdateViews()
{
	std::wstring programCompilerTypeName = m_compilerTool->getSelectedItem();
	std::vector< Guid > textureIds;
	StringOutputStream ss;

	if (!m_shaderGraph)
		return;

	// Create program compiler implementation.
	const TypeInfo* programCompilerType = TypeInfo::find(programCompilerTypeName);
	if (!programCompilerType)
		return;

	Ref< IProgramCompiler > programCompiler = dynamic_type_cast< IProgramCompiler* >(programCompilerType->createInstance());
	if (!programCompiler)
		return;

	// Extract platform permutation.
	const wchar_t* platformSignature = programCompiler->getPlatformSignature();
	T_ASSERT (platformSignature);

	Ref< ShaderGraph > shaderGraph = ShaderGraphStatic(m_shaderGraph).getPlatformPermutation(platformSignature);
	T_ASSERT (shaderGraph);

	// Remove unused branches from shader graph.
	shaderGraph = ShaderGraphOptimizer(shaderGraph).removeUnusedBranches();
	T_ASSERT (shaderGraph);

	// Generate shader graphs from techniques and combinations.
	ShaderGraphTechniques techniques(shaderGraph);
	std::set< std::wstring > techniqueNames = techniques.getNames();
	for (std::set< std::wstring >::iterator i = techniqueNames.begin(); i != techniqueNames.end(); ++i)
	{
		Ref< const ShaderGraph > shaderGraphTechnique = techniques.generate(*i);
		T_ASSERT (shaderGraphTechnique);

		ShaderGraphCombinations combinations(shaderGraphTechnique);
		uint32_t combinationCount = combinations.getCombinationCount();
		for (uint32_t combination = 0; combination < combinationCount; ++combination)
		{
			ss << L"// Technique \"" << *i << L"\"";

			// Get combination parameter names.
			uint32_t mask = combinations.getCombinationMask(combination);
			uint32_t value = combinations.getCombinationValue(combination);
			std::vector< std::wstring > names = combinations.getParameterNames(value & mask);
			for (std::vector< std::wstring >::const_iterator i = names.begin(); i != names.end(); ++i)
				ss << L", " << *i;

			ss << Endl;

			Ref< const ShaderGraph > combinationGraph = combinations.getCombinationShaderGraph(combination);
			if (!combinationGraph)
			{
				ss << L"N/A" << Endl;
				continue;
			}

			Ref< ShaderGraph > programGraph = ShaderGraphStatic(combinationGraph).getVariableResolved();

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
				std::wstring programShader;
				if (programCompiler->generate(programGraph, m_editor->getSettings(), 0, programShader))
					ss << programShader;
				else
					ss << L"N/A" << Endl;
			}
			else
				ss << L"N/A" << Endl;
		}
	}

	m_shaderEdit->setText(ss.str());
	m_shaderEdit->update();
}

void ShaderViewer::eventShaderToolsClick(ui::custom::ToolBarButtonClickEvent* event)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_reflectLock);
	m_reflectShaderGraph = DeepClone(m_shaderGraph).create< ShaderGraph >();
	T_ASSERT (m_reflectShaderGraph);
}

	}
}
