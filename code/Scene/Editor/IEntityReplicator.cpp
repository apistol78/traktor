#include "Scene/Editor/IEntityReplicator.h"

namespace traktor
{
    namespace scene
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.IEntityReplicator", IEntityReplicator, Object)

const wchar_t* IEntityReplicator::VisualMesh = L"Visual.Mesh";
const wchar_t* IEntityReplicator::CollisionMesh = L"Collision.Mesh";
const wchar_t* IEntityReplicator::CollisionShape = L"Collision.Shape";
const wchar_t* IEntityReplicator::CollisionBody = L"Collision.Body";

    }
}
