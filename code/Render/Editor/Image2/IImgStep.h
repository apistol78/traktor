#pragma once

#include <set>
#include <string>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace render
    {

class T_DLLCLASS IImgStep : public ISerializable
{
    T_RTTI_CLASS;

public:
    virtual std::wstring getTitle() const = 0;

    virtual void getInputs(std::set< std::wstring >& outInputs) const = 0;
};

    }
}
