#ifndef _GNM_ERROR_H_
#define _GNM_ERROR_H_

typedef enum {
	// errors used by original GNM and GnmDriver
	GNM_ERROR_OK = 0x0,
	GNM_ERROR_SUBMISSION_FAILED_INVALID_ARGUMENT = (int)0x80d11000,
	GNM_ERROR_SUBMISSION_NOT_ENOUGH_RESOURCES = (int)0x80d11001,
	GNM_ERROR_SUBMISSION_AND_FLIP_FAILED_INVALID_COMMAND_BUFFER =
	    (int)0x80d11080,
	GNM_ERROR_SUBMISSION_AND_FLIP_FAILED_INVALID_QUEUE_FULL =
	    (int)0x80d11081,
	GNM_ERROR_SUBMISSION_AND_FLIP_FAILED_REQUEST_FAILED = (int)0x80d11082,
	GNM_ERROR_SUBMISSION_FAILED_INTERNAL_ERROR = (int)0x80d110ff,

	GNM_ERROR_INVALID_ARGS = (int)0x8eee0001,
	GNM_ERROR_INTERNAL_FAILURE = (int)0x8eee00ff,

	GNM_ERROR_CMD_FAILED = (int)0xffffffff,

	// custom libgnm errors
	GNM_ERROR_INVALID_ALIGNMENT = 1,
	GNM_ERROR_INVALID_STATE,
	GNM_ERROR_OVERFLOW,
	GNM_ERROR_UNDERFLOW,
	GNM_ERROR_UNSUPPORTED,
	GNM_ERROR_SEMANTIC_NOT_FOUND,
	GNM_ERROR_ASM_FAILED,
} GnmError;

const char* gnmStrError(GnmError err);

typedef enum {
	GNM_MSGSEV_WARN = 0,
	GNM_MSGSEV_ERR = 1,
} GnmMessageSeverity;

typedef void (*GnmMessageHandlerFunc)(
    GnmMessageSeverity severity, const char* message, void* userdata
);

void gnmSetMessageHandler(GnmMessageHandlerFunc handlerfunc, void* userdata);
void gnmWriteMsg(GnmMessageSeverity sev, const char* msg);
void gnmWriteMsgf(GnmMessageSeverity sev, const char* fmt, ...);

#endif	// _GNM_ERROR_H_
