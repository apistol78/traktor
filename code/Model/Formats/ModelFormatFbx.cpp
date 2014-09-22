#include <fbxsdk.h>
#include "Core/FbxLock.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Thread/Acquire.h"
#include "Model/Model.h"
#include "Model/Formats/ModelFormatFbx.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

FbxManager* s_fbxManager = 0;
FbxIOSettings* s_ioSettings = 0;
FbxScene* s_scene = 0;

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
		m_stream = static_cast< IStream* >(pStreamData);
		m_state = eOpen;
		return true;
	}

	virtual bool Close()
	{
		T_ASSERT (m_stream);
		m_stream->close();
		m_stream = 0;
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

Vector2 convertVector2(const FbxVector2& v)
{
	return Vector2(
		float(v[0]),
		1.0f - float(v[1])
	);
}

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

const FbxTexture* getTexture(const FbxSurfaceMaterial* material, const char* fbxPropertyName)
{
	if (!material || !fbxPropertyName)
		return 0;

	const FbxProperty prop = material->FindProperty(fbxPropertyName);
	if (!prop.IsValid())
		return 0;

	int fileTextureCount = prop.GetSrcObjectCount(FbxFileTexture::ClassId);
	for (int i = 0; i < fileTextureCount; ++i)
	{
		FbxFileTexture* fileTexture = FbxCast< FbxFileTexture >(prop.GetSrcObject(FbxFileTexture::ClassId, i));
		if (fileTexture)
			return fileTexture;
	}

	int layeredTextureCount = prop.GetSrcObjectCount(FbxLayeredTexture::ClassId);
	if (layeredTextureCount)
	{
		for (int i = 0; i < layeredTextureCount; ++i)
		{
			FbxLayeredTexture* layeredTexture = FbxCast< FbxLayeredTexture >(prop.GetSrcObject(FbxLayeredTexture::ClassId, i));
			if (layeredTexture)
				return layeredTexture;
		}
	}

	return 0;
}

std::wstring getTextureName(const FbxTexture* texture)
{
	const FbxFileTexture* fileTexture = FbxCast< const FbxFileTexture >(texture);
	if (fileTexture)
	{
		const Path texturePath(mbstows(fileTexture->GetFileName()));
		return texturePath.getFileNameNoExtension();
	}
	else
		return std::wstring(mbstows(texture->GetName()));
}

uint32_t uvChannel(std::vector< std::string >& inoutChannels, const std::string uvSet)
{
	std::vector< std::string >::iterator i = std::find(inoutChannels.begin(), inoutChannels.end(), uvSet);
	if (i != inoutChannels.end())
		return std::distance(inoutChannels.begin(), i);

	uint32_t channel = uint32_t(inoutChannels.size());
	inoutChannels.push_back(uvSet);

	return channel;
}

bool convertMesh(Model& outModel, FbxScene* scene, FbxNode* meshNode, const Matrix44& axisTransform, std::vector< std::string >& outChannels, uint32_t importFlags)
{
	int32_t vertexId = 0;

	FbxMesh* mesh = static_cast< FbxMesh* >(meshNode->GetNodeAttribute());
	if (!mesh)
		return false;

	FbxDocumentInfo* documentInfo = scene->GetDocumentInfo();

	bool mayaExported = false;
	if (documentInfo->LastSaved_ApplicationName.Get().Find("Maya") >= 0)
		mayaExported = true;

	// Convert materials.
	uint32_t materialBase = c_InvalidIndex;
	int32_t materialCount = 0;
	if (importFlags & ModelFormat::IfMaterials)
	{
		materialBase = outModel.getMaterials().size();
		materialCount = meshNode->GetMaterialCount();
		for (int32_t i = 0; i < materialCount; ++i)
		{
			FbxSurfaceMaterial* material = meshNode->GetMaterial(i);
			if (!material)
				continue;

			Material mm;
			mm.setName(mbstows(material->GetName()));

			FbxProperty prop = meshNode->GetFirstProperty();
			while (prop.IsValid())
			{
				int userTag = prop.GetUserTag();
				std::wstring propName = mbstows(prop.GetNameAsCStr());
				if (startsWith< std::wstring >(propName, L"DEA_"))
				{
					propName = replaceAll< std::wstring >(propName, L"DEA_", L"");
					FbxPropertyT< FbxBool > propState = prop;
					if (propState.IsValid())
					{
						bool propValue = propState.Get();
						mm.setProperty< PropertyBoolean >(propName, propValue);
					}
				}
				prop = meshNode->GetNextProperty(prop);
			}

			const FbxTexture* diffuseTexture = getTexture(material, FbxSurfaceMaterial::sDiffuse);
			if (diffuseTexture)
			{
				uint32_t channel = uvChannel(outChannels, diffuseTexture->UVSet.Get().Buffer());
				mm.setDiffuseMap(Material::Map(getTextureName(diffuseTexture), channel, true));
			}

			const FbxTexture* specularTexture = getTexture(material, FbxSurfaceMaterial::sSpecular);
			if (specularTexture)
			{
				uint32_t channel = uvChannel(outChannels, specularTexture->UVSet.Get().Buffer());
				mm.setSpecularMap(Material::Map(getTextureName(specularTexture), channel, false));
			}

			const FbxTexture* normalTexture = getTexture(material, FbxSurfaceMaterial::sNormalMap);
			if (normalTexture)
			{
				uint32_t channel = uvChannel(outChannels, normalTexture->UVSet.Get().Buffer());
				mm.setNormalMap(Material::Map(getTextureName(normalTexture), channel, false));
			}

			const FbxTexture* transparencyTexture = getTexture(material, FbxSurfaceMaterial::sTransparentColor);
			if (transparencyTexture)
			{
				uint32_t channel = uvChannel(outChannels, transparencyTexture->UVSet.Get().Buffer());
				mm.setTransparencyMap(Material::Map(getTextureName(transparencyTexture), channel, false));
				mm.setBlendOperator(Material::BoAlpha);
			}

			const FbxTexture* emissiveTexture = getTexture(material, mayaExported ? FbxSurfaceMaterial::sAmbient : FbxSurfaceMaterial::sEmissive);
			if (emissiveTexture)
			{
				uint32_t channel = uvChannel(outChannels, emissiveTexture->UVSet.Get().Buffer());
				mm.setEmissiveMap(Material::Map(getTextureName(emissiveTexture), channel, false));
			}

			if (material->GetClassId().Is(FbxSurfacePhong::ClassId))
			{
				FbxSurfacePhong* phongMaterial = (FbxSurfacePhong*)material;

				FbxPropertyT<FbxDouble3> phongDiffuse = phongMaterial->Diffuse;
				if (phongDiffuse.IsValid())
				{
					FbxDouble3 diffuse = phongDiffuse.Get();
					mm.setColor(Color4ub(
						uint8_t(diffuse[0] * 255),
						uint8_t(diffuse[1] * 255),
						uint8_t(diffuse[2] * 255),
						255
					));
				}

				FbxPropertyT<FbxDouble> phongDiffuseFactor = phongMaterial->DiffuseFactor;
				if (phongDiffuseFactor.IsValid())
				{
					FbxDouble diffuseFactor = phongDiffuseFactor.Get();
					mm.setDiffuseTerm(clamp(float(diffuseFactor), 0.0f, 1.0f));
				}

				FbxPropertyT<FbxDouble> phongSpecularFactor = phongMaterial->SpecularFactor;
				if (phongSpecularFactor.IsValid())
				{
					FbxDouble specularFactor = phongSpecularFactor.Get();
					mm.setSpecularTerm(clamp(float(specularFactor), 0.0f, 1.0f));
				}

				FbxPropertyT<FbxDouble> phongShininess = phongMaterial->Shininess;
				if (phongShininess.IsValid())
				{
					FbxDouble shininess = phongShininess.Get();
					mm.setSpecularRoughness(float(shininess / 16.0));
				}

				FbxPropertyT<FbxDouble3> phongEmissive = mayaExported ? phongMaterial->Ambient : phongMaterial->Emissive;
				if (phongEmissive.IsValid())
				{
					FbxDouble3 emissive = phongEmissive.Get();
					FbxDouble emissiveFactor = mayaExported ? phongMaterial->AmbientFactor.Get() : phongMaterial->EmissiveFactor.Get();

					emissive[0] *= emissiveFactor;
					emissive[1] *= emissiveFactor;
					emissive[2] *= emissiveFactor;

					mm.setEmissive(float(emissive[0] + emissive[1] + emissive[2]) / 3.0f);
				}
			}
			else if (material->GetClassId().Is(FbxSurfaceLambert::ClassId))
			{
				FbxSurfaceLambert* lambertMaterial = (FbxSurfaceLambert*)material;

				FbxPropertyT<FbxDouble3> lambertDiffuse = lambertMaterial->Diffuse;
				if (lambertDiffuse.IsValid())
				{
					FbxDouble3 diffuse = lambertDiffuse.Get();
					mm.setColor(Color4ub(
						uint8_t(diffuse[0] * 255),
						uint8_t(diffuse[1] * 255),
						uint8_t(diffuse[2] * 255),
						255
					));
				}

				FbxPropertyT<FbxDouble> lambertDiffuseFactor = lambertMaterial->DiffuseFactor;
				if (lambertDiffuseFactor.IsValid())
				{
					FbxDouble diffuseFactor = lambertDiffuseFactor.Get();
					mm.setDiffuseTerm(clamp(float(diffuseFactor), 0.0f, 1.0f));
				}

				FbxPropertyT<FbxDouble3> lambertEmissive = mayaExported ? lambertMaterial->Ambient : lambertMaterial->Emissive;
				if (lambertEmissive.IsValid())
				{
					FbxDouble3 emissive = lambertEmissive.Get();
					FbxDouble emissiveFactor = mayaExported ? lambertMaterial->AmbientFactor.Get() : lambertMaterial->EmissiveFactor.Get();

					emissive[0] *= emissiveFactor;
					emissive[1] *= emissiveFactor;
					emissive[2] *= emissiveFactor;

					mm.setEmissive(float(emissive[0] + emissive[1] + emissive[2]) / 3.0f);
				}

				mm.setSpecularTerm(0.0f);
			}

			outModel.addMaterial(mm);
		}
	}

	FbxAnimEvaluator* sceneEvaluator = scene->GetEvaluator();
	FbxMatrix nodeGlobalTransform = sceneEvaluator->GetNodeGlobalTransform(meshNode);
	FbxMatrix geometricTransform = getGeometricTransform(meshNode);
	FbxMatrix globalTransform = nodeGlobalTransform * geometricTransform;
	
	Vector4 Lpivot = convertVector(meshNode->RotationPivot.Get());
	Matrix44 Mglobal = convertMatrix(globalTransform);

	// Convert vertex positions.
	uint32_t positionBase = c_InvalidIndex;
	if (importFlags & ModelFormat::IfMeshPositions)
	{
		FbxVector4* controlPoints = mesh->GetControlPoints();
		if (!controlPoints)
			return false;

		positionBase = outModel.getPositions().size();

		int32_t controlPointsCount = mesh->GetControlPointsCount();
		for (int32_t i = 0; i < controlPointsCount; ++i)
		{
			Vector4 v = Mglobal * convertVector(controlPoints[i]).xyz1();
			outModel.addPosition(axisTransform * (v - Lpivot).xyz1());
		}
	}

	typedef std::map< uint32_t, float > bone_influences_t;
	std::vector< bone_influences_t > vertexJoints;

	if (importFlags & ModelFormat::IfMeshBlendWeights)
	{
		int32_t controlPointsCount = mesh->GetControlPointsCount();
		vertexJoints.resize(controlPointsCount);

		int32_t deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
		for (int32_t i = 0; i < deformerCount; ++i)
		{
			FbxSkin* skinDeformer = (FbxSkin*) mesh->GetDeformer(i, FbxDeformer::eSkin);
			if (!skinDeformer)
				continue;

			int32_t clusterCount = skinDeformer->GetClusterCount();
			for (int32_t j = 0; j < clusterCount; ++j)
			{
				FbxCluster* cluster = skinDeformer->GetCluster(j);
				if (!cluster)
					continue;

				int32_t weightCount = cluster->GetControlPointIndicesCount();
				if (weightCount <= 0)
					continue;

				const FbxNode* jointNode = cluster->GetLink();
				T_ASSERT (jointNode);
				
				std::wstring jointName = mbstows(jointNode->GetName());
				
				size_t p = jointName.find(L':');
				if (p != std::wstring::npos)
					jointName = jointName.substr(p + 1);

				uint32_t jointIndex = outModel.addJoint(jointName);

				const double* weights = cluster->GetControlPointWeights();
				const int32_t* indices = cluster->GetControlPointIndices();

				for (int32_t k = 0; k < weightCount; ++k)
				{
					int32_t vertexIndex = indices[k];
					float jointWeight = float(weights[k]);
					if (jointWeight > FUZZY_EPSILON)
						vertexJoints[vertexIndex].insert(std::pair< uint32_t, float >(jointIndex, jointWeight));
				}
			}
		}
	}

	// Convert polygons.
	if (importFlags & (ModelFormat::IfMeshPolygons | ModelFormat::IfMeshVertices))
	{
		int32_t polygonCount = mesh->GetPolygonCount();
		for (int32_t i = 0; i < polygonCount; ++i)
		{
			Polygon polygon;

			// Assign material; \fixme should probably create a polygon for each layer of material.
			if (importFlags & ModelFormat::IfMaterials)
			{
				for (int32_t j = 0; j < mesh->GetLayerCount(); ++j)
				{
					FbxLayerElementMaterial* layerMaterials = mesh->GetLayer(j)->GetMaterials();
					if (layerMaterials)
					{
						int32_t materialIndex = layerMaterials->GetIndexArray().GetAt(i);
						polygon.setMaterial(materialBase + materialIndex);
					}
				}
				// If no material found but mesh have a material defined then set first material.
				if (materialCount > 0 && polygon.getMaterial() == c_InvalidIndex)
					polygon.setMaterial(materialBase);
			}

			int32_t polygonSize = mesh->GetPolygonSize(i);
			for (int32_t j = 0; j < polygonSize; ++j, ++vertexId)
			{
				int32_t pointIndex = mesh->GetPolygonVertex(i, j);

				Vertex vertex;
				vertex.setPosition(positionBase + pointIndex);

				if (pointIndex < int32_t(vertexJoints.size()))
				{
					for (bone_influences_t::const_iterator k = vertexJoints[pointIndex].begin(); k != vertexJoints[pointIndex].end(); ++k)
						vertex.setJointInfluence(k->first, k->second);
				}

				for (int32_t k = 0; k < mesh->GetLayerCount(); ++k)
				{
					// Vertex colors.
					FbxLayerElementVertexColor* layerVertexColors = mesh->GetLayer(k)->GetVertexColors();
					if (layerVertexColors)
					{
					}

					// Vertex texture UVs.
					FbxLayerElementUV* layerUVs = mesh->GetLayer(k)->GetUVs();
					if (layerUVs)
					{
						uint32_t channel = uvChannel(outChannels, layerUVs->GetName());
						switch (layerUVs->GetMappingMode())
						{
						case FbxLayerElement::eByControlPoint:
							switch (layerUVs->GetReferenceMode())
							{
							case FbxLayerElement::eDirect:
								{
									Vector2 uv = convertVector2(layerUVs->GetDirectArray().GetAt(pointIndex));
									vertex.setTexCoord(channel, outModel.addUniqueTexCoord(uv));
								}
								break;

							case FbxLayerElement::eIndexToDirect:
								{
									int32_t id = layerUVs->GetIndexArray().GetAt(pointIndex);
									Vector2 uv = convertVector2(layerUVs->GetDirectArray().GetAt(id));
									vertex.setTexCoord(channel, outModel.addUniqueTexCoord(uv));
								}
								break;

							default:
								break;
							}
							break;

						case FbxLayerElement::eByPolygonVertex:
							{
								int32_t textureUVIndex = mesh->GetTextureUVIndex(i, j);
								switch (layerUVs->GetReferenceMode())
								{
								case FbxLayerElement::eDirect:
								case FbxLayerElement::eIndexToDirect:
									{
										Vector2 uv = convertVector2(layerUVs->GetDirectArray().GetAt(textureUVIndex));
										vertex.setTexCoord(channel, outModel.addUniqueTexCoord(uv));
									}
									break;

								default:
									break;
								}
							}
							break;
							
						default:
							break;
						}
					}

					// Vertex normals.
					FbxLayerElementNormal* layerNormals = mesh->GetLayer(k)->GetNormals();
					if (layerNormals)
					{
						if (layerNormals->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
						{
							switch (layerNormals->GetReferenceMode())
							{
							case FbxLayerElement::eDirect:
								{
									FbxVector4 n = layerNormals->GetDirectArray().GetAt(vertexId);
									Vector4 normal = (axisTransform * (Mglobal * convertVector(n)).xyz0()).normalized();
									vertex.setNormal(outModel.addUniqueNormal(normal));
								}
								break;

							case FbxLayerElement::eIndexToDirect:
								{
									int32_t id = layerNormals->GetIndexArray().GetAt(vertexId);
									FbxVector4 n = layerNormals->GetDirectArray().GetAt(id);
									Vector4 normal = (axisTransform * (Mglobal * convertVector(n)).xyz0()).normalized();
									vertex.setNormal(outModel.addUniqueNormal(normal));
								}
								break;

							default:
								break;
							}
						}
					}

					// Vertex tangents.
					FbxLayerElementTangent* layerTangents = mesh->GetLayer(k)->GetTangents();
					if (layerTangents)
					{
						if (layerTangents->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
						{
							switch (layerTangents->GetReferenceMode())
							{
							case FbxLayerElement::eDirect:
								{
									FbxVector4 t = layerTangents->GetDirectArray().GetAt(vertexId);
									Vector4 tangent = (axisTransform * (Mglobal * convertVector(t)).xyz0()).normalized();
									vertex.setTangent(outModel.addUniqueNormal(tangent));
								}
								break;

							case FbxLayerElement::eIndexToDirect:
								{
									int32_t id = layerTangents->GetIndexArray().GetAt(vertexId);
									FbxVector4 t = layerTangents->GetDirectArray().GetAt(id);
									Vector4 tangent = (axisTransform * (Mglobal * convertVector(t)).xyz0()).normalized();
									vertex.setTangent(outModel.addUniqueNormal(tangent));
								}
								break;

							default:
								break;
							}
						}
					}

					// Vertex binormals.
					FbxLayerElementBinormal* layerBinormals = mesh->GetLayer(k)->GetBinormals();
					if (layerBinormals)
					{
						if (layerBinormals->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
						{
							switch (layerBinormals->GetReferenceMode())
							{
							case FbxLayerElement::eDirect:
								{
									FbxVector4 b = layerBinormals->GetDirectArray().GetAt(vertexId);
									Vector4 binormal = (axisTransform * (Mglobal * convertVector(b)).xyz0()).normalized();
									vertex.setBinormal(outModel.addUniqueNormal(binormal));
								}
								break;

							case FbxLayerElement::eIndexToDirect:
								{
									int32_t id = layerBinormals->GetIndexArray().GetAt(vertexId);
									FbxVector4 b = layerBinormals->GetDirectArray().GetAt(id);
									Vector4 binormal = (axisTransform * (Mglobal * convertVector(b)).xyz0()).normalized();
									vertex.setBinormal(outModel.addUniqueNormal(binormal));
								}
								break;

							default:
								break;
							}
						}
					}
				}

				polygon.addVertex(outModel.addUniqueVertex(vertex));
			}

			outModel.addPolygon(polygon);
		}
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelFormatFbx", 0, ModelFormatFbx, ModelFormat)

void ModelFormatFbx::getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const
{
	outDescription = L"Autodesk FBX";
	outExtensions.push_back(L"fbx");
}

bool ModelFormatFbx::supportFormat(const std::wstring& extension) const
{
	return compareIgnoreCase< std::wstring >(extension, L"fbx") == 0;
}

Ref< Model > ModelFormatFbx::read(IStream* stream, uint32_t importFlags) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(g_fbxLock);

	if (!s_fbxManager)
	{
		s_fbxManager = FbxManager::Create();
		if (!s_fbxManager)
		{
			log::error << L"Unable to import FBX model; failed to create FBX SDK instance." << Endl;
			return 0;
		}

		s_ioSettings = FbxIOSettings::Create(s_fbxManager, IOSROOT);
		s_fbxManager->SetIOSettings(s_ioSettings);

		s_scene = FbxScene::Create(s_fbxManager, "");
	}

	FbxIOPluginRegistry* registry = s_fbxManager->GetIOPluginRegistry();
	int readerID = registry->FindReaderIDByExtension("fbx");
    if (readerID < 0)
	{
		log::error << L"Unable to import FBX model; no reader for \"fbx\" extension registered." << Endl;
		return 0;
	}

	FbxImporter* importer = FbxImporter::Create(s_fbxManager, "");
	if (!importer)
	{
		log::error << L"Unable to import FBX model; failed to create FBX importer instance." << Endl;
		return 0;
	}

	AutoPtr< FbxStream > fbxStream(new FbxIStreamWrap());
	bool status = importer->Initialize(fbxStream.ptr(), stream, readerID, s_fbxManager->GetIOSettings());
	if (!status)
	{
		log::error << L"Unable to import FBX model; failed to initialize FBX importer (" << mbstows(importer->GetLastErrorString()) << L")." << Endl;
		return 0;
	}

	s_scene->Clear();

	status = importer->Import(s_scene);
	if (!status)
	{
		log::error << L"Unable to import FBX model; FBX importer failed (" << mbstows(importer->GetLastErrorString()) << L")." << Endl;
		return 0;
	}

	importer->Destroy();
	importer = 0;

	// Calculate axis transformation.
	FbxAxisSystem axisSystem = s_scene->GetGlobalSettings().GetAxisSystem();
	bool lightwaveExported = false;

	// \hack If exported from Lightwave then we need to correct buggy exporter.
#if defined(T_USE_FBX_LIGHTWAVE_HACK)
	FbxDocumentInfo* documentInfo = s_scene->GetDocumentInfo();
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

	Ref< Model > model = new Model();
	std::vector< std::string > channels;

	FbxNode* node = s_scene->GetRootNode();
	if (node)
	{
		int32_t childCount = node->GetChildCount();
		for (int32_t i = 0; i < childCount; ++i)
		{
			FbxNode* childNode = node->GetChild(i);
			if (!childNode || !childNode->GetVisibility() || !childNode->GetNodeAttribute())
				continue;

			FbxNodeAttribute::EType attributeType = childNode->GetNodeAttribute()->GetAttributeType();
			if (attributeType == FbxNodeAttribute::eMesh)
			{
				if (!convertMesh(*model, s_scene, childNode, axisTransform, channels, importFlags))
				{
					log::error << L"Unable to import FBX model; failed to convert mesh" << Endl;
					return 0;
				}
			}
		}
	}

	// Create and assign default material if anonymous faces has been created.
	if (importFlags & IfMaterials)
	{
		uint32_t defaultMaterialIndex = c_InvalidIndex;
		for (uint32_t i = 0; i < model->getPolygonCount(); ++i)
		{
			const Polygon& polygon = model->getPolygon(i);
			if (polygon.getMaterial() == c_InvalidIndex)
			{
				if (defaultMaterialIndex == c_InvalidIndex)
				{
					Material material;
					material.setName(L"FBX_Default");
					material.setColor(Color4ub(255, 255, 255));
					material.setDiffuseTerm(1.0f);
					material.setSpecularTerm(1.0f);
					defaultMaterialIndex = model->addMaterial(material);
				}

				Polygon replacement = polygon;
				replacement.setMaterial(defaultMaterialIndex);
				model->setPolygon(i, replacement);
			}
		}
	}

	return model;
}

bool ModelFormatFbx::write(IStream* stream, const Model* model) const
{
	return false;
}

	}
}
