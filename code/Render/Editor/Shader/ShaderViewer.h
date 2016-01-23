#ifndef traktor_render_ShaderViewer_H
#define traktor_render_ShaderViewer_H

#include "Ui/Container.h"

namespace traktor
{

class Job;

	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class CheckBox;
class DropDown;

		namespace custom
		{

class SyntaxRichEdit;

		}
	}

	namespace render
	{

class IProgramCompiler;
class ShaderGraph;

class ShaderViewer : public ui::Container
{
	T_RTTI_CLASS;

public:
	ShaderViewer(editor::IEditor* editor);

	virtual void destroy();

	bool create(ui::Widget* parent);

	void reflect(const ShaderGraph* shaderGraph);

	bool handleCommand(const ui::Command& command);

private:
	struct CombinationInfo
	{
		uint32_t mask;
		uint32_t value;
		std::wstring vertexShader;
		std::wstring pixelShader;
	};

	struct TechniqueInfo
	{
		std::vector< std::wstring > parameters;
		std::vector< CombinationInfo > combinations;
	};

	editor::IEditor* m_editor;
	Ref< ui::DropDown > m_dropCompiler;
	Ref< ui::DropDown > m_dropTechniques;
	Ref< ui::Container > m_containerCombinations;
	RefArray< ui::CheckBox > m_checkCombinations;
	Ref< ui::custom::SyntaxRichEdit > m_shaderEditVertex;
	Ref< ui::custom::SyntaxRichEdit > m_shaderEditPixel;
	Ref< ShaderGraph > m_pendingShaderGraph;
	Ref< ShaderGraph > m_lastShaderGraph;
	Ref< Job > m_reflectJob;
	std::map< std::wstring, TechniqueInfo > m_techniques;

	// These members are updated by reflection job thus
	// do not access while job is pending.
	std::map< std::wstring, TechniqueInfo > m_reflectedTechniques;

	void updateTechniques();

	void updateCombinations();

	void updateShaders();

	void eventCompilerChange(ui::SelectionChangeEvent* event);

	void eventTechniqueChange(ui::SelectionChangeEvent* event);

	void eventCombinationClick(ui::ButtonClickEvent* event);

	void eventTimer(ui::TimerEvent* event);

	void jobReflect(Ref< ShaderGraph > shaderGraph, Ref< const IProgramCompiler > compiler);
};

	}
}

#endif	// traktor_render_ShaderViewer_H
