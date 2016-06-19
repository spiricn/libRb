/********************************************************/
/*                 Includes                             */
/********************************************************/

#include "Common.h"


/********************************************************/
/*                 Local Module Variables (MODULE)      */
/********************************************************/

static const uint64_t gMajorVersion = RB_VERSION_MAJOR;
static const uint64_t gMinorVersion = RB_VERSION_MINOR;
static const uint64_t gPatchVersion = RB_VERSION_PATCH;

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

uint64_t Rb_getVersion(){
	return RB_VERSION_NUMBER(gMajorVersion, gMinorVersion, gPatchVersion);
}
