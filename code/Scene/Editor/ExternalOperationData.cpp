/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/ExternalOperationData.h"

#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.scene.ExternalOperationData", 0, ExternalOperationData, ISceneOperationData)

void ExternalOperationData::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"externalDataId", m_externalDataId);
}

}
