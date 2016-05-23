#include <3ds.h>
#include "ifile.h"
#include "fsldr.h"

// TODO - What exactly is the point of these? Can't we just use FSFILE directly?
//        These are just shitty wrappers.

Result
IFile_Open(IFile* file, FS_ArchiveID archiveId, FS_Path archivePath,
           FS_Path filePath, u32 flags)
{
    Result res;

    res = FSLDR_OpenFileDirectly(&file->handle, archiveId, archivePath,
                                 filePath, flags, 0);
    file->pos = 0;
    file->size = 0;
    return res;
}

Result
IFile_Close(IFile* file)
{
    return FSFILE_Close(file->handle);
}

Result
IFile_GetSize(IFile* file, u64* size)
{
    Result res;

    res = FSFILE_GetSize(file->handle, size);
    file->size = *size;
    return res;
}

Result
IFile_Read(IFile* file, u64* total, void* buffer, u32 len)
{
    u32 read;
    u32 left;
    char* buf;
    u64 cur;
    Result res;

    if (len == 0) {
        *total = 0;
        return 0;
    }

    buf = (char*)buffer;
    cur = 0;
    left = len;
    while (1) {
        res = FSFILE_Read(file->handle, &read, file->pos, buf, left);
        if (R_FAILED(res)) {
            break;
        }

        cur += read;
        file->pos += read;
        if (read == left) {
            break;
        }
        buf += read;
        left -= read;
    }

    *total = cur;
    return res;
}

Result
IFile_Write(IFile* file, u64* total, void* buffer, u32 len)
{
    return 1; // FIXME - Not yet implemented.
}
