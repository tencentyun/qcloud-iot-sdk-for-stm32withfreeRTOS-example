#define VERSION_MAJOR           1
#define VERSION_MINOR           0
#define VERSION_REVISION        0
#define NUM3STR(a,b,c)          #a "." #b "." #c
#define VERSIONBUILDSTR(a,b,c)  NUM3STR(a,b,c)
#define FW_VERSION_STR          VERSIONBUILDSTR(VERSION_MAJOR,VERSION_MINOR,VERSION_REVISION)

#define BUILDING_DATE           __DATE__
#define BUILDING_TIME           __TIME__

