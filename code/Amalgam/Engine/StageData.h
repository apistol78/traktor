#ifndef traktor_amalgam_StageData_H
#define traktor_amalgam_StageData_H

#include <map>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;

	}

	namespace render
	{

class Shader;

	}

	namespace script
	{

class IScriptContext;

	}

	namespace amalgam
	{

class LayerData;
class Stage;

class T_DLLCLASS StageData : public ISerializable
{
	T_RTTI_CLASS;

public:
	StageData();

	virtual Ref< Stage > createInstance(amalgam::IEnvironment* environment, const Object* params) const;

	virtual void serialize(ISerializer& s);

	const Guid& getInherit() const { return m_inherit; }

	const RefArray< LayerData >& getLayers() const { return m_layers; }

	void setScript(const resource::Id< script::IScriptContext >& script) { m_script = script; }

	const resource::Id< script::IScriptContext >& getScript() const { return m_script; }

	void setShaderFade(const resource::Id< render::Shader >& shaderFade) { m_shaderFade = shaderFade; }

	const resource::Id< render::Shader >& getShaderFade() const { return m_shaderFade; }

	void setFadeRate(float fadeRate) { m_fadeRate = fadeRate; }

	float getFadeRate() const { return m_fadeRate; }

	void setTransitions(const std::map< std::wstring, Guid >& transitions) { m_transitions = transitions; }

	const std::map< std::wstring, Guid >& getTransitions() const { return m_transitions; }

private:
	friend class StagePipeline;

	Guid m_inherit;
	RefArray< LayerData > m_layers;
	resource::Id< script::IScriptContext > m_script;
	resource::Id< render::Shader > m_shaderFade;
	float m_fadeRate;
	std::map< std::wstring, Guid > m_transitions;
	Guid m_resourceBundle;
};

	}
}

#endif	// traktor_amalgam_StageData_H
