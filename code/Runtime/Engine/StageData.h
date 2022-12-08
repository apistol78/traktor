/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IRuntimeClass;
class PropertyGroup;

}

namespace traktor::render
{

class Shader;

}

namespace traktor::runtime
{

class IEnvironment;
class LayerData;
class Stage;

/*!
 * \ingroup Runtime
 */
class T_DLLCLASS StageData : public ISerializable
{
	T_RTTI_CLASS;

public:
	Ref< Stage > createInstance(IEnvironment* environment, const Object* params) const;

	virtual void serialize(ISerializer& s) override final;

	const std::wstring& getName() const { return m_name; }

	const Guid& getInherit() const { return m_inherit; }

	void setLayers(const RefArray< LayerData >& layers) { m_layers = layers; }

	const RefArray< LayerData >& getLayers() const { return m_layers; }

	void setClass(const resource::Id< IRuntimeClass >& clazz) { m_class = clazz; }

	const resource::Id< IRuntimeClass >& getClass() const { return m_class; }

	void setShaderFade(const resource::Id< render::Shader >& shaderFade) { m_shaderFade = shaderFade; }

	const resource::Id< render::Shader >& getShaderFade() const { return m_shaderFade; }

	void setFadeOutUpdate(bool fadeOutUpdate) { m_fadeOutUpdate = fadeOutUpdate; }

	bool getFadeOutUpdate() const { return m_fadeOutUpdate; }

	void setFadeRate(float fadeRate) { m_fadeRate = fadeRate; }

	float getFadeRate() const { return m_fadeRate; }

	void setTransitions(const SmallMap< std::wstring, Guid >& transitions) { m_transitions = transitions; }

	const SmallMap< std::wstring, Guid >& getTransitions() const { return m_transitions; }

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
	resource::Id< render::Shader > m_shaderFade;
	bool m_fadeOutUpdate = false;
	float m_fadeRate = 1.0f;
	SmallMap< std::wstring, Guid > m_transitions;
	Guid m_resourceBundle;
	Ref< const PropertyGroup > m_properties;
};

}
