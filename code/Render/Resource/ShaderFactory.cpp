#include "Database/Database.h"
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
	db::Database* database,
	IRenderSystem* renderSystem
)
:	m_database(database)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet ShaderFactory::getResourceTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ShaderResource >());
	return typeSet;
}

const TypeInfoSet ShaderFactory::getProductTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Shader >());
	return typeSet;
}

bool ShaderFactory::isCacheable() const
{
	return true;
}

Ref< Object > ShaderFactory::create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const
{
	Ref< ShaderResource > shaderResource = m_database->getObjectReadOnly< ShaderResource >(guid);
	if (!shaderResource)
		return 0;

	Ref< Shader > shader = new Shader();

	// Create combination parameter mapping.
	const std::map< std::wstring, uint32_t >& parameterBits = shaderResource->getParameterBits();
	for (std::map< std::wstring, uint32_t >::const_iterator i = parameterBits.begin(); i != parameterBits.end(); ++i)
		shader->m_parameterBits[getParameterHandle(i->first)] = i->second;

	// Create shader techniques.
	const std::vector< ShaderResource::Technique >& techniques = shaderResource->getTechniques();
	for (std::vector< ShaderResource::Technique >::const_iterator i = techniques.begin(); i != techniques.end(); ++i)
	{
		Shader::Technique& technique = shader->m_techniques[getParameterHandle(i->name)];
		technique.mask = i->mask;

		for (std::vector< ShaderResource::Combination >::const_iterator j = i->combinations.begin(); j != i->combinations.end(); ++j)
		{
			if (!j->program)
				continue;

			Ref< ProgramResource > programResource;
			if (ShaderGraph* programShaderGraph = dynamic_type_cast< ShaderGraph* >(j->program))
			{
				Ref< render::IProgramCompiler > programCompiler = m_renderSystem->createProgramCompiler();
				if (!programCompiler)
					return 0;

				programResource = programCompiler->compile(programShaderGraph, 4, false, 0, 0);
			}
			else
				programResource = checked_type_cast< ProgramResource* >(j->program);

			if (!programResource)
				return 0;

			Shader::Combination combination;
			combination.mask = j->mask;
			combination.value = j->value;
			combination.priority = j->priority;
			combination.program = m_renderSystem->createProgram(programResource);
			if (!combination.program)
				return 0;

			TextureReaderAdapter textureReader(resourceManager);
			if (!TextureLinker(textureReader).link(*j, combination.program))
				return 0;

			technique.combinations.push_back(combination);
		}
	}

	shader->setTechnique(getParameterHandle(L"Default"));
	return shader;
}

	}
}
