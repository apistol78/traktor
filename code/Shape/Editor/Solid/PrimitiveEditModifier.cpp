#include "Shape/Editor/Solid/PrimitiveEditModifier.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.PrimitiveEditModifier", PrimitiveEditModifier, scene::IModifier)

PrimitiveEditModifier::PrimitiveEditModifier(scene::SceneEditorContext* context)
:   m_context(context)
{
}

void PrimitiveEditModifier::selectionChanged()
{
}

bool PrimitiveEditModifier::cursorMoved(
    const scene::TransformChain& transformChain,
    const Vector2& cursorPosition,
    const Vector4& worldRayOrigin,
    const Vector4& worldRayDirection
)
{
    return true;
}

bool PrimitiveEditModifier::handleCommand(const ui::Command& command)
{
    return false;
}

bool PrimitiveEditModifier::begin(
    const scene::TransformChain& transformChain,
    int32_t mouseButton
)
{
    // \tbd See if we hit any primitive vertex, edge or surface.

    return false;
}

void PrimitiveEditModifier::apply(
    const scene::TransformChain& transformChain,
    const Vector2& cursorPosition,
    const Vector4& worldRayOrigin,
    const Vector4& worldRayDirection,
    const Vector4& screenDelta,
    const Vector4& viewDelta
)
{
}

void PrimitiveEditModifier::end(const scene::TransformChain& transformChain)
{
}

void PrimitiveEditModifier::draw(render::PrimitiveRenderer* primitiveRenderer) const
{
}

    }
}