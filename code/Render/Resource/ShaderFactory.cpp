#include "Database/Instance.h"
#include "Render/IProgram.h"
#include "Render/IProgramCompiler.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader/Nodes.h"
#include "Render/Shader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Render/Resource/ProgramResource.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/ShaderResource.h"
#include "Render/Resource/TextureLinker.h"
#include "Render/Resource/TextureProxy.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

class TextureReaderAdapter : public TextureLinker::TextureReader
{
public:
	TextureReaderAdapter(resource::IResourceManager* resourceManager)
	:	m_resourceManager(resourceManager)
	{
	}

	virtual Ref< ITexture > read(const Guid& textureGuid)
	{
		resource::Proxy< ITexture > texture;
		if (m_resourceManager->bind(resource::Id< ITexture >(textureGuid), texture))
			return new TextureProxy(texture);
		else
			return (ITexture*)0;
	}

private:
	resource::IResourceManager* m_resourceManager;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShaderFactory", ShaderFactory, resource::IResourceFactory)

ShaderFactory::ShaderFactory(
	IRenderSystem* renderSystem
)
:	m_renderSystem(renderSystem)
{
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
	Ref< ShaderResource > shaderResource = instance->getObject< ShaderResource >();
	if (!shaderResource)
		return 0;

	std::wstring shaderName = instance->getName();

	Ref< Shader > shader = new Shader();

	// Create combination parameter mapping.
	const std::map< std::wstring, uint32_t >& parameterBits = shaderResource->getParameterBits();
	for (std::map< std::wstring, uint32_t >::const_iterator i = parameterBits.begin(); i != parameterBits.end(); ++i)
		shader->m_parameterBits[getParameterHandle(i->first)] = i->second;

	// Create shader techniques.
	const std::vector< ShaderResource::Technique >& techniques = shaderResource->getTechniques();
	for (std::vector< ShaderResource::Technique >::const_iterator i = techniques.begin(); i != techniques.end(); ++i)
	{
		std::wstring programName = shaderName + L"." + i->name;

		Shader::Technique& technique = shader->m_techniques[getParameterHandle(i->name)];
		technique.mask = i->mask;

		for (std::vector< ShaderResource::Combination >::const_iterator j = i->combinations.begin(); j != i->combinations.end(); ++j)
		{
			if (!j->program)
				continue;

			Ref< ProgramResource > programResource;
			if (ShaderGraph* programShaderGraph = dynamic_type_cast< ShaderGraph* >(j->program))
			{
				Ref< IProgramCompiler > programCompiler = m_renderSystem->createProgramCompiler();
				if (!programCompiler)
					return 0;

				programResource = programCompiler->compile(programShaderGraph, 0, 4, false, 0);
			}
			else
				programResource = checked_type_cast< ProgramResource* >(j->program);

			if (!programResource)
				return 0;

			Shader::Combination combination;
			combination.mask = j->mask;
			combination.value = j->value;
			combination.priority = j->priority;
			combination.program = m_renderSystem->createProgram(programResource, programName.c_str());
			if (!combination.program)
				return 0;

			// Set implicit texture uniforms.
			TextureReaderAdapter textureReader(resourceManager);
			if (!TextureLinker(textureReader).link(*j, combination.program))
				return 0;

			// Set uniform default values.
			for (AlignedVector< ShaderResource::InitializeUniformScalar >::const_iterator k = j->initializeUniformScalar.begin(); k != j->initializeUniformScalar.end(); ++k)
				combination.program->setFloatParameter(k->name, k->value);
			for (AlignedVector< ShaderResource::InitializeUniformVector >::const_iterator k = j->initializeUniformVector.begin(); k != j->initializeUniformVector.end(); ++k)
				combination.program->setVectorParameter(k->name, k->value);

			technique.combinations.push_back(combination);
		}
	}

	shader->setTechnique(getParameterHandle(L"Default"));
	return shader;
}

	}
}
