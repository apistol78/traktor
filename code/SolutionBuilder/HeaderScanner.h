#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Containers/SmallSet.h"

namespace traktor
{
    namespace sb
    {

class HeaderScanner : public Object
{
    T_RTTI_CLASS;

public:
    virtual ~HeaderScanner();

    void removeAllIncludePaths();

    void addIncludePath(const std::wstring& includePath);

    bool get(const std::wstring& fileName, const std::wstring& projectPath, SmallSet< std::wstring >& outHeaderFiles);

private:
    struct Includes
    {
        SmallSet< std::wstring > files;
    };

    SmallSet< std::wstring > m_includePaths;
    SmallMap< std::wstring, Includes* > m_cache;

    /*! Scan header dependencies of source file. */
    const Includes* scan(const std::wstring& fileName);
};

    }
}