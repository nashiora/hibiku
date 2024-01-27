#ifndef HIBIKU_H
#define HIBIKU_H

#include <stdint.h>

/// Fancy C macro trickery to get strings when we need them.
/// If you don't know what this means, don't worry too much about it.
#define HBK_STR_HELPER(x) #x
#define HBK_STR(x) HBK_STR_HELPER(x)

/// Hibiku follows a simplified version of Semantic Versioning (https://semver.org/).
/// This library provides the Major, Minor and Patch version numbers as macros.
/// Also provided are macros to convert a version to a single integer for the purposes of comparison,
/// as well as a concatenated version string.

/// For the purposes of explaining these version numbers:
/// -  X refers to the Major version,
/// -  Y refers to the Minor version, and
/// -  Z refers to the Patch version

#define HBK_VERSION_MAJOR 1
#define HBK_VERSION_MINOR 0
#define HBK_VERSION_PATCH 0

/// The version number is the Major and Minor version numbers in the form XYY.
/// This means that the Minor version takes the lower two digits, and the Major
/// fills out the higest.
#define HBK_VERSION_NUMBER ((HBK_VERSION_MAJOR * 100) + HBK_VERSION_MINOR)
/// The release version number includes the Patch version by multiplying
/// the base version by 100 so that the Patch version number can take the lowest two
/// digits. This version number takes the form XYYZZ.
#define HBK_VERSION_RELEASE_NUMBER ((HBK_VERSION_NUMBER * 100) + HBK_VERSION_PATCH)
/// The Hibiku version as a string, including the name "Hibiku" at the start.
/// This string is in the form of "Hibiku X.Y"
#define HBK_VERSION "Hibiku " HBK_STR(HBK_VERSION_MAJOR) "." HBK_STR(HBK_VERSION_MINOR)
/// The Hibiku release version as a string, including the name "Hibiku" at the start.
/// This includes the Patch version, so this string is in the form of "Hibiku X.Y.Z".
#define HBK_VERSION_RELEASE HBK_VERSION "." HBK_STR(HBK_VERSION_PATCH)

typedef int64_t hbk_source_id;
typedef struct hbk_state hbk_state;

hbk_state* hbk_state_create(void);
hbk_source_id hbk_state_add_source_from_file(hbk_state* state, const char* file_path);
const char* hbk_state_get_source_name(hbk_state* state, hbk_source_id source_id);
const char* hbk_state_get_source_text(hbk_state* state, hbk_source_id source_id);

#endif // !HIBIKU_H
