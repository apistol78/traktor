#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/MeshClassFactory.h"
#include "Mesh/MeshEntity.h"
#include "Mesh/Blend/BlendMeshEntity.h"
#include "Mesh/Composite/CompositeMeshEntity.h"
#include "Mesh/Indoor/IndoorMeshEntity.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "Mesh/Lod/LodMeshEntity.h"
#include "Mesh/Partition/PartitionMeshEntity.h"
#include "Mesh/Skinned/SkinnedMeshEntity.h"
#include "Mesh/Static/StaticMeshEntity.h"
#include "Mesh/Stream/StreamMeshEntity.h"
#include "Render/ITexture.h"
#include "Render/Context/ProgramParameters.h"

namespace traktor
{
	namespace mesh
	{
		namespace
		{

class BoxedProgramParameters : public Object
{
	T_RTTI_CLASS;

public:
	BoxedProgramParameters()
	:	m_programParameters(0)
	{
	}

	void setProgramParameters(render::ProgramParameters* programParameters)
	{
		m_programParameters = programParameters;
	}

	void setFloatParameter(const std::wstring& name, float param)
	{
		m_programParameters->setFloatParameter(name, param);
	}

	void setVectorParameter(const std::wstring& name, const Vector4& param)
	{
		m_programParameters->setVectorParameter(name, param);
	}

	void setVectorArrayParameter(const std::wstring& name, const AlignedVector< Vector4 >& param)
	{
		m_programParameters->setVectorArrayParameter(name, param.c_ptr(), param.size());
	}

	void setTextureParameter(const std::wstring& name, render::ITexture* texture)
	{
		m_programParameters->setTextureParameter(name, texture);
	}

	void setStencilReference(uint32_t stencilReference)
	{
		m_programParameters->setStencilReference(stencilReference);
	}

private:
	render::ProgramParameters* m_programParameters;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.ProgramParameters", BoxedProgramParameters, Object)

class DelegateMeshParameterCallback : public RefCountImpl< IMeshParameterCallback >
{
public:
	DelegateMeshParameterCallback(IRuntimeDelegate* callback)
	:	m_callback(callback)
	{
	}

	virtual void setCombination(render::Shader* shader) const
	{
	}

	virtual void setParameters(render::ProgramParameters* programParameters) const
	{
		m_programParameters.setProgramParameters(programParameters);
		Any argv[] =
		{
			CastAny< BoxedProgramParameters* >::set(&m_programParameters)
		};
		m_callback->call(sizeof_array(argv), argv);
	}

private:
	Ref< IRuntimeDelegate > m_callback;
	mutable BoxedProgramParameters m_programParameters;
};

void MeshEntity_setParameterCallback(MeshEntity* self, IRuntimeDelegate* callback)
{
	if (callback)
		self->setParameterCallback(new DelegateMeshParameterCallback(callback));
	else
		self->setParameterCallback(0);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshClassFactory", 0, MeshClassFactory, IRuntimeClassFactory)

void MeshClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< BoxedProgramParameters > > classBoxedProgramParameters = new AutoRuntimeClass< BoxedProgramParameters >();
	classBoxedProgramParameters->addMethod("setFloatParameter", &BoxedProgramParameters::setFloatParameter);
	classBoxedProgramParameters->addMethod("setVectorParameter", &BoxedProgramParameters::setVectorParameter);
	classBoxedProgramParameters->addMethod("setVectorArrayParameter", &BoxedProgramParameters::setVectorArrayParameter);
	classBoxedProgramParameters->addMethod("setTextureParameter", &BoxedProgramParameters::setTextureParameter);
	classBoxedProgramParameters->addMethod("setStencilReference", &BoxedProgramParameters::setStencilReference);
	registrar->registerClass(classBoxedProgramParameters);

	Ref< AutoRuntimeClass< MeshEntity > > classMeshEntity = new AutoRuntimeClass< MeshEntity >();
	classMeshEntity->addMethod("setParameterCallback", MeshEntity_setParameterCallback);
	registrar->registerClass(classMeshEntity);

	Ref< AutoRuntimeClass< BlendMeshEntity > > classBlendMeshEntity = new AutoRuntimeClass< BlendMeshEntity >();
	classBlendMeshEntity->addMethod("setBlendWeights", &BlendMeshEntity::setBlendWeights);
	classBlendMeshEntity->addMethod("getBlendWeights", &BlendMeshEntity::getBlendWeights);
	registrar->registerClass(classBlendMeshEntity);

	Ref< AutoRuntimeClass< CompositeMeshEntity > > classCompositeMeshEntity = new AutoRuntimeClass< CompositeMeshEntity >();
	classCompositeMeshEntity->addMethod("addMeshEntity", &CompositeMeshEntity::addMeshEntity);
	classCompositeMeshEntity->addMethod("getMeshEntity", &CompositeMeshEntity::getMeshEntity);
	registrar->registerClass(classCompositeMeshEntity);

	Ref< AutoRuntimeClass< IndoorMeshEntity > > classIndoorMeshEntity = new AutoRuntimeClass< IndoorMeshEntity >();
	registrar->registerClass(classIndoorMeshEntity);

	Ref< AutoRuntimeClass< InstanceMeshEntity > > classInstanceMeshEntity = new AutoRuntimeClass< InstanceMeshEntity >();
	registrar->registerClass(classInstanceMeshEntity);

	Ref< AutoRuntimeClass< LodMeshEntity > > classLodMeshEntity = new AutoRuntimeClass< LodMeshEntity >();
	registrar->registerClass(classLodMeshEntity);

	Ref< AutoRuntimeClass< PartitionMeshEntity > > classPartitionMeshEntity = new AutoRuntimeClass< PartitionMeshEntity >();
	registrar->registerClass(classPartitionMeshEntity);

	Ref< AutoRuntimeClass< SkinnedMeshEntity > > classSkinnedMeshEntity = new AutoRuntimeClass< SkinnedMeshEntity >();
	registrar->registerClass(classSkinnedMeshEntity);

	Ref< AutoRuntimeClass< StaticMeshEntity > > classStaticMeshEntity = new AutoRuntimeClass< StaticMeshEntity >();
	registrar->registerClass(classStaticMeshEntity);

	Ref< AutoRuntimeClass< StreamMeshEntity > > classStreamMeshEntity = new AutoRuntimeClass< StreamMeshEntity >();
	classStreamMeshEntity->addMethod("getFrameCount", &StreamMeshEntity::getFrameCount);
	classStreamMeshEntity->addMethod("setFrame", &StreamMeshEntity::setFrame);
	registrar->registerClass(classStreamMeshEntity);
}

	}
}
