#include "fs.h"

#include "fatfs/ff.h"

static FATFS fs;
static FIL file;

bool InitFS()
{
    *(u32*)0x10000020 = 0;
    *(u32*)0x10000020 = 0x340;
    return f_mount(&fs, "0:", 0) == FR_OK;
}

void DeinitFS()
{
    f_mount(NULL, "0:", 0);
}

bool FileOpen(const char* path)
{
    unsigned flags = FA_READ | FA_WRITE | FA_OPEN_EXISTING;
    bool ret = (f_open(&file, path, flags) == FR_OK);
    f_lseek(&file, 0);
    f_sync(&file);
    return ret;
}

bool FileCreate(const char* path, bool truncate)
{
    unsigned flags = FA_READ | FA_WRITE;
    flags |= truncate ? FA_CREATE_ALWAYS : FA_OPEN_ALWAYS;
    bool ret = (f_open(&file, path, flags) == FR_OK);
    f_lseek(&file, 0);
    f_sync(&file);
    return ret;
}

size_t FileRead(void* buf, size_t size, size_t foffset)
{
    UINT bytes_read = 0;
    f_lseek(&file, foffset);
    f_read(&file, buf, size, &bytes_read);
    return bytes_read;
}

size_t FileWrite(void* buf, size_t size, size_t foffset)
{
    UINT bytes_written = 0;
    f_lseek(&file, foffset);
    f_write(&file, buf, size, &bytes_written);
    f_sync(&file);
    return bytes_written;
}

size_t FileGetSize()
{
    return f_size(&file);
}

void FileClose()
{
    f_close(&file);
}

static uint64_t ClustersToBytes(FATFS* fs, DWORD clusters)
{
    uint64_t sectors = clusters * fs->csize;
#if _MAX_SS != _MIN_SS
    return sectors * fs->ssize;
#else
    return sectors * _MAX_SS;
#endif
}

uint64_t RemainingStorageSpace()
{
    DWORD free_clusters;
    FATFS *fs2;
    FRESULT res = f_getfree("0:", &free_clusters, &fs2);
    if (res)
        return -1;

    return ClustersToBytes(&fs, free_clusters);
}
