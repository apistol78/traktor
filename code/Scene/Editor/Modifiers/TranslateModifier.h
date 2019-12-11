#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Scene/Editor/IModifier.h"

namespace traktor
{
	namespace scene
	{

class EntityAdapter;

/*! Translation modifier. */
class TranslateModifier : public IModifier
{
	T_RTTI_CLASS;

public:
	TranslateModifier(SceneEditorContext* context);

	/*! \name Notifications */
	//\{

	virtual void selectionChanged() override final;

	virtual bool cursorMoved(
		const TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection
	) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	//\}

	/*! \name Modifications */
	//\{

	virtual bool begin(
		const TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		int32_t mouseButton
	) override final;

	virtual void apply(
		const TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		const Vector4& screenDelta,
		const Vector4& viewDelta
	) override final;

	virtual void end(const TransformChain& transformChain) override final;

	//\}

	/*! \name Preview */
	//\{

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) const override final;

	//\}

private:
	SceneEditorContext* m_context;
	RefArray< EntityAdapter > m_entityAdapters;
	AlignedVector< Vector4 > m_baseTranslations;
	Vector4 m_center0;
	Vector4 m_center;
	uint32_t m_axisEnable;
	uint32_t m_axisHot;

	Vector4 snap(const Vector4& position, uint32_t axisEnable) const;
};

	}
}

