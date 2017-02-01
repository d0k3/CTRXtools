#pragma once

#include "common.h"

#define ST_REFERENCE    (1<<0)
#define MFDATE (u8*)0x01FFB81A

// --> FEATURE FUNCTIONS <--
u32 SelfTest(u32 param);
u32 SystemInfo(u32 param);
