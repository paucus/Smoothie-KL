/* mbed Microcontroller Library - FATFileSystem
 * Copyright (c) 2008, sford
 */

#include "FATFileSystem.h"

#include "mbed.h"

#include "FileSystemLike.h"
#include "FATFileHandle.h"
#include "FATDirHandle.h"
#include "ff.h"
//#include "Debug.h"
#include <stdio.h>
#include <stdlib.h>

DWORD get_fattime (void) {
    return 999;
}

namespace mbed {

#if FFSDEBUG_ENABLED
static const char *FR_ERRORS[] = {
    "FR_OK = 0",
    "FR_NOT_READY",
    "FR_NO_FILE",
    "FR_NO_PATH",
    "FR_INVALID_NAME",
    "FR_INVALID_DRIVE",
    "FR_DENIED",
    "FR_EXIST",
    "FR_RW_ERROR",
    "FR_WRITE_PROTECTED",
    "FR_NOT_ENABLED",
    "FR_NO_FILESYSTEM",
    "FR_INVALID_OBJECT",
    "FR_MKFS_ABORTED"
};
#endif

FATFileSystem *FATFileSystem::_ffs[_DRIVES] = {0};

FATFileSystem::FATFileSystem(const char* n) : FileSystemLike(n) {
    FFSDEBUG("FATFileSystem(%s)\n", n);
    for(int i=0; i<_DRIVES; i++) {
        if(_ffs[i] == 0) {
            _ffs[i] = this;
            _fsid = i;
            FFSDEBUG("Mounting [%s] on ffs drive [%d]\n", _name, _fsid);
            f_mount(i, &_fs);
            return;
        }
    }
    error("Couldn't create %s in FATFileSystem::FATFileSystem\n",n);
}

FATFileSystem::~FATFileSystem() {
    for(int i=0; i<_DRIVES; i++) {
        if(_ffs[i] == this) {
            _ffs[i] = 0;
            f_mount(i, NULL);
        }
    }
}

FileHandle *FATFileSystem::open(const char* name, int flags) {
    FFSDEBUG("open(%s) on filesystem [%s], drv [%d]\n", name, _name, _fsid);
    // BEGIN MODIF fat
    char n[PATH_BUFF_SIZE];
    // END MODIF fat
    sprintf(n, "%d:/%s", _fsid, name);

    /* POSIX flags -> FatFS open mode */
    BYTE openmode;
    if(flags & O_RDWR) {
        openmode = FA_READ|FA_WRITE;
    } else if(flags & O_WRONLY) {
        openmode = FA_WRITE;
    } else {
        openmode = FA_READ;
    }
    if(flags & O_CREAT) {
        if(flags & O_TRUNC) {
            openmode |= FA_CREATE_ALWAYS;
        } else {
            openmode |= FA_OPEN_ALWAYS;
        }
    }

    // BEGIN MODIF avoid_copying_FIL_t
    // The former implementation made an instantiation of FIL_t, and then it got copied 2 times
    // (one for the FATFileHandle constructor, and one for the attribute itself). We ran out of
    // stack memory, so I changed it so that we use only the attribute (the only instance that
    // can't be removed).
    FATFileHandle* ffh = new FATFileHandle();
    FIL_t& fh = ffh->_fh;
    // END MODIF avoid_copying_FIL_t
    FRESULT res = f_open(&fh, n, openmode);
    if(res) {
        FFSDEBUG("f_open('w') failed (%d, %s)\n", res, FR_ERRORS[res]);
        delete ffh;
        return NULL;
    }
    if(flags & O_APPEND) {
        f_lseek(&fh, fh.fsize);
    }
    // BEGIN MODIF avoid_copying_FIL_t
    //return new FATFileHandle(fh);
    return ffh;
    // END MODIF avoid_copying_FIL_t
}

int FATFileSystem::remove(const char *filename) {
    // BEGIN MODIF external sdcard
    FFSDEBUG("remove(%s) on filesystem [%s], drv [%d]\n", filename, _name, _fsid);
    char n[PATH_BUFF_SIZE];
    // adapt name to internal structure (drive number:/path/to/dir)
    sprintf(n, "%d:/%s", _fsid, filename);
    
    FRESULT res = f_unlink(n);
    // END MODIF external sdcard
    if(res) {
        FFSDEBUG("f_unlink() failed (%d, %s)\n", res, FR_ERRORS[res]);
        return -1;
    }
    return 0;
}

int FATFileSystem::rename(const char *filename1, const char *filename2) {
    // BEGIN MODIF external sdcard
    FFSDEBUG("rename(%s) on filesystem [%s], drv [%d]\n", filename, _name, _fsid);
    char n[PATH_BUFF_SIZE];
    // adapt name to internal structure (drive number:/path/to/dir)
    sprintf(n, "%d:/%s", _fsid, filename1);
    FRESULT res = f_rename(n, filename2);
    // END MODIF external sdcard
    if(res) {
        FFSDEBUG("f_rename() failed (%d, %s)\n", res, FR_ERRORS[res]);
        return -1;
    }
    return 0;
}

int FATFileSystem::format() {
    FFSDEBUG("format()\n");
    FRESULT res = f_mkfs(_fsid, 0, 512); // Logical drive number, Partitioning rule, Allocation unit size (bytes per cluster)
    if(res) {
        FFSDEBUG("f_mkfs() failed (%d, %s)\n", res, FR_ERRORS[res]);
        return -1;
    }
    return 0;
}

DirHandle *FATFileSystem::opendir(const char *name) {
    // BEGIN MODIF external sdcard
    FFSDEBUG("opendir(%s) on filesystem [%s], drv [%d]\n", name, _name, _fsid);
    char n[PATH_BUFF_SIZE];
    // adapt name to internal structure (drive number:/path/to/dir)
    sprintf(n, "%d:/%s", _fsid, name);
    
    DIR_t dir;
    FRESULT res = f_opendir(&dir, n);
    // END MODIF external sdcard
    if(res != 0) {
        return NULL;
    }
    return new FATDirHandle(dir);
}

int FATFileSystem::mkdir(const char *name, mode_t mode) {
    // BEGIN MODIF external sdcard
    FFSDEBUG("mkdir(%s) on filesystem [%s], drv [%d]\n", name, _name, _fsid);
    char n[PATH_BUFF_SIZE];
    // adapt name to internal structure (drive number:/path/to/dir)
    sprintf(n, "%d:/%s", _fsid, name);

    FRESULT res = f_mkdir(n);
    // END MODIF external sdcard
    return res == 0 ? 0 : -1;
}

// BEGIN MODIF fat
static struct tm fat_datetime_to_tm(WORD fdate, WORD ftime) {
    WORD day_of_month = 0x001F & fdate;
    WORD month_of_year = (0x01E0 & fdate) >> 5;
    // FAT stores years since 1980.
    WORD year_since_1980 = ((0xFE00 & fdate) >> 9) + 1980;
    
    WORD hour = (0xF800 & ftime) >> 11;
    WORD minutes = (0x07E0 & ftime) >> 5;
    WORD seconds = (0x001F & ftime) * 2;
    
    struct tm time_data;
    time_data.tm_sec = seconds;
    time_data.tm_min = minutes;
    time_data.tm_hour = hour;
    time_data.tm_mday = day_of_month;
    time_data.tm_mon = month_of_year;
    // Struct tm stores years since 1900.
    time_data.tm_year = year_since_1980 - 1900;
    time_data.tm_isdst = -1;
    return time_data;
}
static time_t fat_datetime_to_time_t(WORD fdate, WORD ftime) {
    struct tm time_data = fat_datetime_to_tm(fdate, ftime);
    time_t tt = mktime(&time_data);
    return tt;
}
static void filinfo_to_stat(FILINFO* fi, struct stat *st) {
    // WARNING Only mtime will be populated. Many of the other attributes make no sense in this case.
    st->st_mtime = fat_datetime_to_time_t(fi->fdate, fi->ftime);
    st->st_size = fi->fsize;
    st->st_mode = (fi->fattrib & 0x10)?S_IFDIR:S_IFREG;	// only directories and regular files
    /*st->st_dev = _fsid; // FIXME not sure about this
    st->st_ino = -1;
    //st->st_mode = ?;
    st->st_nlink = 1;
    st->st_atime = st->st_mtime;
    st->st_ctime = st->st_mtime;*/
    
}
int FATFileSystem::stat(const char *name, struct stat *st) {
    FFSDEBUG("stat(%s) on filesystem [%s], drv [%d]\n", name, _name, _fsid);
    char n[PATH_BUFF_SIZE];
    // adapt name to internal structure (drive number:/path/to/dir)
    sprintf(n, "%d:/%s", _fsid, name);

    char aux[PATH_BUFF_SIZE];
    FILINFO fi;
    fi.lfname = aux;
    fi.lfsize = strlen(n);
    int result = f_stat(n, &fi);
    if (result != FR_OK) {
        return -1;
    }
    
    filinfo_to_stat(&fi, st);
    return 0;
}
// END MODIF fat

} // namespace mbed
