#if defined(T_USE_SIMPLYGON_SDK)

#include <SimplygonSDKLoader.h>
#include "Core/Io/Path.h"
#include "Model/Model.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/ReduceSimplygon.h"
#include "Model/Operations/Triangulate.h"

using namespace SimplygonSDK;

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ReduceSimplygon", ReduceSimplygon, IModelOperation)

ReduceSimplygon::ReduceSimplygon(float target)
:	m_target(target)
{
}

bool ReduceSimplygon::apply(Model& model) const
{
	ISimplygonSDK* sg = 0;

	Triangulate().apply(model);
	CalculateTangents().apply(model);

	// Determine number of UV channels.
	uint32_t maxUVChannel = 0;
	for (uint32_t i = 0; i < model.getVertexCount(); ++i)
		maxUVChannel = std::max< uint32_t >(maxUVChannel, model.getVertex(i).getTexCoordCount());

#if defined(_WIN32)
	wchar_t moduleFileName[MAX_PATH];
	GetModuleFileName(NULL, moduleFileName, MAX_PATH);
	SimplygonSDK::AddSearchPath(Path(moduleFileName).getPathOnly().c_str());
#else
	SimplygonSDK::AddSearchPath(_T("."));
#endif

	if (SimplygonSDK::Initialize(&sg) != SimplygonSDK::SG_ERROR_NOERROR)
		return false;

	{
		spGeometryData geometryData = sg->CreateGeometryData();

		// Convert from model into Simplygon geometry.
		{
			geometryData->SetVertexCount(model.getPolygonCount() * 3);
			geometryData->SetTriangleCount(model.getPolygonCount());

			geometryData->AddNormals();
			geometryData->AddMaterialIds();

			spRealArray vertexCoords = geometryData->GetCoords();
			spRealArray vertexNormals = geometryData->GetNormals();
			spRidArray triangleIds = geometryData->GetVertexIds();
			spRidArray materialIds = geometryData->GetMaterialIds();

			spRealArray vertexTexCoords[16] = { 0 };
			for (uint32_t i = 0; i < maxUVChannel; ++i)
			{
				geometryData->AddTexCoords(i);
				vertexTexCoords[i] = geometryData->GetTexCoords(i);
			}

			float tuple[4] = { 0.0f };
			uint32_t ii = 0;

			for (uint32_t i = 0; i < model.getPolygonCount(); ++i)
			{
				const Polygon& polygon = model.getPolygon(i);

				for (uint32_t j = 0; j < 3; ++j)
				{
					uint32_t vertexIndex = polygon.getVertex(j);
					const Vertex& vertex = model.getVertex(vertexIndex);

					model.getPosition(vertex.getPosition()).storeUnaligned(tuple);
					vertexCoords->SetTuple(ii, tuple);

					model.getNormal(vertex.getNormal()).storeUnaligned(tuple);
					vertexNormals->SetTuple(ii, tuple);

					for (uint32_t k = 0; k < vertex.getTexCoordCount(); ++k)
					{
						uint32_t texCoordIndex = vertex.getTexCoord(k);
						const Vector2& texCoord = model.getTexCoord(texCoordIndex);

						tuple[0] = texCoord.x;
						tuple[1] = texCoord.y;
						tuple[2] = 0.0f;
						tuple[3] = 0.0f;

						vertexTexCoords[k]->SetTuple(ii, tuple);
					}

					triangleIds->SetItem(ii, ii);

					++ii;
				}

				materialIds->SetItem(i, polygon.getMaterial());
			}
		}

		geometryData->CleanupInvalidTriangles();
		geometryData->CleanupNanValues();
		geometryData->ConvertHandedness();
		geometryData->Weld(0.01f);
		geometryData->Compact();

		spReductionProcessor reductionProcessor = sg->CreateReductionProcessor();
		reductionProcessor->SetGeometry(geometryData);

		spReductionSettings reductionSettings = reductionProcessor->GetReductionSettings();
		reductionSettings->SetEnablePreprocessing(true);
		reductionSettings->SetEnablePostprocessing(true);
		reductionSettings->SetKeepSymmetry(true);
		reductionSettings->SetUseAutomaticSymmetryDetection(true);
		reductionSettings->SetUseHighQualityNormalCalculation(true);
		reductionSettings->SetReductionHeuristics(SG_REDUCTIONHEURISTICS_CONSISTENT);

		unsigned int featureFlagsMask = 0;
		featureFlagsMask |= SG_FEATUREFLAGS_GROUP;
		featureFlagsMask |= SG_FEATUREFLAGS_MATERIAL;
		featureFlagsMask |= SG_FEATUREFLAGS_TEXTURE0;
		featureFlagsMask |= SG_FEATUREFLAGS_SHADING;
		reductionSettings->SetFeatureFlags( featureFlagsMask );

		reductionSettings->SetStopCondition(SG_STOPCONDITION_EITHER_IS_REACHED);
		reductionSettings->SetReductionRatio(m_target);
		reductionSettings->SetMaxDeviation(REAL_MAX);

		spRepairSettings repairSettings = reductionProcessor->GetRepairSettings();
		repairSettings->SetTjuncDist(0.0f);
		repairSettings->SetWeldDist(0.0f);

		spNormalCalculationSettings normalSettings = reductionProcessor->GetNormalCalculationSettings();
		normalSettings->SetReplaceNormals(false);

		reductionProcessor->RunProcessing();

		geometryData->ConvertHandedness();

		// Convert from Simplygon geometry into model.
		{
			model.clear(Model::CfVertices | Model::CfPolygons | Model::CfPositions | Model::CfNormals | Model::CfTexCoords | Model::CfJoints);

			spRealArray vertexCoords = geometryData->GetCoords();
			spRealArray vertexNormals = geometryData->GetNormals();
			spRidArray triangleIds = geometryData->GetVertexIds();
			spRidArray materialIds = geometryData->GetMaterialIds();
			spFieldData corners = geometryData->GetCorners();

			spRealArray vertexTexCoords[16] = { 0 };
			for (uint32_t i = 0; i < maxUVChannel; ++i)
				vertexTexCoords[i] = geometryData->GetTexCoords(i);

			float tuple[4] = { 0.0f };
			for (uint32_t i = 0; i < geometryData->GetTriangleCount(); ++i)
			{
				uint32_t indices[] =
				{
					triangleIds->GetItem(i * 3 + 0),
					triangleIds->GetItem(i * 3 + 1),
					triangleIds->GetItem(i * 3 + 2)
				};

				Polygon polygon;
				polygon.setMaterial(materialIds->GetItem(i));

				for (uint32_t j = 0; j < 3; ++j)
				{
					Vertex vertex;

					vertexCoords->GetTuple(indices[j], tuple);
					Vector4 position = Vector4::loadUnaligned(tuple);
					vertex.setPosition(model.addPosition(position));

					vertexNormals->GetTuple(i * 3 + j, tuple);
					Vector4 normal = Vector4::loadUnaligned(tuple);
					vertex.setNormal(model.addNormal(normal));

					for (uint32_t k = 0; k < maxUVChannel; ++k)
					{
						vertexTexCoords[k]->GetTuple(i * 3 + j, tuple);
						Vector2 uv(tuple[0], tuple[1]);
						vertex.setTexCoord(k, model.addTexCoord(uv));
					}

					polygon.addVertex(model.addVertex(vertex));
				}

				model.addPolygon(polygon);
			}
		}
	}

	SimplygonSDK::Deinitialize();
	return true;
}

	}
}

#endif
