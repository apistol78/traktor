#include <fbxsdk.h>
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/Editor/SkeletonFormatFbx.h"
#include "Core/FbxLock.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Matrix44.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

class FbxIStreamWrap : public FbxStream
{
public:
	FbxIStreamWrap()
	:	m_stream(0)
	,	m_state(eEmpty)
	{
	}

	virtual ~FbxIStreamWrap()
	{
		m_stream = 0;
	}

	virtual EState GetState()
	{
		return m_state;
	}

	virtual bool Open(void* pStreamData)
	{
		T_ASSERT (!m_stream);
		if (!m_stream)
			m_stream = static_cast< IStream* >(pStreamData);
		m_stream->seek(IStream::SeekSet, 0);
		m_state = eOpen;
		return true;
	}

	virtual bool Close()
	{
		T_ASSERT (m_stream);
		m_stream->seek(IStream::SeekSet, 0);
		m_state = eClosed;
		return true;
	}

	virtual bool Flush()
	{
		T_ASSERT (m_stream);
		m_stream->flush();
		return true;
	}

	virtual int Write(const void* /*pData*/, int /*pSize*/)
	{
		return 0;
	}

	virtual int Read(void* pData, int pSize) const
	{
		T_ASSERT (m_stream);
		return m_stream->read(pData, pSize);
	}

	virtual int GetReaderID() const
	{
		return -1;
	}

	virtual int GetWriterID() const
	{
		return -1;
	}

	virtual void Seek(const FbxInt64& pOffset, const FbxFile::ESeekPos& pSeekPos)
	{
		T_ASSERT (m_stream);
		if (pSeekPos == FbxFile::eCurrent)
			m_stream->seek(IStream::SeekCurrent, int(pOffset));
		else if (pSeekPos == FbxFile::eBegin)
			m_stream->seek(IStream::SeekSet, int(pOffset));
		else if (pSeekPos == FbxFile::eEnd)
			m_stream->seek(IStream::SeekEnd, int(pOffset));
	}

	virtual long GetPosition() const
	{
		T_ASSERT (m_stream);
		return m_stream->tell();
	}

	virtual void SetPosition(long pPosition)
	{
		T_ASSERT (m_stream);
		m_stream->seek(IStream::SeekSet, int(pPosition));
	}

	virtual int GetError() const
	{
		return 0;
	}

	virtual void ClearError()
	{
	}

private:
	Ref< IStream > m_stream;
	EState m_state;
};

Vector4 convertPosition(const Matrix44& axisTransform, const FbxVector4& v)
{
	return axisTransform * Vector4(float(v[0]), float(v[1]), float(v[2]), 1.0f);
}

Vector4 convertNormal(const Matrix44& axisTransform, const FbxVector4& v)
{
	return axisTransform * Vector4(float(v[0]), float(v[1]), float(v[2]), 0.0f);
}

Vector4 convertVector(const FbxVector4& v)
{
	return Vector4(float(v[0]), float(v[1]), float(v[2]), float(v[3]));
}

Matrix44 convertMatrix(const FbxMatrix& m)
{
	return Matrix44(
		convertVector(m.GetRow(0)),
		convertVector(m.GetRow(1)),
		convertVector(m.GetRow(2)),
		convertVector(m.GetRow(3))
	);
}

FbxMatrix getGeometricTransform(const FbxNode* fbxNode)
{
	FbxVector4 t = fbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 r = fbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxVector4 s = fbxNode->GetGeometricScaling(FbxNode::eSourcePivot);
	return FbxMatrix(t, r, s);
}

void createJoints(
	Skeleton* skeleton,
	FbxNode* fbxNode,
	int32_t parent,
	const Vector4& offset,
	float radius,
	const Matrix44& axisTransform
)
{
	if (!fbxNode)
		return;

	FbxVector4 nodeTranslation = fbxNode->GetScene()->GetAnimationEvaluator()->GetNodeLocalTranslation(fbxNode);	
	Vector4 translation = convertPosition(axisTransform, nodeTranslation);

	Ref< Joint > joint = new Joint();
	joint->setParent(parent);
	joint->setName(mbstows(fbxNode->GetName()));
	joint->setTransform(Transform(translation + offset));
	joint->setRadius(radius);
	joint->setEnableLimits(false);

	int32_t jointIndex = skeleton->addJoint(joint);
	T_DEBUG(jointIndex << joint->getName());

	int32_t childCount= fbxNode->GetChildCount();
	for (int32_t i = 0; i < childCount; i++)
	{
		FbxNode* childNode = fbxNode->GetChild(i);
		createJoints(
			skeleton,
			childNode,
			jointIndex,
			Vector4::zero(),
			radius,
			axisTransform
		);
	}
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SkeletonFormatFbx", SkeletonFormatFbx, ISkeletonFormat)

Ref< Skeleton > SkeletonFormatFbx::import(IStream* stream, const Vector4& offset, float radius, bool invertX, bool invertZ) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(g_fbxLock);

	const Vector4 jointModifier(
		invertX ? -1.0f : 1.0f, 
		1.0f, 
		invertZ ? -1.0f : 1.0f, 
		1.0f
	);

	FbxManager* sdkManager = FbxManager::Create();
	if (!sdkManager)
	{
		log::error << L"Unable to import FBX skeleton; failed to create FBX SDK instance" << Endl;
		return 0;
	}

	FbxIOSettings* ios = FbxIOSettings::Create(sdkManager, IOSROOT);
	sdkManager->SetIOSettings(ios);

	FbxScene* scene = FbxScene::Create(sdkManager, "");
	if (!scene)
	{
		log::error << L"Unable to import FBX skeleton; failed to create FBX scene instance" << Endl;
		return 0;
	}

	FbxImporter* importer = FbxImporter::Create(sdkManager, "");
	if (!importer)
	{
		log::error << L"Unable to import FBX skeleton; failed to create FBX importer instance" << Endl;
		return 0;
	}

	FbxIOPluginRegistry* registry = sdkManager->GetIOPluginRegistry();
	int readerID = registry->FindReaderIDByExtension("fbx");

	// Wrap source stream into a buffered stream if necessary as
	// FBX keep reading very small chunks.
	Ref< IStream > rs = stream;
	if (!is_a< BufferedStream >(rs))
		rs = new BufferedStream(stream);

	AutoPtr< FbxStream > fbxStream(new FbxIStreamWrap());
	bool status = importer->Initialize(fbxStream.ptr(), rs, readerID, sdkManager->GetIOSettings());
	if (!status)
	{
		log::error << L"Unable to import FBX skeleton; failed to initialize FBX importer" << Endl;
		return 0;
	}

	status = importer->Import(scene);
	if (!status)
	{
		log::error << L"Unable to import FBX skeleton; FBX importer failed" << Endl;
		return 0;
	}

	// Calculate axis transformation.
	FbxAxisSystem axisSystem = scene->GetGlobalSettings().GetAxisSystem();
	bool lightwaveExported = false;

	// \hack If exported from Lightwave then we need to correct buggy exporter.
#if defined(T_USE_FBX_LIGHTWAVE_HACK)
	FbxDocumentInfo* documentInfo = scene->GetDocumentInfo();
	if (documentInfo && documentInfo->mAuthor.Find("Lightwave") >= 0)
		lightwaveExported = true;
#endif

	Matrix44 axisTransform = Matrix44::identity();

	int upSign;
	FbxAxisSystem::EUpVector up = axisSystem.GetUpVector(upSign);

	int frontSign;
	FbxAxisSystem::EFrontVector front = axisSystem.GetFrontVector(frontSign);

	bool leftHanded = bool(axisSystem.GetCoorSystem() == FbxAxisSystem::eLeftHanded);
	if (lightwaveExported)
		leftHanded = true;

	float sign = upSign < 0 ? -1.0f : 1.0f;
	float scale = leftHanded ? 1.0f : -1.0f;

	switch (up)
	{
	case FbxAxisSystem::eXAxis:
		axisTransform = Matrix44(
			0.0f, sign, 0.0f, 0.0f,
			-sign, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, scale, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;

	case FbxAxisSystem::eYAxis:
		axisTransform = Matrix44(
			sign * scale, 0.0f, 0.0f, 0.0f,
			0.0f, sign, 0.0f, 0.0f,
			0.0f, 0.0f, lightwaveExported ? -1.0f : 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;

	case FbxAxisSystem::eZAxis:
		axisTransform = Matrix44(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, -sign * scale, 0.0f,
			0.0f, sign, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		break;
	}

	Ref< Skeleton > skeleton;
	FbxNode* node = scene->GetRootNode();
	if (node)
	{
		int32_t childCount = node->GetChildCount();
		for (int32_t i = 0; i < childCount; ++i)
		{
			FbxNode* childNode = node->GetChild(i);
			if (!childNode || !childNode->GetVisibility() || !childNode->GetNodeAttribute())
				continue;

			FbxNodeAttribute::EType attributeType = childNode->GetNodeAttribute()->GetAttributeType();
			if (attributeType == FbxNodeAttribute::eSkeleton)
			{
				FbxSkeleton* fbxSkeleton = childNode->GetSkeleton();
				if (!fbxSkeleton)
				{
					log::error << L"Unable to import FBX skeleton; null skeleton" << Endl;
					return 0;
				}

				skeleton = new Skeleton();

				FbxVector4 scaleVector = scene->GetAnimationEvaluator()->GetNodeLocalScaling(childNode);
				FbxVector4 translation = scene->GetAnimationEvaluator()->GetNodeLocalTranslation(childNode);
				Matrix44 scaleMatrix = traktor::scale(convertVector(scaleVector));
				axisTransform = axisTransform * scaleMatrix;

				createJoints(
					skeleton,
					fbxSkeleton->GetNode(),
					-1,
					offset - axisTransform * convertVector(translation),
					radius,
					axisTransform
				);

				break;
			}
		}
	}

	if (skeleton)
		log::info << L"Created " << skeleton->getJointCount() << L" joints(s) in skeleton" << Endl;

	return skeleton;
}

	}
}
