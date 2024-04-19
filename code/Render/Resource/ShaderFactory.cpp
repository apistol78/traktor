/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/ObjectStore.h"
#include "Database/Instance.h"
#include "Render/IProgram.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/Resource/ProgramResource.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/ShaderResource.h"
#include "Render/Resource/TextureLinker.h"
#include "Render/Resource/TextureProxy.h"
#include "Resource/IResourceManager.h"

namespace traktor::render
{
	namespace
	{

class TextureReaderAdapter : public TextureLinker::TextureReader
{
public:
	explicit TextureReaderAdapter(resource::IResourceManager* resourceManager, const Guid& shaderId)
	:	m_resourceManager(resourceManager)
	,	m_shaderId(shaderId)
	{
	}

	virtual Ref< ITexture > read(const Guid& textureGuid) const override final
	{
		resource::Proxy< ITexture > texture;
		if (m_resourceManager->bind(resource::Id< ITexture >(textureGuid), texture))
			return new TextureProxy(texture);
		else
		{
			log::error << L"Unable to bind texture \"" << textureGuid.format() << L"\" in shader \"" << m_shaderId.format() << L"\"." << Endl;
			return (ITexture*)nullptr;
		}
	}

private:
	resource::IResourceManager* m_resourceManager;
	const Guid& m_shaderId;
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ShaderFactory", 0, ShaderFactory, resource::IResourceFactory)

ShaderFactory::ShaderFactory(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

bool ShaderFactory::initialize(const ObjectStore& objectStore)
{
	m_renderSystem = objectStore.get< IRenderSystem >();
	return true;
}

const TypeInfoSet ShaderFactory::getResourceTypes() const
{
	return makeTypeInfoSet< ShaderResource >();
}

const TypeInfoSet ShaderFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Shader >();
}

bool ShaderFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > ShaderFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< const ShaderResource > shaderResource = instance->getObject< const ShaderResource >();
	if (!shaderResource)
		return nullptr;

	const Guid shaderId = instance->getGuid();
	const std::wstring shaderName = instance->getPath();
	Ref< Shader > shader = new Shader();

	// Create combination parameter mapping.
	for (auto parameterBit : shaderResource->getParameterBits())
		shader->m_parameterBits[getParameterHandle(parameterBit.first)] = parameterBit.second;

	// Create shader techniques.
	for (auto resourceTechnique : shaderResource->getTechniques())
	{
		const std::wstring programName = shaderName + L"[" + resourceTechnique.name + L"]";

		Shader::Technique& technique = shader->m_techniques[getParameterHandle(resourceTechnique.name)];
		technique.mask = resourceTechnique.mask;

		for (const auto& resourceCombination : resourceTechnique.combinations)
		{
			if (!resourceCombination.program)
				continue;

			const ProgramResource* programResource = checked_type_cast< const ProgramResource* >(resourceCombination.program);
			if (!programResource)
				return nullptr;

			Shader::Combination combination;
			combination.mask = resourceCombination.mask;
			combination.value = resourceCombination.value;
			combination.priority = resourceCombination.priority;
			combination.program = m_renderSystem->createProgram(programResource, programName.c_str());
			if (!combination.program)
				return nullptr;

			// Set implicit texture uniforms.
			TextureReaderAdapter textureReader(resourceManager, shaderId);
			if (!TextureLinker(textureReader).link(resourceCombination, combination.program))
				return nullptr;

			// Set uniform default values.
			for (const auto& ius : resourceCombination.initializeUniformScalar)
				combination.program->setFloatParameter(getParameterHandle(ius.name), ius.value);
			for (const auto& iuv : resourceCombination.initializeUniformVector)
				combination.program->setVectorParameter(getParameterHandle(iuv.name), iuv.value);

			technique.combinations.push_back(combination);
		}
	}

	return shader;
}

}
