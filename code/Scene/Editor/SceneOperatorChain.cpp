/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/SceneOperatorChain.h"

#include "Core/Log/Log.h"
#include "Scene/Editor/ISceneOperationData.h"
#include "Scene/Editor/ISceneOperator.h"
#include "Scene/Editor/SceneAsset.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.SceneOperatorChain", SceneOperatorChain, Object)

bool SceneOperatorChain::create(const editor::IPipelineSettings* settings)
{
	// Instantiate only geometric operators; non-geometric operators (e.g. bake)
	// are a build-time concern and must not run in the interactive editor.
	for (const auto operatorType : type_of< ISceneOperator >().findAllOf(false))
	{
		Ref< ISceneOperator > spo = dynamic_type_cast< ISceneOperator* >(operatorType->createInstance());
		if (!spo)
			continue;
		if (!spo->isGeometricTransform())
			continue;
		if (!spo->create(settings))
		{
			log::error << L"Unable to create scene transform chain; operator \"" << operatorType->getName() << L"\" failed to create." << Endl;
			return false;
		}
		m_operators.push_back(spo);
	}
	return true;
}

void SceneOperatorChain::destroy()
{
	for (auto op : m_operators)
		op->destroy();
	m_operators.clear();
}

bool SceneOperatorChain::apply(SceneAsset* inoutSceneAsset, const ISceneOperator::TransformContext& context) const
{
	for (const auto op : inoutSceneAsset->getOperationData())
	{
		const ISceneOperator* spo = findOperator(type_of(op));
		if (!spo)
			continue;

		if (!spo->transform(context, op, inoutSceneAsset))
		{
			log::error << L"Scene transform chain; operator transform failed." << Endl;
			return false;
		}
	}
	return true;
}

const ISceneOperator* SceneOperatorChain::findOperator(const TypeInfo& operationType) const
{
	for (const auto sop : m_operators)
	{
		for (const auto ts : sop->getOperatorTypes())
		{
			if (is_type_a(*ts, operationType))
				return sop;
		}
	}
	return nullptr;
}

}
