#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedAabb3.h"
#include "Core/Class/Boxes/BoxedAlignedVector.h"
#include "Core/Class/Boxes/BoxedMatrix44.h"
#include "Core/Class/Boxes/BoxedQuaternion.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/Boxes/BoxedTransform.h"
#include "Core/Class/Boxes/BoxedVector2.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Model/IModelOperation.h"
#include "Model/Model.h"
#include "Model/ModelAdjacency.h"
#include "Model/ModelClassFactory.h"
#include "Model/ModelFormat.h"
#include "Model/Pose.h"
#include "Model/Operations/Boolean.h"
#include "Model/Operations/CalculateConvexHull.h"
#include "Model/Operations/CalculateTangents.h"
#include "Model/Operations/CleanDegenerate.h"
#include "Model/Operations/CleanDuplicates.h"
#include "Model/Operations/Clear.h"
#include "Model/Operations/CullDistantFaces.h"
#include "Model/Operations/FlattenDoubleSided.h"
#include "Model/Operations/MergeCoplanarAdjacents.h"
#include "Model/Operations/MergeModel.h"
#include "Model/Operations/NormalizeTexCoords.h"
#include "Model/Operations/Quantize.h"
#include "Model/Operations/Reduce.h"
#include "Model/Operations/ScaleAlongNormal.h"
#include "Model/Operations/SortCacheCoherency.h"
#include "Model/Operations/SortProjectedArea.h"
#include "Model/Operations/Transform.h"
#include "Model/Operations/Triangulate.h"
#include "Model/Operations/Unweld.h"
#include "Model/Operations/UnwrapUV.h"

namespace traktor
{
    namespace model
    {
        namespace
        {

void Model_clear(Model* self, uint32_t flags)
{
    self->clear(flags);
}

const AlignedVector< Material >& Model_getMaterials(Model* self)
{
    return self->getMaterials();
}

const AlignedVector< Vertex >& Model_getVertices(Model* self)
{
    return self->getVertices();
}

const AlignedVector< Polygon >& Model_getPolygons(Model* self)
{
    return self->getPolygons();
}

AlignedVector< uint32_t > Model_findChildJoints(Model* self, uint32_t jointId)
{
    AlignedVector< uint32_t > childJoints;
    self->findChildJoints(jointId, childJoints);
    return childJoints;
}

Ref< Model > ModelFormat_readAny(const Path& filePath)
{
    return ModelFormat::readAny(filePath);
}

bool ModelFormat_writeAny(const Path& filePath, const Model* model)
{
    return ModelFormat::writeAny(filePath, model);
}

void Polygon_setVertices(Polygon* self, const AlignedVector< uint32_t >& vertices)
{
    self->setVertices(Polygon::vertices_t(
        vertices.begin(),
        vertices.end()
    ));
}

AlignedVector< uint32_t > Polygon_getVertices(Polygon* self)
{
    const auto& vertices = self->getVertices();
    return AlignedVector< uint32_t >(vertices.begin(), vertices.end());
}

Ref< ModelAdjacency > ModelAdjacency_ctor_2(const Model* model, int32_t mode)
{
    return new ModelAdjacency(model, (ModelAdjacency::Mode)mode);
}

Ref< ModelAdjacency > ModelAdjacency_ctor_3(const Model* model, int32_t mode, uint32_t channel)
{
    return new ModelAdjacency(model, (ModelAdjacency::Mode)mode, channel);
}

AlignedVector< uint32_t > ModelAdjacency_getEnteringEdges(ModelAdjacency* self, uint32_t vertexId)
{
    ModelAdjacency::share_vector_t edges;
    self->getEnteringEdges(vertexId, edges);
    return AlignedVector< uint32_t >(edges.begin(), edges.end());
}

AlignedVector< uint32_t > ModelAdjacency_getLeavingEdges(ModelAdjacency* self, uint32_t vertexId)
{
    ModelAdjacency::share_vector_t edges;
    self->getLeavingEdges(vertexId, edges);
    return AlignedVector< uint32_t >(edges.begin(), edges.end());
}

AlignedVector< uint32_t > ModelAdjacency_getSharedEdges_2(ModelAdjacency* self, uint32_t edge)
{
    ModelAdjacency::share_vector_t edges;
    self->getSharedEdges(edge, edges);
    return AlignedVector< uint32_t >(edges.begin(), edges.end());
}

AlignedVector< uint32_t > ModelAdjacency_getSharedEdges_3(ModelAdjacency* self, uint32_t polygon, uint32_t polygonEdge)
{
    ModelAdjacency::share_vector_t edges;
    self->getSharedEdges(polygon, polygonEdge, edges);
    return AlignedVector< uint32_t >(edges.begin(), edges.end());
}

uint32_t ModelAdjacency_getSharedEdgeCount_1(ModelAdjacency* self, uint32_t edge)
{
    return self->getSharedEdgeCount(edge);
}

uint32_t ModelAdjacency_getSharedEdgeCount_2(ModelAdjacency* self, uint32_t polygon, uint32_t polygonEdge)
{
    return self->getSharedEdgeCount(polygon, polygonEdge);
}

AlignedVector< uint32_t > ModelAdjacency_getEdgeIndices(ModelAdjacency* self, uint32_t edge)
{
    AlignedVector< uint32_t > indices(2);
    self->getEdgeIndices(edge, indices[0], indices[1]);
    return indices;
}

        }

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.model.ModelClassFactory", 0, ModelClassFactory, IRuntimeClassFactory)

void ModelClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
    auto classAnimation = new AutoRuntimeClass< Animation >();
    classAnimation->addProperty("name", &Animation::setName, &Animation::getName);
    classAnimation->addProperty("keyFrameCount", &Animation::getKeyFrameCount);
    classAnimation->addMethod("insertKeyFrame", &Animation::insertKeyFrame);
    classAnimation->addMethod("getKeyFrameTime", &Animation::getKeyFrameTime);
    classAnimation->addMethod("setKeyFramePose", &Animation::setKeyFramePose);
    classAnimation->addMethod("getKeyFramePose", &Animation::getKeyFramePose);
    registrar->registerClass(classAnimation);

    auto classIModelOperation = new AutoRuntimeClass< IModelOperation >();
    classIModelOperation->addMethod("apply", &IModelOperation::apply);
    registrar->registerClass(classIModelOperation);

    auto classJoint = new AutoRuntimeClass< Joint >();
    classJoint->addConstructor();
    classJoint->addProperty("parent", &Joint::setParent, &Joint::getParent);
    classJoint->addProperty("name", &Joint::setName, &Joint::getName);
    classJoint->addProperty("transform", &Joint::setTransform, &Joint::getTransform);
    classJoint->addProperty("length", &Joint::setLength, &Joint::getLength);
    registrar->registerClass(classJoint);

    auto classMaterial = new AutoRuntimeClass< Material >();
    classMaterial->addConstructor();
    classMaterial->addConstructor< const std::wstring& >();
    classMaterial->addProperty("name", &Material::setName, &Material::getName);
    classMaterial->addProperty("color", &Material::setName, &Material::getName);
    classMaterial->addProperty("diffuseTerm", &Material::setDiffuseTerm, &Material::getDiffuseTerm);
    classMaterial->addProperty("specularTerm", &Material::setSpecularTerm, &Material::getSpecularTerm);
    classMaterial->addProperty("roughness", &Material::setRoughness, &Material::getRoughness);
    classMaterial->addProperty("metalness", &Material::setMetalness, &Material::getMetalness);
    classMaterial->addProperty("transparency", &Material::setTransparency, &Material::getTransparency);
    classMaterial->addProperty("emissive", &Material::setEmissive, &Material::getEmissive);
    classMaterial->addProperty("reflective", &Material::setReflective, &Material::getReflective);
    // classMaterial->addProperty("blendOperator", &Material::setName, &Material::getName);
    classMaterial->addProperty("doubleSided", &Material::setDoubleSided, &Material::isDoubleSided);
    registrar->registerClass(classMaterial);

    auto classModel = new AutoRuntimeClass< Model >();
    classModel->addConstant("InvalidIndex", Any::fromInt32(c_InvalidIndex));
    classModel->addConstant("CfMaterials", Any::fromInt32(Model::CfMaterials));
    classModel->addConstant("CfVertices", Any::fromInt32(Model::CfVertices));
    classModel->addConstant("CfPolygons", Any::fromInt32(Model::CfPolygons));
    classModel->addConstant("CfPositions", Any::fromInt32(Model::CfPositions));
    classModel->addConstant("CfColors", Any::fromInt32(Model::CfColors));
    classModel->addConstant("CfNormals", Any::fromInt32(Model::CfNormals));
    classModel->addConstant("CfTexCoords", Any::fromInt32(Model::CfTexCoords));
    classModel->addConstant("CfJoints", Any::fromInt32(Model::CfJoints));
    classModel->addConstant("CfAll", Any::fromInt32(Model::CfAll));
    classModel->addConstructor();
    classModel->addProperty("boundingBox", &Model::getBoundingBox);
    classModel->addProperty("materialCount", &Model::getMaterialCount);
    classModel->addProperty("vertexCount", &Model::getVertexCount);
    classModel->addProperty("polygonCount", &Model::getPolygonCount);
    classModel->addProperty("positionCount", &Model::getPositionCount);
    classModel->addProperty("colorCount", &Model::getColorCount);
    classModel->addProperty("normalCount", &Model::getNormalCount);
    classModel->addProperty("jointCount", &Model::getJointCount);
    classModel->addProperty("animationCount", &Model::getAnimationCount);
    classModel->addProperty("blendTargetCount", &Model::getBlendTargetCount);
    classModel->addMethod("clear", &Model_clear);
    classModel->addMethod("addMaterial", &Model::addMaterial);
    classModel->addMethod("addUniqueMaterial", &Model::addUniqueMaterial);
    classModel->addMethod("getMaterial", &Model::getMaterial);
    classModel->addMethod("setMaterials", &Model::setMaterials);
    classModel->addMethod("getMaterials", &Model_getMaterials);
    classModel->addMethod("reserveVertices", &Model::reserveVertices);
    classModel->addMethod("addVertex", &Model::addVertex);
    classModel->addMethod("addUniqueVertex", &Model::addUniqueVertex);
    classModel->addMethod("setVertex", &Model::setVertex);
    classModel->addMethod("getVertex", &Model::getVertex);
    classModel->addMethod("setVertices", &Model::setVertices);
    classModel->addMethod("getVertices", &Model_getVertices);
    classModel->addMethod("reservePolygons", &Model::reservePolygons);
    classModel->addMethod("addPolygon", &Model::addPolygon);
    classModel->addMethod("addUniquePolygon", &Model::addUniquePolygon);
    classModel->addMethod("setPolygon", &Model::setPolygon);
    classModel->addMethod("getPolygon", &Model::getPolygon);
    classModel->addMethod("setPolygons", &Model::setPolygons);
    classModel->addMethod("getPolygons", &Model_getPolygons);
    classModel->addMethod("reservePositions", &Model::reservePositions);
    classModel->addMethod("addPosition", &Model::addPosition);
    classModel->addMethod("addUniquePosition", &Model::addUniquePosition);
    classModel->addMethod("setPosition", &Model::setPosition);
    classModel->addMethod("getPosition", &Model::getPosition);
    classModel->addMethod("getVertexPosition", &Model::getVertexPosition);
    classModel->addMethod("setPositions", &Model::setPositions);
    classModel->addMethod("getPositions", &Model::getPositions);
    classModel->addMethod("reserveColors", &Model::reserveColors);
    classModel->addMethod("addColor", &Model::addColor);
    classModel->addMethod("addUniqueColor", &Model::addUniqueColor);
    classModel->addMethod("getColor", &Model::getColor);
    classModel->addMethod("setColors", &Model::setColors);
    classModel->addMethod("getColors", &Model::getColors);
    classModel->addMethod("reserveNormals", &Model::reserveNormals);
    classModel->addMethod("addNormal", &Model::addNormal);
    classModel->addMethod("addUniqueNormal", &Model::addUniqueNormal);
    classModel->addMethod("getNormal", &Model::getNormal);
    classModel->addMethod("setNormals", &Model::setNormals);
    classModel->addMethod("getNormals", &Model::getNormals);
    classModel->addMethod("addTexCoord", &Model::addTexCoord);
    classModel->addMethod("addUniqueTexCoord", &Model::addUniqueTexCoord);
    classModel->addMethod("getTexCoord", &Model::getTexCoord);
    classModel->addMethod("setTexCoords", &Model::setTexCoords);
    classModel->addMethod("getTexCoords", &Model::getTexCoords);
    classModel->addMethod("addUniqueTexCoordChannel", &Model::addUniqueTexCoordChannel);
    classModel->addMethod("getTexCoordChannels", &Model::getTexCoordChannels);
    classModel->addMethod("addJoint", &Model::addJoint);
    classModel->addMethod("addUniqueJoint", &Model::addUniqueJoint);
    classModel->addMethod("getJoint", &Model::getJoint);
    classModel->addMethod("setJoints", &Model::setJoints);
    classModel->addMethod("getJoints", &Model::getJoints);
    classModel->addMethod("findJointIndex", &Model::findJointIndex);
    classModel->addMethod("findChildJoints", &Model_findChildJoints);
    classModel->addMethod("getJointGlobalTransform", &Model::getJointGlobalTransform);
    classModel->addMethod("setJointRotation", &Model::setJointRotation);
    classModel->addMethod("addAnimation", &Model::addAnimation);
    classModel->addMethod("getAnimation", &Model::getAnimation);
    classModel->addMethod("findAnimation", &Model::findAnimation);
    classModel->addMethod("getAnimations", &Model::getAnimations);
    classModel->addMethod("addBlendTarget", &Model::addBlendTarget);
    classModel->addMethod("getBlendTarget", &Model::getBlendTarget);
    classModel->addMethod("setBlendTargetPosition", &Model::setBlendTargetPosition);
    classModel->addMethod("getBlendTargetPosition", &Model::getBlendTargetPosition);
    registrar->registerClass(classModel);

    auto classModelAdjacency = new AutoRuntimeClass< ModelAdjacency >();
    classModelAdjacency->addConstant("ByVertex", Any::fromInt32((int32_t)ModelAdjacency::Mode::ByVertex));
	classModelAdjacency->addConstant("ByPosition", Any::fromInt32((int32_t)ModelAdjacency::Mode::ByPosition));
	classModelAdjacency->addConstant("ByTexCoord", Any::fromInt32((int32_t)ModelAdjacency::Mode::ByTexCoord));
    classModelAdjacency->addConstructor< const Model*, int32_t >(&ModelAdjacency_ctor_2);
    classModelAdjacency->addConstructor< const Model*, int32_t, uint32_t >(&ModelAdjacency_ctor_3);
    classModelAdjacency->addProperty("edgeCount", &ModelAdjacency::getEdgeCount);
    classModelAdjacency->addMethod("add", &ModelAdjacency::add);
    classModelAdjacency->addMethod("remove", &ModelAdjacency::remove);
    classModelAdjacency->addMethod("update", &ModelAdjacency::update);
    classModelAdjacency->addMethod("getEdge", &ModelAdjacency::getEdge);
    classModelAdjacency->addMethod("getEnteringEdges", &ModelAdjacency_getEnteringEdges);
    classModelAdjacency->addMethod("getLeavingEdges", &ModelAdjacency_getLeavingEdges);
    classModelAdjacency->addMethod("getSharedEdges", &ModelAdjacency_getSharedEdges_2);
    classModelAdjacency->addMethod("getSharedEdges", &ModelAdjacency_getSharedEdges_3);
    classModelAdjacency->addMethod("getSharedEdgeCount", &ModelAdjacency_getSharedEdgeCount_1);
    classModelAdjacency->addMethod("getSharedEdgeCount", &ModelAdjacency_getSharedEdgeCount_2);
    classModelAdjacency->addMethod("getPolygon", &ModelAdjacency::getPolygon);
    classModelAdjacency->addMethod("getPolygonEdge", &ModelAdjacency::getPolygonEdge);
    classModelAdjacency->addMethod("getEdgeIndices", &ModelAdjacency_getEdgeIndices);
    registrar->registerClass(classModelAdjacency);

    auto classModelFormat = new AutoRuntimeClass< ModelFormat >();
    classModelFormat->addMethod("supportFormat", &ModelFormat::supportFormat);
    classModelFormat->addStaticMethod("readAny", &ModelFormat_readAny);
    classModelFormat->addStaticMethod("writeAny", &ModelFormat_writeAny);
    registrar->registerClass(classModelFormat);

    auto classPolygon = new AutoRuntimeClass< Polygon >();
    classPolygon->addConstructor();
    classPolygon->addConstructor< uint32_t, uint32_t, uint32_t >();
    classPolygon->addConstructor< uint32_t, uint32_t, uint32_t, uint32_t >();
    classPolygon->addConstructor< uint32_t, uint32_t, uint32_t, uint32_t, uint32_t >();
    classPolygon->addProperty("material", &Polygon::setMaterial, &Polygon::getMaterial);
    classPolygon->addProperty("normal", &Polygon::setNormal, &Polygon::getNormal);
    classPolygon->addProperty("smoothGroup", &Polygon::setSmoothGroup, &Polygon::getSmoothGroup);
    classPolygon->addProperty("vertexCount", &Polygon::getVertexCount);
    classPolygon->addMethod("clearVertices", &Polygon::clearVertices);
    classPolygon->addMethod("flipWinding", &Polygon::flipWinding);
    classPolygon->addMethod("addVertex", &Polygon::addVertex);
    classPolygon->addMethod("setVertex", &Polygon::setVertex);
    classPolygon->addMethod("getVertex", &Polygon::getVertex);
    classPolygon->addMethod("setVertices", &Polygon_setVertices);
    classPolygon->addMethod("getVertices", &Polygon_getVertices);
    registrar->registerClass(classPolygon);

    auto classPose = new AutoRuntimeClass< Pose >();
    classPose->addConstructor();
    classPose->addMethod("setJointTransform", &Pose::setJointTransform);
    classPose->addMethod("getJointTransform", &Pose::getJointTransform);
    classPose->addMethod("getJointGlobalTransform", &Pose::getJointGlobalTransform);
    registrar->registerClass(classPose);

    auto classVertex = new AutoRuntimeClass< Vertex >();
    classVertex->addConstructor();
    classVertex->addConstructor< uint32_t >();
    classVertex->addProperty("position", &Vertex::setPosition, &Vertex::getPosition);
    classVertex->addProperty("color", &Vertex::setColor, &Vertex::getColor);
    classVertex->addProperty("normal", &Vertex::setNormal, &Vertex::getNormal);
    classVertex->addProperty("tangent", &Vertex::setTangent, &Vertex::getTangent);
    classVertex->addProperty("binormal", &Vertex::setBinormal, &Vertex::getBinormal);
    classVertex->addProperty("texCoordCount", &Vertex::getTexCoordCount);
    classVertex->addProperty("jointInfluenceCount", &Vertex::getJointInfluenceCount);
    classVertex->addProperty("hash", &Vertex::getHash);
    classVertex->addMethod("clearTexCoords", &Vertex::clearTexCoords);
    classVertex->addMethod("setTexCoord", &Vertex::setTexCoord);
    classVertex->addMethod("getTexCoord", &Vertex::getTexCoord);
    classVertex->addMethod("clearJointInfluences", &Vertex::clearJointInfluences);
    classVertex->addMethod("setJointInfluence", &Vertex::setJointInfluence);
    classVertex->addMethod("getJointInfluence", &Vertex::getJointInfluence);
    registrar->registerClass(classVertex);

    // Model operations.
    auto classBoolean = new AutoRuntimeClass< Boolean >();
    registrar->registerClass(classBoolean);

    auto classCalculateConvexHull = new AutoRuntimeClass< CalculateConvexHull >();
    classCalculateConvexHull->addConstructor();
    registrar->registerClass(classCalculateConvexHull);

    auto classCalculateTangents = new AutoRuntimeClass< CalculateTangents >();
    classCalculateTangents->addConstructor< bool >();
    registrar->registerClass(classCalculateTangents);

    auto classCleanDegenerate = new AutoRuntimeClass< CleanDegenerate >();
    classCleanDegenerate->addConstructor();
    registrar->registerClass(classCleanDegenerate);

    auto classCleanDuplicates = new AutoRuntimeClass< CleanDuplicates >();
    classCleanDuplicates->addConstructor< float >();
    registrar->registerClass(classCleanDuplicates);

    auto classClear = new AutoRuntimeClass< Clear >();
    classClear->addConstructor< uint32_t >();
    registrar->registerClass(classClear);

    auto classCullDistantFaces = new AutoRuntimeClass< CullDistantFaces >();
    classCullDistantFaces->addConstructor< const Aabb3& >();
    registrar->registerClass(classCullDistantFaces);

    auto classFlattenDoubleSided = new AutoRuntimeClass< FlattenDoubleSided >();
    classFlattenDoubleSided->addConstructor();
    registrar->registerClass(classFlattenDoubleSided);

    auto classMergeCoplanarAdjacents = new AutoRuntimeClass< MergeCoplanarAdjacents >();
    classMergeCoplanarAdjacents->addConstructor();
    registrar->registerClass(classMergeCoplanarAdjacents);

    auto classMergeModel = new AutoRuntimeClass< MergeModel >();
    classMergeModel->addConstructor< const Model&, const traktor::Transform&, float >();
    registrar->registerClass(classMergeModel);

    auto classNormalizeTexCoords = new AutoRuntimeClass< NormalizeTexCoords >();
    classNormalizeTexCoords->addConstructor< uint32_t, float, float, float, float >();
    registrar->registerClass(classNormalizeTexCoords);

    auto classQuantize = new AutoRuntimeClass< Quantize >();
    classQuantize->addConstructor< float >();
    classQuantize->addConstructor< const Vector4& >();
    registrar->registerClass(classQuantize);

    auto classReduce = new AutoRuntimeClass< Reduce >();
    classReduce->addConstructor< float >();
    registrar->registerClass(classReduce);

    auto classScaleAlongNormal = new AutoRuntimeClass< ScaleAlongNormal >();
    classScaleAlongNormal->addConstructor< float >();
    registrar->registerClass(classScaleAlongNormal);

    auto classSortCacheCoherency = new AutoRuntimeClass< SortCacheCoherency >();
    classSortCacheCoherency->addConstructor();
    registrar->registerClass(classSortCacheCoherency);

    auto classSortProjectedArea = new AutoRuntimeClass< SortProjectedArea >();
    classSortProjectedArea->addConstructor< bool >();
    registrar->registerClass(classSortProjectedArea);

    auto classTransform = new AutoRuntimeClass< model::Transform >();
    classTransform->addConstructor< const Matrix44& >();
    registrar->registerClass(classTransform);

    auto classTriangulate = new AutoRuntimeClass< Triangulate >();
    classTriangulate->addConstructor();
    registrar->registerClass(classTriangulate);

    auto classUnweld = new AutoRuntimeClass< Unweld >();
    classUnweld->addConstructor();
    registrar->registerClass(classUnweld);

    auto classUnwrapUV = new AutoRuntimeClass< UnwrapUV >();
    classUnwrapUV->addConstructor< uint32_t, uint32_t >();
    registrar->registerClass(classUnwrapUV);
}

    }
}