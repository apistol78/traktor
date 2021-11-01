#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Io/IVolume.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_COMPRESS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

    namespace compress
    {

class T_DLLCLASS ZipVolume : public IVolume
{
    T_RTTI_CLASS;

public:
    explicit ZipVolume(IStream* zipFile);

    virtual std::wstring getDescription() const override final;

    virtual Ref< File > get(const Path& path) override final;

    virtual int find(const Path& mask, RefArray< File >& out) override final;

    virtual bool modify(const Path& fileName, uint32_t flags) override final;

    virtual bool modify(const Path& fileName, const DateTime* creationTime, const DateTime* lastAccessTime, const DateTime* lastWriteTime) override final;

    virtual Ref< IStream > open(const Path& fileName, uint32_t mode) override final;

    virtual bool exist(const Path& fileName) override final;

    virtual bool remove(const Path& fileName) override final;

    virtual bool move(const Path& fileName, const std::wstring& newName, bool overwrite) override final;

    virtual bool copy(const Path& fileName, const std::wstring& newName, bool overwrite) override final;

    virtual bool makeDirectory(const Path& directory) override final;

    virtual bool removeDirectory(const Path& directory) override final;

    virtual bool renameDirectory(const Path& directory, const std::wstring& newName) override final;

    virtual bool setCurrentDirectory(const Path& directory) override final;

    virtual Path getCurrentDirectory() const override final;

private:
    struct FileInfo
    {
        std::wstring name;
        int32_t parent = -1;
        AlignedVector< int32_t > children;
        uint64_t offset = 0;
        uint64_t compressedSize = 0;
        uint64_t uncompressedSize = 0;
        uint16_t attributes = 0;

        bool isDirectory() const { return (bool)(compressedSize == 0); }
    };

    Ref< IStream > m_zipFile;
    AlignedVector< FileInfo > m_fileInfo;

    Path m_currentDirectory;

    std::wstring getSystemPath(const Path& path) const;

    std::wstring getPathName(int32_t index) const;

    int32_t findFileInfoIndex(const Path& path) const;

    void dump(int32_t index) const;
};

    }
}