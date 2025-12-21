#ifndef _GNM_PLATFORM_H_
#define _GNM_PLATFORM_H_

#include "types.h"

GnmGpuMode gnmGpuMode(void);

typedef struct {
	GnmGpuMode gpumode;
	int32_t (*getbufferlabeladdress)(
	    int32_t videohandle, uint64_t* outaddr
	);
} GnmPlatParams;

void gnmPlatInit(GnmPlatParams* params);
int32_t gnmPlatGetBufferLabelAddress(int32_t videohandle, uint64_t* outaddr);

#endif	// _GNM_PLATFORM_H_
