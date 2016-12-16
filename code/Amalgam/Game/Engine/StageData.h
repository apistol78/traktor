#ifndef traktor_amalgam_StageData_H
#define traktor_amalgam_StageData_H

#include <map>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;
class PropertyGroup;

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

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS StageData : public ISerializable
{
	T_RTTI_CLASS;

public:
	StageData();

	Ref< Stage > createInstance(IEnvironment* environment, const Object* params) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const std::wstring& getName() const { return m_name; }

	const Guid& getInherit() const { return m_inherit; }

	void setLayers(const RefArray< LayerData >& layers) { m_layers = layers; }

	const RefArray< LayerData >& getLayers() const { return m_layers; }

	void setClass(const resource::Id< IRuntimeClass >& clazz) { m_class = clazz; }

	const resource::Id< IRuntimeClass >& getClass() const { return m_class; }

	void setScript(const resource::Id< script::IScriptContext >& script) { m_script = script; }

	const resource::Id< script::IScriptContext >& getScript() const { return m_script; }

	void setShaderFade(const resource::Id< render::Shader >& shaderFade) { m_shaderFade = shaderFade; }

	const resource::Id< render::Shader >& getShaderFade() const { return m_shaderFade; }

	void setFadeRate(float fadeRate) { m_fadeRate = fadeRate; }

	float getFadeRate() const { return m_fadeRate; }

	void setTransitions(const std::map< std::wstring, Guid >& transitions) { m_transitions = transitions; }

	const std::map< std::wstring, Guid >& getTransitions() const { return m_transitions; }

	void setResourceBundle(const Guid& resourceBundle) { m_resourceBundle = resourceBundle; }

	const Guid& getResourceBundle() const { return m_resourceBundle; }

	void setProperties(const PropertyGroup* properties) { m_properties = properties; }

	const PropertyGroup* getProperties() const { return m_properties; }

private:
	friend class StagePipeline;

	std::wstring m_name;
	Guid m_inherit;
	RefArray< LayerData > m_layers;
	resource::Id< IRuntimeClass > m_class;
	resource::Id< script::IScriptContext > m_script;
	resource::Id< render::Shader > m_shaderFade;
	float m_fadeRate;
	std::map< std::wstring, Guid > m_transitions;
	Guid m_resourceBundle;
	Ref< const PropertyGroup > m_properties;
};

	}
}

#endif	// traktor_amalgam_StageData_H
