#include <stddef.h>
#include <ctr9/io.h>
#include <common.h>

// ctr_nand_crypto_interface ctr_io;
// ctr_nand_crypto_interface twl_io;
static ctr_sd_interface   sd_io;
static ctr_nand_interface nand_io;
static FATFS fs;
static int set_up_fs = 0;

// This function is based on PathDeleteWorker from GodMode9.
// It was easier to just import it.
int
recurse_call_back(char *fpath, void (*call_fun_param)(char*))
{
    FILINFO fno;
    DIR pdir;
    char *fname = &fpath[strnlen(fpath, 255)];
    if (f_opendir(&pdir, fpath) != FR_OK)
        return 1;

    fname[0] = '/';
    fname++;

    while (f_readdir(&pdir, &fno) == FR_OK) {
        strcpy(fname, fno.fname);

        if (fno.fname[0] == 0)
            break;

        FILINFO f2;
        if (f_stat(fpath, &f2) != FR_OK)
            break;

        if (f2.fattrib & AM_DIR) {
            // return value won't matter
            recurse_call_back(fpath, call_fun_param);
        } else {
            call_fun_param(fpath);
        }
    }

    f_closedir(&pdir);
    --fname;
    fname[0] = 0;

    call_fun_param(fpath);

    return 0;
}

void recurse_call(const char *name, void (*call_fun_param)(char*)) {
    static char fpath[256];
    strncpy(fpath, name, 256);
    recurse_call_back(fpath, call_fun_param);
}

int
rrmdir(const char *name)
{
    recurse_call(name, (void (*)(char*))f_unlink);
    return 0;
}

int
crmount(void)
{
    if(!set_up_fs && ctr_fatfs_initialize(&nand_io, NULL, NULL, &sd_io))
        return 1;

    set_up_fs = 1;

    if (f_mount(&fs, "SD:", 1))
        return 1;

    if (f_chdrive("SD:"))
        return 1;

    return 0;
}

int
crumount(void)
{
    if (f_mount(NULL, "SD:", 1))
        return 1;

    set_opt_u32(OPTION_SAVE_LOGS, 0); // FS unmounted, can't log anymore

    return 0;
}

FILE *
cropen(const char *filename, const char *mode)
{
    if (mode[0] != 'r' && mode[0] != 'w' && mode[0] != 'a')
        return NULL; // Mode not valid.

    FILE *fp = (FILE*)malloc(sizeof(FILE));

    fp->mode = (mode[0] == 'r' ? FA_READ : (FA_WRITE | FA_OPEN_ALWAYS));

    if (f_open(&(fp->handle), filename, fp->mode)) {
        free(fp);
        return NULL;
    }

    fp->is_open = 1;

    return fp;
}

void
crclose(FILE *fp)
{
    if (fp == NULL || !fp->is_open)
        return;

    f_close(&(fp->handle));

    memset(fp, 0, sizeof(FILE));

    free(fp);
}

void
crseek(FILE *fp, int64_t offset, int whence)
{
    if (fp == NULL || !fp->is_open)
        return;

    uint32_t fixed_offset;
    switch (whence) {
        case SEEK_SET:
            fixed_offset = 0;
            break;
        case SEEK_CUR:
            fixed_offset = crtell(fp);
            break;
        case SEEK_END:
            fixed_offset = crsize(fp);
            break;
        default:
            return;
    }

    f_lseek(&(fp->handle), fixed_offset + offset);
}

size_t
crtell(FILE *fp)
{
    if (fp == NULL || !fp->is_open)
        return 0;

    return f_tell(&(fp->handle));
}

int
creof(FILE *fp)
{
    if (fp == NULL || !fp->is_open)
        return 0;

    return f_eof(&(fp->handle));
}

size_t
crsize(FILE *fp)
{
    if (fp == NULL || !fp->is_open)
        return 0;

    return f_size(&(fp->handle));
}

size_t
crwrite(const void *buffer, size_t elementSize, size_t elementCnt, FILE *fp)
{
    if (fp == NULL || !fp->is_open)
        return 0;

    UINT br;
    if (f_write(&(fp->handle), buffer, elementSize * elementCnt, &br))
        return 0;
    if (elementSize != 1)
        br /= elementSize;
    return br;
}

size_t
crread(void *buffer, size_t elementSize, size_t elementCnt, FILE *fp)
{
    if (!fp->is_open)
        return 0;

    size_t br;
    if (f_read(&(fp->handle), buffer, elementSize * elementCnt, &br))
        return 0;

    if (elementSize != 1)
        br /= elementSize;
    return br;
}

size_t
write_file(void *data, const char *path, size_t size)
{
    FILE *temp = cropen(path, "w");

    if (!temp)
        return 0;

    if (!temp->is_open) {
        crclose(temp);
        return 0;
    }

    size_t wrote = crwrite(data, 1, size, temp);

    crclose(temp);

    return wrote;
}

size_t
read_file(void *data, const char *path, size_t size)
{
    FILE *temp = cropen(path, "r");

    if (!temp)
        return 0;

    if (!temp->is_open) {
        crclose(temp);
        return 0;
    }

    size_t read = crread(data, 1, size, temp);

    crclose(temp);

    return read;
}
