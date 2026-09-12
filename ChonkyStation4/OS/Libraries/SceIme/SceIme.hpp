#pragma once

#include <Common.hpp>
#include <OS/Libraries/SceUserService/SceUserService.hpp>


class Module;

namespace PS4::OS::Libs::SceIme {

void init(Module& module);

static constexpr s32 SCE_IME_ERROR_BUSY         = 0x80BC0001;
static constexpr s32 SCE_IME_ERROR_NOT_OPENED   = 0x80BC0002;


static constexpr s32 SCE_IME_MAX_TEXT_AREA = 4;

enum SceImeType {
    SCE_IME_TYPE_DEFAULT        = 0,
    SCE_IME_TYPE_BASIC_LATIN    = 1,
    SCE_IME_TYPE_URL            = 2,
    SCE_IME_TYPE_MAIL           = 3,
    SCE_IME_TYPE_NUMBER         = 4,
};

enum SceImeEnterLabel {
    SCE_IME_ENTER_LABEL_DEFAULT = 0,
    SCE_IME_ENTER_LABEL_SEND    = 1,
    SCE_IME_ENTER_LABEL_SEARCH  = 2,
    SCE_IME_ENTER_LABEL_GO      = 3
};

enum SceImeInputMethod {
    SCE_IME_INPUT_METHOD_DEFAULT = 0
};

enum SceImeHorizontalAlignment {
    SCE_IME_HALIGN_LEFT     = 0,
    SCE_IME_HALIGN_CENTER   = 1,
    SCE_IME_HALIGN_RIGHT    = 2
};

enum SceImeVerticalAlignment {
    SCE_IME_VALIGN_TOP    = 0,
    SCE_IME_VALIGN_CENTER = 1,
    SCE_IME_VALIGN_BOTTOM = 2
};


enum SceImeEventId {
    SCE_IME_EVENT_OPEN                      = 0,
    SCE_IME_EVENT_UPDATE_TEXT               = 1,
    SCE_IME_EVENT_UPDATE_CARET              = 2,
    SCE_IME_EVENT_CHANGE_SIZE               = 3,
    SCE_IME_EVENT_PRESS_CLOSE               = 4,
    SCE_IME_EVENT_PRESS_ENTER               = 5,
    SCE_IME_EVENT_ABORT                     = 6,
    SCE_IME_EVENT_CANDIDATE_LIST_START      = 7,
    SCE_IME_EVENT_CANDIDATE_LIST_END        = 8,
    SCE_IME_EVENT_CANDIDATE_WORD            = 9,
    SCE_IME_EVENT_CANDIDATE_INDEX           = 10,
    SCE_IME_EVENT_CANDIDATE_DONE            = 11,
    SCE_IME_EVENT_CANDIDATE_CANCEL          = 12,
    SCE_IME_EVENT_CHANGE_DEVICE             = 14,
    SCE_IME_EVENT_JUMP_TO_NEXT_OBJECT       = 15,
    SCE_IME_EVENT_JUMP_TO_BEFORE_OBJECT     = 16,
    SCE_IME_EVENT_CHANGE_WINDOW_TYPE        = 17,

    SCE_IME_EVENT_CHANGE_INPUT_METHOD_STATE = 18,

    SCE_IME_KEYBOARD_EVENT_OPEN             = 256,
    SCE_IME_KEYBOARD_EVENT_KEYCODE_DOWN     = 257,
    SCE_IME_KEYBOARD_EVENT_KEYCODE_UP       = 258,
    SCE_IME_KEYBOARD_EVENT_KEYCODE_REPEAT   = 259,
    SCE_IME_KEYBOARD_EVENT_CONNECTION       = 260,
    SCE_IME_KEYBOARD_EVENT_DISCONNECTION    = 261,
    SCE_IME_KEYBOARD_EVENT_ABORT            = 262
};

enum SceImeTextAreaMode {
    SCE_IME_TEXT_AREA_MODE_DISABLE  = 0,	
    SCE_IME_TEXT_AREA_MODE_EDIT     = 1,	
    SCE_IME_TEXT_AREA_MODE_PREEDIT  = 2,	
    SCE_IME_TEXT_AREA_MODE_SELECT   = 3
};

struct SceImeTextAreaProperty {
    SceImeTextAreaMode mode;
    u32 index;
    s32 length;
};

struct SceImeEditText {
    wchar_t* str;
    u32 caret_idx;
    u32 n_areas;
    SceImeTextAreaProperty text_areas[SCE_IME_MAX_TEXT_AREA];
};

union SceImeEventParam {
    SceImeEditText text;
    // TODO: Other events
    s8 reserved[64];
};

struct SceImeEvent {
    SceImeEventId id;
    SceImeEventParam param;
};

using SceImeTextFilter   = PS4_FUNC s32(*)(wchar_t* out_text, u32* out_text_len, const wchar_t* src_text, u32 src_text_len);
using SceImeEventHandler = PS4_FUNC void (*)(void* arg, const SceImeEvent* e);

struct SceImeParam {
    SceUserService::SceUserServiceUserId userId;
    SceImeType type;
    u64 supported_languages;
    SceImeEnterLabel enter_label;
    SceImeInputMethod input_method;
    SceImeTextFilter filter;
    u32 option;
    u32 max_text_len;
    wchar_t* input_text_buffer;
    float pos_x;
    float pos_y;
    SceImeHorizontalAlignment horizontal_align;
    SceImeVerticalAlignment vertical_align;
    void* work;
    void* arg;
    SceImeEventHandler handler;
    s8 reserved[8];
};

struct SceImeParamExtended;

s32 PS4_FUNC sceImeOpen(const SceImeParam* param, const SceImeParamExtended* extended_param);
s32 PS4_FUNC sceImeUpdate(SceImeEventHandler handler);
s32 PS4_FUNC sceImeClose();

}   // End namespace PS4::OS::Libs::SceIme