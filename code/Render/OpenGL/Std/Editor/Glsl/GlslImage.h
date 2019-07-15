#pragma once

#include "Render/OpenGL/Std/Editor/Glsl/GlslResource.h"
#include "Render/OpenGL/Std/Editor/Glsl/GlslType.h"

namespace traktor
{
	namespace render
	{
	
class GlslImage : public GlslResource
{
	T_RTTI_CLASS;

public:
	GlslImage(const std::wstring& name);
};

	}
}