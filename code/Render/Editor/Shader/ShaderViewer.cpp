#include "Core/Io/StringOutputStream.h"
#include "Core/Serialization/DeepClone.h"
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
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguageHlsl.h"
#include "Ui/Custom/SyntaxRichEdit/SyntaxRichEdit.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderViewer", ShaderViewer, ui::Container)

ShaderViewer::ShaderViewer(editor::IEditor* editor)
:	m_editor(editor)
{
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
	for (std::set< const TypeInfo* >::const_iterator i = programCompilerTypes.begin(); i != programCompilerTypes.end(); ++i)
		m_compilerTool->add((*i)->getName());

	m_compilerTool->select(0);
	m_shaderTools->addItem(m_compilerTool);
	m_shaderTools->addClickEventHandler(ui::createMethodHandler(this, &ShaderViewer::eventShaderToolsClick));

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

	return true;
}

void ShaderViewer::reflect(const ShaderGraph* shaderGraph)
{
	m_shaderGraph = DeepClone(shaderGraph).create< ShaderGraph >();
	T_ASSERT (m_shaderGraph);

	class FragmentReaderAdapter : public FragmentLinker::FragmentReader
	{
	public:
		FragmentReaderAdapter(db::Database* db)
		:	m_db(db)
		{
		}

		virtual Ref< const ShaderGraph > read(const Guid& fragmentGuid)
		{
			return m_db->getObjectReadOnly< ShaderGraph >(fragmentGuid);
		}

	private:
		Ref< db::Database > m_db;
	};

	// Link shader fragments.
	FragmentReaderAdapter fragmentReader(m_editor->getSourceDatabase());
	m_shaderGraph = FragmentLinker(fragmentReader).resolve(m_shaderGraph, true);

	updateViews();
}

void ShaderViewer::updateViews()
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
			std::vector< std::wstring > names = combinations.getParameterNames(combination);
			for (std::vector< std::wstring >::const_iterator i = names.begin(); i != names.end(); ++i)
				ss << L", " << *i;

			ss << Endl;

			Ref< const ShaderGraph > combinationGraph = combinations.getCombinationShaderGraph(combination);
			if (!combinationGraph)
			{
				ss << L"N/A" << Endl;
				continue;
			}

			Ref< ShaderGraph > programGraph = ShaderGraphStatic(combinationGraph).getTypePermutation();
			
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
				if (programCompiler->generate(programGraph, 0, programShader))
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

void ShaderViewer::eventShaderToolsClick(ui::Event* event)
{
	updateViews();
}

	}
}
