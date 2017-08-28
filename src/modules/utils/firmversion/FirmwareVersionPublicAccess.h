#ifndef FIRMWARE_VERSION_PUBLIC_ACCESS_H
#define FIRMWARE_VERSION_PUBLIC_ACCESS_H

#include "checksumm.h"

#define firmware_version_checksum      CHECKSUM("firmware_version")
#define number_checksum                CHECKSUM("number")
#define commit_checksum                CHECKSUM("commit")

#endif // FIRMWARE_VERSION_PUBLIC_ACCESS_H
