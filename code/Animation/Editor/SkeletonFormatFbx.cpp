#include <fbxsdk.h>
#include "Animation/Joint.h"
#include "Animation/Skeleton.h"
#include "Animation/Editor/SkeletonFormatFbx.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Matrix44.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{
class FbxIStreamWrap : public FbxStream
{
	IStream* m_stream;
	EState m_state;
public:
	/** \enum EState
	  * - \e eClosed	indicate a closed stream  
	  * - \e eOpen		indicate an opened stream  
	  * - \e eEmpty		indicate an empty stream
	  */
	FbxIStreamWrap() { m_stream = 0; m_state = eEmpty; }

	//! Destructor
	virtual ~FbxIStreamWrap() { m_stream = 0; }

	/** Query the current state of the stream.
	  */
	virtual EState GetState() { return m_state; }


	/** Open the stream.
	  * \return True if successful.
	  */
	virtual bool Open( void* pStreamData ) { m_stream = (traktor::IStream* ) pStreamData; m_state = eOpen; return true; }

	/** Close the stream.
	  * \return True if successful.
	  */
	virtual bool Close() { m_stream->close(); m_state = eClosed; return true; }

	/** Empties the internal data of the stream.
	  * \return True if successful.
	  */
	virtual bool Flush() { m_stream->flush(); return true; }

	/** Writes a memory block.
	  * \param pData Pointer to the memory block to write.
	  * \param pSize Size (in bytes) of the memory block to write.
	  * \return The number of bytes written in the stream.
	  */
	virtual int Write(const void* /*pData*/, int /*pSize*/) { return 0;}

	/** Read bytes from the stream and store them in the memory block.
	  * \param pData Pointer to the memory block where the read bytes are stored.
	  * \param pSize Number of bytes read from the stream.
	  * \return The actual number of bytes successfully read from the stream.
	  */
	virtual int Read(void* pData, int pSize) const { return m_stream->read( pData, pSize); }

	/** Read a string from the stream.
	  * The default implementation is written in terms of Read() but
	  * does not cope with DOS line endings.
	  * Subclasses may need to override this if DOS line endings
	  * are to be supported.
	  * \param pData Pointer to the memory block where the read bytes are stored.
	  * \param pMaxSize Maximum number of bytes to be read from the stream.
	  * \param pStopAtFirstWhiteSpace Stop reading when any whitespace is encountered. 
										Otherwise read to end of line (like fgets()).
	  * \return pBuffer, if successful, else NULL.
	  */
//	virtual char* ReadString(char* pBuffer, int pMaxSize, bool pStopAtFirstWhiteSpace=false);

///////////////////////////////////////////////////////////////////////////////
//
//  WARNING!
//
//	Anything beyond these lines may not be documented accurately and is 
// 	subject to change without notice.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef DOXYGEN_SHOULD_SKIP_THIS
//	int Write(const char* pData, int pSize){ return Write((void*)pData, pSize); }
//	int Write(const int* pData, int pSize){ return Write((void*)pData, pSize); }
//	int Read(char* pData, int pSize) const { return Read((void*)pData, pSize); }
//	int Read(int* pData, int pSize) const { return Read((void*)pData, pSize); }
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

	/** If not specified by KFbxImporter::Initialize(), the importer will ask
	  * the stream to select an appropriate reader ID to associate with the stream.
	  * KFbxIOPluginRegistry can be used to locate id by extension or description.
	  * Return -1 to allow FBX to select an appropriate default.
	  */
	virtual int GetReaderID() const { return -1; }

	/** If not specified by KFbxExporter::Initialize(), the exporter will ask
	  * the stream to select an appropriate writer ID to associate with the stream.
	  * KFbxIOPluginRegistry can be used to locate id by extension or description.
	  * Return -1 to allow FBX to select an appropriate default.
	  */
	virtual int GetWriterID() const { return -1; }

	/** Adjust the current stream position.
	  * \param pSeekPos Pre-defined position where offset is added (FbxFile::eBegin, FbxFile::eCurrent:, FbxFile::eEnd)
	  * \param pOffset Number of bytes to offset from pSeekPos.
	  */
	virtual void Seek(const FbxInt64& pOffset, const FbxFile::ESeekPos& pSeekPos)
	{
		if (pSeekPos == FbxFile::eCurrent)
			m_stream->seek(IStream::SeekCurrent, int(pOffset));
		else if (pSeekPos == FbxFile::eBegin)
			m_stream->seek(IStream::SeekSet, int(pOffset));
		else if (pSeekPos == FbxFile::eEnd)
			m_stream->seek(IStream::SeekEnd, int(pOffset));
	}

	/** Get the current stream position.
	  * \return Current number of bytes from the beginning of the stream.
	  */
	virtual long GetPosition() const { return m_stream->tell(); }

	/** Set the current stream position.
	  * \param pPosition Number of bytes from the beginning of the stream to seek to.
	  */
	virtual void SetPosition(long pPosition) { m_stream->seek(IStream::SeekSet, int(pPosition)); }

	/** Return 0 if no errors occurred. Otherwise, return 1 to indicate
	  * an error. This method will be invoked whenever FBX needs to verify
	  * that the last operation succeeded.
	  */
	virtual int GetError() const { return 0; }

	/** Clear current error condition by setting the current error value to 0.
	  */
	virtual void ClearError() {}

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
	Ref< Joint > joint = new Joint();

	joint->setParent(parent);
	joint->setName(mbstows(fbxNode->GetName()));
	FbxVector4 nodeTranslation = fbxNode->GetScene()->GetEvaluator()->GetNodeLocalTranslation(fbxNode);	
	Vector4 translation = convertPosition(axisTransform, nodeTranslation);
	joint->setTransform(Transform(translation + offset));
	joint->setRadius(radius);
	joint->setEnableLimits(false);

	int32_t jointIndex = skeleton->addJoint(joint);
	log::info << jointIndex << joint->getName() << Endl;

	int childCount= fbxNode->GetChildCount();
	for (int i = 0; i < childCount; i++)
	{
		FbxNode* childNode = fbxNode->GetChild(i);
		createJoints(skeleton, childNode, jointIndex, Vector4::zero(), radius, axisTransform);
	}
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.SkeletonFormatFbx", SkeletonFormatFbx, ISkeletonFormat)

Ref< Skeleton > SkeletonFormatFbx::import(IStream* stream, const Vector4& offset, float radius, bool invertX, bool invertZ) const
{
	Vector4 jointModifier(
		invertX ? -1.0f : 1.0f, 
		1.0f, 
		invertZ ? -1.0f : 1.0f, 
		1.0f
	);

	FbxManager* sdkManager = FbxManager::Create();
	if (!sdkManager)
	{
		log::error << L"Unable to import FBX model; failed to create FBX SDK instance" << Endl;
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

	FbxStream *fbxStream = new FbxIStreamWrap();
	bool status = importer->Initialize(fbxStream, stream, readerID, sdkManager->GetIOSettings());
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

#if defined(_DEBUG)
	log::info << L"Up axis: " << (upSign < 0 ? L"-" : L"");
	switch (up)
	{
	case FbxAxisSystem::eXAxis:
		log::info << L"X" << Endl;
		break;
	case FbxAxisSystem::eYAxis:
		log::info << L"Y" << Endl;
		break;
	case FbxAxisSystem::eZAxis:
		log::info << L"Z" << Endl;
		break;
	}

	log::info << L"Front axis: " << (frontSign < 0 ? L"-" : L"");
	switch (front)
	{
	case FbxAxisSystem::eParityEven:
		log::info << L"Even" << Endl;
		break;
	case FbxAxisSystem::eParityOdd:
		log::info << L"Odd" << Endl;
		break;
	}

	if (leftHanded)
		log::info << L"Left handed" << Endl;
	else
		log::info << L"Right handed" << Endl;
#endif

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
					log::error << L"Unable to import FBX skeleton" << Endl;
					return 0;
				}
				int nodeCount = fbxSkeleton->GetNodeCount();
				skeleton = new Skeleton();
				FbxNode* boneNode = fbxSkeleton->GetNode();

				FbxVector4 scaleVector = scene->GetEvaluator()->GetNodeLocalScaling(childNode);
				FbxVector4 translation = scene->GetEvaluator()->GetNodeLocalTranslation(childNode);
				Matrix44 scaleMatrix = traktor::scale(convertVector(scaleVector));
				axisTransform = axisTransform * scaleMatrix;

				createJoints(skeleton, boneNode, -1, offset -  axisTransform * convertVector(translation), radius, axisTransform);
			}
		}
	}

	if (skeleton)
		log::info << L"Created " << skeleton->getJointCount() << L" joints(s) in skeleton" << Endl;

	return skeleton;
}

	}
}
