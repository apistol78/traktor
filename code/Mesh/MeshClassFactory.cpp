/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Mesh/IMeshParameterCallback.h"
#include "Mesh/MeshClassFactory.h"
#include "Mesh/MeshComponent.h"
#include "Mesh/MeshEntity.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/Blend/BlendMeshComponent.h"
#include "Mesh/Blend/BlendMeshEntity.h"
#include "Mesh/Composite/CompositeMeshComponent.h"
#include "Mesh/Composite/CompositeMeshEntity.h"
#include "Mesh/Indoor/IndoorMesh.h"
#include "Mesh/Indoor/IndoorMeshComponent.h"
#include "Mesh/Indoor/IndoorMeshEntity.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshComponent.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "Mesh/Lod/AutoLodMesh.h"
#include "Mesh/Lod/AutoLodMeshComponent.h"
#include "Mesh/Lod/AutoLodMeshEntity.h"
#include "Mesh/Lod/LodMeshEntity.h"
#include "Mesh/Partition/PartitionMesh.h"
#include "Mesh/Partition/PartitionMeshComponent.h"
#include "Mesh/Partition/PartitionMeshEntity.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshComponent.h"
#include "Mesh/Skinned/SkinnedMeshEntity.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshComponent.h"
#include "Mesh/Static/StaticMeshEntity.h"
#include "Mesh/Stream/StreamMesh.h"
#include "Mesh/Stream/StreamMeshComponent.h"
#include "Mesh/Stream/StreamMeshEntity.h"
#include "Render/ITexture.h"
#include "Render/Context/ProgramParameters.h"
#include "Resource/ResourceCast.h"

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
		m_programParameters->setVectorArrayParameter(name, param.c_ptr(), (int)param.size());
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

	Ref< AutoRuntimeClass< IMesh > > classIMesh = new AutoRuntimeClass< IMesh >();
	registrar->registerClass(classIMesh);

	Ref< AutoRuntimeClass< MeshComponent > > classMeshComponent = new AutoRuntimeClass< MeshComponent >();
	registrar->registerClass(classMeshComponent);

	Ref< AutoRuntimeClass< MeshEntity > > classMeshEntity = new AutoRuntimeClass< MeshEntity >();
	classMeshEntity->addMethod("setParameterCallback", MeshEntity_setParameterCallback);
	registrar->registerClass(classMeshEntity);

	Ref< AutoRuntimeClass< BlendMesh > > classBlendMesh = new AutoRuntimeClass< BlendMesh >();
	registrar->registerClass(classBlendMesh);

	Ref< AutoRuntimeClass< BlendMeshComponent > > classBlendMeshComponent = new AutoRuntimeClass< BlendMeshComponent >();
	classBlendMeshComponent->addConstructor< const resource::Proxy< BlendMesh >&, bool >();
	classBlendMeshComponent->addMethod("setBlendWeights", &BlendMeshComponent::setBlendWeights);
	classBlendMeshComponent->addMethod("getBlendWeights", &BlendMeshComponent::getBlendWeights);
	registrar->registerClass(classBlendMeshComponent);

	Ref< AutoRuntimeClass< BlendMeshEntity > > classBlendMeshEntity = new AutoRuntimeClass< BlendMeshEntity >();
	classBlendMeshEntity->addMethod("setBlendWeights", &BlendMeshEntity::setBlendWeights);
	classBlendMeshEntity->addMethod("getBlendWeights", &BlendMeshEntity::getBlendWeights);
	registrar->registerClass(classBlendMeshEntity);

	Ref< AutoRuntimeClass< CompositeMeshComponent > > classCompositeMeshComponent = new AutoRuntimeClass< CompositeMeshComponent >();
	classCompositeMeshComponent->addConstructor();
	classCompositeMeshComponent->addMethod("removeAll", &CompositeMeshComponent::removeAll);
	classCompositeMeshComponent->addMethod("remove", &CompositeMeshComponent::remove);
	classCompositeMeshComponent->addMethod("add", &CompositeMeshComponent::add);
	registrar->registerClass(classCompositeMeshComponent);

	Ref< AutoRuntimeClass< CompositeMeshEntity > > classCompositeMeshEntity = new AutoRuntimeClass< CompositeMeshEntity >();
	classCompositeMeshEntity->addMethod("addMeshEntity", &CompositeMeshEntity::addMeshEntity);
	classCompositeMeshEntity->addMethod("getMeshEntity", &CompositeMeshEntity::getMeshEntity);
	registrar->registerClass(classCompositeMeshEntity);

	Ref< AutoRuntimeClass< IndoorMesh > > classIndoorMesh = new AutoRuntimeClass< IndoorMesh >();
	registrar->registerClass(classIndoorMesh);

	Ref< AutoRuntimeClass< IndoorMeshComponent > > classIndoorMeshComponent = new AutoRuntimeClass< IndoorMeshComponent >();
	classIndoorMeshComponent->addConstructor< const resource::Proxy< IndoorMesh >&, bool >();
	registrar->registerClass(classIndoorMeshComponent);

	Ref< AutoRuntimeClass< IndoorMeshEntity > > classIndoorMeshEntity = new AutoRuntimeClass< IndoorMeshEntity >();
	registrar->registerClass(classIndoorMeshEntity);

	Ref< AutoRuntimeClass< InstanceMesh > > classInstanceMesh = new AutoRuntimeClass< InstanceMesh >();
	registrar->registerClass(classInstanceMesh);

	Ref< AutoRuntimeClass< InstanceMeshComponent > > classInstanceMeshComponent = new AutoRuntimeClass< InstanceMeshComponent >();
	classInstanceMeshComponent->addConstructor< const resource::Proxy< InstanceMesh >&, bool >();
	registrar->registerClass(classInstanceMeshComponent);

	Ref< AutoRuntimeClass< InstanceMeshEntity > > classInstanceMeshEntity = new AutoRuntimeClass< InstanceMeshEntity >();
	registrar->registerClass(classInstanceMeshEntity);

	Ref< AutoRuntimeClass< AutoLodMesh > > classAutoLodMesh = new AutoRuntimeClass< AutoLodMesh >();
	registrar->registerClass(classAutoLodMesh);

	Ref< AutoRuntimeClass< AutoLodMeshComponent > > classAutoLodMeshComponent = new AutoRuntimeClass< AutoLodMeshComponent >();
	classAutoLodMeshComponent->addConstructor< const resource::Proxy< AutoLodMesh >&, bool >();
	registrar->registerClass(classAutoLodMeshComponent);

	Ref< AutoRuntimeClass< AutoLodMeshEntity > > classAutoLodMeshEntity = new AutoRuntimeClass< AutoLodMeshEntity >();
	registrar->registerClass(classAutoLodMeshEntity);

	Ref< AutoRuntimeClass< LodMeshEntity > > classLodMeshEntity = new AutoRuntimeClass< LodMeshEntity >();
	registrar->registerClass(classLodMeshEntity);

	Ref< AutoRuntimeClass< PartitionMesh > > classPartitionMesh = new AutoRuntimeClass< PartitionMesh >();
	registrar->registerClass(classPartitionMesh);

	Ref< AutoRuntimeClass< PartitionMeshComponent > > classPartitionMeshComponent = new AutoRuntimeClass< PartitionMeshComponent >();
	classPartitionMeshComponent->addConstructor< const resource::Proxy< PartitionMesh >&, bool >();
	registrar->registerClass(classPartitionMeshComponent);

	Ref< AutoRuntimeClass< PartitionMeshEntity > > classPartitionMeshEntity = new AutoRuntimeClass< PartitionMeshEntity >();
	registrar->registerClass(classPartitionMeshEntity);

	Ref< AutoRuntimeClass< SkinnedMesh > > classSkinnedMesh = new AutoRuntimeClass< SkinnedMesh >();
	registrar->registerClass(classSkinnedMesh);

	Ref< AutoRuntimeClass< SkinnedMeshComponent > > classSkinnedMeshComponent = new AutoRuntimeClass< SkinnedMeshComponent >();
	classSkinnedMeshComponent->addConstructor< const resource::Proxy< SkinnedMesh >&, bool >();
	registrar->registerClass(classSkinnedMeshComponent);

	Ref< AutoRuntimeClass< SkinnedMeshEntity > > classSkinnedMeshEntity = new AutoRuntimeClass< SkinnedMeshEntity >();
	registrar->registerClass(classSkinnedMeshEntity);

	Ref< AutoRuntimeClass< StaticMesh > > classStaticMesh = new AutoRuntimeClass< StaticMesh >();
	registrar->registerClass(classStaticMesh);

	Ref< AutoRuntimeClass< StaticMeshComponent > > classStaticMeshComponent = new AutoRuntimeClass< StaticMeshComponent >();
	classStaticMeshComponent->addConstructor< const resource::Proxy< StaticMesh >&, bool >();
	registrar->registerClass(classStaticMeshComponent);

	Ref< AutoRuntimeClass< StaticMeshEntity > > classStaticMeshEntity = new AutoRuntimeClass< StaticMeshEntity >();
	registrar->registerClass(classStaticMeshEntity);

	Ref< AutoRuntimeClass< StreamMesh > > classStreamMesh = new AutoRuntimeClass< StreamMesh >();
	registrar->registerClass(classStreamMesh);

	Ref< AutoRuntimeClass< StreamMeshComponent > > classStreamMeshComponent = new AutoRuntimeClass< StreamMeshComponent >();
	classStreamMeshComponent->addConstructor< const resource::Proxy< StreamMesh >&, bool >();
	classStreamMeshComponent->addMethod("getFrameCount", &StreamMeshComponent::getFrameCount);
	classStreamMeshComponent->addMethod("setFrame", &StreamMeshComponent::setFrame);
	registrar->registerClass(classStreamMeshComponent);

	Ref< AutoRuntimeClass< StreamMeshEntity > > classStreamMeshEntity = new AutoRuntimeClass< StreamMeshEntity >();
	classStreamMeshEntity->addMethod("getFrameCount", &StreamMeshEntity::getFrameCount);
	classStreamMeshEntity->addMethod("setFrame", &StreamMeshEntity::setFrame);
	registrar->registerClass(classStreamMeshEntity);
}

	}
}
