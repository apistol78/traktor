#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Shape/Editor/Solid/PrimitiveEditModifier.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"

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
    m_entityAdapters.clear();
	m_context->getEntities(m_entityAdapters, scene::SceneEditorContext::GfDefault | scene::SceneEditorContext::GfSelectedOnly | scene::SceneEditorContext::GfNoExternalChild);
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
    for (auto entityAdapter : m_entityAdapters)
    {
        auto primitiveEntity = dynamic_type_cast< PrimitiveEntity* >(entityAdapter->getEntity());
        if (!primitiveEntity)
            continue;

        primitiveRenderer->pushWorld(primitiveEntity->getTransform().toMatrix44());

        for (const auto& winding : primitiveEntity->getWindings())
        {
            primitiveRenderer->drawSolidPoint(winding.center(), 8.0f, Color4ub(255, 255, 0, 255));
            for (const auto& vertex : winding.get())
                primitiveRenderer->drawSolidPoint(vertex, 8.0f, Color4ub(255, 255, 0, 255));
        }

        primitiveRenderer->popWorld();
    }
}

    }
}