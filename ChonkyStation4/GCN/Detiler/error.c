#include "error.h"

const char* gpaStrError(const GpaError err) {
	switch (err) {
	case GPA_ERR_OK:
		return "No error";
	case GPA_ERR_INVALID_ARGS:
		return "An invalid argument was used";
	case GPA_ERR_OVERFLOW:
		return "A buffer has overflown";
	case GPA_ERR_TILING_ERROR:
		return "An internal tiling error occured";
	case GPA_ERR_UNSUPPORTED:
		return "A requested feature is unsupported";
	case GPA_ERR_INTERNAL_ERROR:
		return "An internal error occured";
	case GPA_ERR_NOT_COMPRESSED:
		return "The texture is not compressed";
	default:
		return 0;
	}
}
