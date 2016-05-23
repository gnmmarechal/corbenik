#include <3ds.h>
#include "../patcher.h"
#include "../ifile.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#define _MAX_LFN 255
#endif
#include "../config.h"
#include "../../../../source/patch_format.h"

void
fake_friends_version(u64 progId, u8* code, u32 size)
{
    static const u8 fpdVerPattern[] = { 0xE0, 0x1E, 0xFF, 0x2F,
                                        0xE1, 0x01, 0x01, 0x01 };
    static const u8 fpdVerPatch = 0x06; // Latest version.

    // Allow online access to work with old friends modules
    patchMemory(code, size, fpdVerPattern, sizeof(fpdVerPattern), 9,
                &fpdVerPatch, sizeof(fpdVerPatch), 1);
}
