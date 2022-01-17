
#include "luat_base.h"
#ifndef LUAT_LV_ENUM
#define LUAT_LV_ENUM
#include "rotable.h"
#define LUAT_LV_ENMU_RLT {"T", NULL, 0xFF},\
    {"RES_INV", NULL, LV_RES_INV},\
    {"RES_OK", NULL, LV_RES_OK},\
    {"OPA_TRANSP", NULL, LV_OPA_TRANSP},\
    {"OPA_0", NULL, LV_OPA_0},\
    {"OPA_10", NULL, LV_OPA_10},\
    {"OPA_20", NULL, LV_OPA_20},\
    {"OPA_30", NULL, LV_OPA_30},\
    {"OPA_40", NULL, LV_OPA_40},\
    {"OPA_50", NULL, LV_OPA_50},\
    {"OPA_60", NULL, LV_OPA_60},\
    {"OPA_70", NULL, LV_OPA_70},\
    {"OPA_80", NULL, LV_OPA_80},\
    {"OPA_90", NULL, LV_OPA_90},\
    {"OPA_100", NULL, LV_OPA_100},\
    {"OPA_COVER", NULL, LV_OPA_COVER},\
    {"PALETTE_RED", NULL, LV_PALETTE_RED},\
    {"PALETTE_PINK", NULL, LV_PALETTE_PINK},\
    {"PALETTE_PURPLE", NULL, LV_PALETTE_PURPLE},\
    {"PALETTE_DEEP_PURPLE", NULL, LV_PALETTE_DEEP_PURPLE},\
    {"PALETTE_INDIGO", NULL, LV_PALETTE_INDIGO},\
    {"PALETTE_BLUE", NULL, LV_PALETTE_BLUE},\
    {"PALETTE_LIGHT_BLUE", NULL, LV_PALETTE_LIGHT_BLUE},\
    {"PALETTE_CYAN", NULL, LV_PALETTE_CYAN},\
    {"PALETTE_TEAL", NULL, LV_PALETTE_TEAL},\
    {"PALETTE_GREEN", NULL, LV_PALETTE_GREEN},\
    {"PALETTE_LIGHT_GREEN", NULL, LV_PALETTE_LIGHT_GREEN},\
    {"PALETTE_LIME", NULL, LV_PALETTE_LIME},\
    {"PALETTE_YELLOW", NULL, LV_PALETTE_YELLOW},\
    {"PALETTE_AMBER", NULL, LV_PALETTE_AMBER},\
    {"PALETTE_ORANGE", NULL, LV_PALETTE_ORANGE},\
    {"PALETTE_DEEP_ORANGE", NULL, LV_PALETTE_DEEP_ORANGE},\
    {"PALETTE_BROWN", NULL, LV_PALETTE_BROWN},\
    {"PALETTE_BLUE_GREY", NULL, LV_PALETTE_BLUE_GREY},\
    {"PALETTE_GREY", NULL, LV_PALETTE_GREY},\
    {"PALETTE_NONE", NULL, LV_PALETTE_NONE},\
    {"ALIGN_DEFAULT", NULL, LV_ALIGN_DEFAULT},\
    {"ALIGN_TOP_LEFT", NULL, LV_ALIGN_TOP_LEFT},\
    {"ALIGN_TOP_MID", NULL, LV_ALIGN_TOP_MID},\
    {"ALIGN_TOP_RIGHT", NULL, LV_ALIGN_TOP_RIGHT},\
    {"ALIGN_BOTTOM_LEFT", NULL, LV_ALIGN_BOTTOM_LEFT},\
    {"ALIGN_BOTTOM_MID", NULL, LV_ALIGN_BOTTOM_MID},\
    {"ALIGN_BOTTOM_RIGHT", NULL, LV_ALIGN_BOTTOM_RIGHT},\
    {"ALIGN_LEFT_MID", NULL, LV_ALIGN_LEFT_MID},\
    {"ALIGN_RIGHT_MID", NULL, LV_ALIGN_RIGHT_MID},\
    {"ALIGN_CENTER", NULL, LV_ALIGN_CENTER},\
    {"ALIGN_OUT_TOP_LEFT", NULL, LV_ALIGN_OUT_TOP_LEFT},\
    {"ALIGN_OUT_TOP_MID", NULL, LV_ALIGN_OUT_TOP_MID},\
    {"ALIGN_OUT_TOP_RIGHT", NULL, LV_ALIGN_OUT_TOP_RIGHT},\
    {"ALIGN_OUT_BOTTOM_LEFT", NULL, LV_ALIGN_OUT_BOTTOM_LEFT},\
    {"ALIGN_OUT_BOTTOM_MID", NULL, LV_ALIGN_OUT_BOTTOM_MID},\
    {"ALIGN_OUT_BOTTOM_RIGHT", NULL, LV_ALIGN_OUT_BOTTOM_RIGHT},\
    {"ALIGN_OUT_LEFT_TOP", NULL, LV_ALIGN_OUT_LEFT_TOP},\
    {"ALIGN_OUT_LEFT_MID", NULL, LV_ALIGN_OUT_LEFT_MID},\
    {"ALIGN_OUT_LEFT_BOTTOM", NULL, LV_ALIGN_OUT_LEFT_BOTTOM},\
    {"ALIGN_OUT_RIGHT_TOP", NULL, LV_ALIGN_OUT_RIGHT_TOP},\
    {"ALIGN_OUT_RIGHT_MID", NULL, LV_ALIGN_OUT_RIGHT_MID},\
    {"ALIGN_OUT_RIGHT_BOTTOM", NULL, LV_ALIGN_OUT_RIGHT_BOTTOM},\
    {"DIR_NONE", NULL, LV_DIR_NONE},\
    {"DIR_LEFT", NULL, LV_DIR_LEFT},\
    {"DIR_RIGHT", NULL, LV_DIR_RIGHT},\
    {"DIR_TOP", NULL, LV_DIR_TOP},\
    {"DIR_BOTTOM", NULL, LV_DIR_BOTTOM},\
    {"DIR_HOR", NULL, LV_DIR_HOR},\
    {"DIR_VER", NULL, LV_DIR_VER},\
    {"DIR_ALL", NULL, LV_DIR_ALL},\
    {"IMG_CF_UNKNOWN", NULL, LV_IMG_CF_UNKNOWN},\
    {"IMG_CF_RAW", NULL, LV_IMG_CF_RAW},\
    {"IMG_CF_RAW_ALPHA", NULL, LV_IMG_CF_RAW_ALPHA},\
    {"IMG_CF_RAW_CHROMA_KEYED", NULL, LV_IMG_CF_RAW_CHROMA_KEYED},\
    {"IMG_CF_TRUE_COLOR", NULL, LV_IMG_CF_TRUE_COLOR},\
    {"IMG_CF_TRUE_COLOR_ALPHA", NULL, LV_IMG_CF_TRUE_COLOR_ALPHA},\
    {"IMG_CF_TRUE_COLOR_CHROMA_KEYED", NULL, LV_IMG_CF_TRUE_COLOR_CHROMA_KEYED},\
    {"IMG_CF_INDEXED_1BIT", NULL, LV_IMG_CF_INDEXED_1BIT},\
    {"IMG_CF_INDEXED_2BIT", NULL, LV_IMG_CF_INDEXED_2BIT},\
    {"IMG_CF_INDEXED_4BIT", NULL, LV_IMG_CF_INDEXED_4BIT},\
    {"IMG_CF_INDEXED_8BIT", NULL, LV_IMG_CF_INDEXED_8BIT},\
    {"IMG_CF_ALPHA_1BIT", NULL, LV_IMG_CF_ALPHA_1BIT},\
    {"IMG_CF_ALPHA_2BIT", NULL, LV_IMG_CF_ALPHA_2BIT},\
    {"IMG_CF_ALPHA_4BIT", NULL, LV_IMG_CF_ALPHA_4BIT},\
    {"IMG_CF_ALPHA_8BIT", NULL, LV_IMG_CF_ALPHA_8BIT},\
    {"IMG_CF_RESERVED_15", NULL, LV_IMG_CF_RESERVED_15},\
    {"IMG_CF_RESERVED_16", NULL, LV_IMG_CF_RESERVED_16},\
    {"IMG_CF_RESERVED_17", NULL, LV_IMG_CF_RESERVED_17},\
    {"IMG_CF_RESERVED_18", NULL, LV_IMG_CF_RESERVED_18},\
    {"IMG_CF_RESERVED_19", NULL, LV_IMG_CF_RESERVED_19},\
    {"IMG_CF_RESERVED_20", NULL, LV_IMG_CF_RESERVED_20},\
    {"IMG_CF_RESERVED_21", NULL, LV_IMG_CF_RESERVED_21},\
    {"IMG_CF_RESERVED_22", NULL, LV_IMG_CF_RESERVED_22},\
    {"IMG_CF_RESERVED_23", NULL, LV_IMG_CF_RESERVED_23},\
    {"IMG_CF_USER_ENCODED_0", NULL, LV_IMG_CF_USER_ENCODED_0},\
    {"IMG_CF_USER_ENCODED_1", NULL, LV_IMG_CF_USER_ENCODED_1},\
    {"IMG_CF_USER_ENCODED_2", NULL, LV_IMG_CF_USER_ENCODED_2},\
    {"IMG_CF_USER_ENCODED_3", NULL, LV_IMG_CF_USER_ENCODED_3},\
    {"IMG_CF_USER_ENCODED_4", NULL, LV_IMG_CF_USER_ENCODED_4},\
    {"IMG_CF_USER_ENCODED_5", NULL, LV_IMG_CF_USER_ENCODED_5},\
    {"IMG_CF_USER_ENCODED_6", NULL, LV_IMG_CF_USER_ENCODED_6},\
    {"IMG_CF_USER_ENCODED_7", NULL, LV_IMG_CF_USER_ENCODED_7},\
    {"DISP_ROT_NONE", NULL, LV_DISP_ROT_NONE},\
    {"DISP_ROT_90", NULL, LV_DISP_ROT_90},\
    {"DISP_ROT_180", NULL, LV_DISP_ROT_180},\
    {"DISP_ROT_270", NULL, LV_DISP_ROT_270},\
    {"INDEV_TYPE_NONE", NULL, LV_INDEV_TYPE_NONE},\
    {"INDEV_TYPE_POINTER", NULL, LV_INDEV_TYPE_POINTER},\
    {"INDEV_TYPE_KEYPAD", NULL, LV_INDEV_TYPE_KEYPAD},\
    {"INDEV_TYPE_BUTTON", NULL, LV_INDEV_TYPE_BUTTON},\
    {"INDEV_TYPE_ENCODER", NULL, LV_INDEV_TYPE_ENCODER},\
    {"INDEV_STATE_RELEASED", NULL, LV_INDEV_STATE_RELEASED},\
    {"INDEV_STATE_PRESSED", NULL, LV_INDEV_STATE_PRESSED},\
    {"FONT_SUBPX_NONE", NULL, LV_FONT_SUBPX_NONE},\
    {"FONT_SUBPX_HOR", NULL, LV_FONT_SUBPX_HOR},\
    {"FONT_SUBPX_VER", NULL, LV_FONT_SUBPX_VER},\
    {"FONT_SUBPX_BOTH", NULL, LV_FONT_SUBPX_BOTH},\
    {"ANIM_OFF", NULL, LV_ANIM_OFF},\
    {"ANIM_ON", NULL, LV_ANIM_ON},\
    {"TEXT_FLAG_NONE", NULL, LV_TEXT_FLAG_NONE},\
    {"TEXT_FLAG_RECOLOR", NULL, LV_TEXT_FLAG_RECOLOR},\
    {"TEXT_FLAG_EXPAND", NULL, LV_TEXT_FLAG_EXPAND},\
    {"TEXT_FLAG_FIT", NULL, LV_TEXT_FLAG_FIT},\
    {"TEXT_CMD_STATE_WAIT", NULL, LV_TEXT_CMD_STATE_WAIT},\
    {"TEXT_CMD_STATE_PAR", NULL, LV_TEXT_CMD_STATE_PAR},\
    {"TEXT_CMD_STATE_IN", NULL, LV_TEXT_CMD_STATE_IN},\
    {"TEXT_ALIGN_AUTO", NULL, LV_TEXT_ALIGN_AUTO},\
    {"TEXT_ALIGN_LEFT", NULL, LV_TEXT_ALIGN_LEFT},\
    {"TEXT_ALIGN_CENTER", NULL, LV_TEXT_ALIGN_CENTER},\
    {"TEXT_ALIGN_RIGHT", NULL, LV_TEXT_ALIGN_RIGHT},\
    {"BASE_DIR_LTR", NULL, LV_BASE_DIR_LTR},\
    {"BASE_DIR_RTL", NULL, LV_BASE_DIR_RTL},\
    {"BASE_DIR_AUTO", NULL, LV_BASE_DIR_AUTO},\
    {"BASE_DIR_NEUTRAL", NULL, LV_BASE_DIR_NEUTRAL},\
    {"BASE_DIR_WEAK", NULL, LV_BASE_DIR_WEAK},\
    {"BLEND_MODE_NORMAL", NULL, LV_BLEND_MODE_NORMAL},\
    {"BLEND_MODE_ADDITIVE", NULL, LV_BLEND_MODE_ADDITIVE},\
    {"BLEND_MODE_SUBTRACTIVE", NULL, LV_BLEND_MODE_SUBTRACTIVE},\
    {"BLEND_MODE_MULTIPLY", NULL, LV_BLEND_MODE_MULTIPLY},\
    {"TEXT_DECOR_NONE", NULL, LV_TEXT_DECOR_NONE},\
    {"TEXT_DECOR_UNDERLINE", NULL, LV_TEXT_DECOR_UNDERLINE},\
    {"TEXT_DECOR_STRIKETHROUGH", NULL, LV_TEXT_DECOR_STRIKETHROUGH},\
    {"BORDER_SIDE_NONE", NULL, LV_BORDER_SIDE_NONE},\
    {"BORDER_SIDE_BOTTOM", NULL, LV_BORDER_SIDE_BOTTOM},\
    {"BORDER_SIDE_TOP", NULL, LV_BORDER_SIDE_TOP},\
    {"BORDER_SIDE_LEFT", NULL, LV_BORDER_SIDE_LEFT},\
    {"BORDER_SIDE_RIGHT", NULL, LV_BORDER_SIDE_RIGHT},\
    {"BORDER_SIDE_FULL", NULL, LV_BORDER_SIDE_FULL},\
    {"BORDER_SIDE_INTERNAL", NULL, LV_BORDER_SIDE_INTERNAL},\
    {"GRAD_DIR_NONE", NULL, LV_GRAD_DIR_NONE},\
    {"GRAD_DIR_VER", NULL, LV_GRAD_DIR_VER},\
    {"GRAD_DIR_HOR", NULL, LV_GRAD_DIR_HOR},\
    {"STYLE_PROP_INV", NULL, LV_STYLE_PROP_INV},\
    {"STYLE_WIDTH", NULL, LV_STYLE_WIDTH},\
    {"STYLE_MIN_WIDTH", NULL, LV_STYLE_MIN_WIDTH},\
    {"STYLE_MAX_WIDTH", NULL, LV_STYLE_MAX_WIDTH},\
    {"STYLE_HEIGHT", NULL, LV_STYLE_HEIGHT},\
    {"STYLE_MIN_HEIGHT", NULL, LV_STYLE_MIN_HEIGHT},\
    {"STYLE_MAX_HEIGHT", NULL, LV_STYLE_MAX_HEIGHT},\
    {"STYLE_X", NULL, LV_STYLE_X},\
    {"STYLE_Y", NULL, LV_STYLE_Y},\
    {"STYLE_ALIGN", NULL, LV_STYLE_ALIGN},\
    {"STYLE_TRANSFORM_WIDTH", NULL, LV_STYLE_TRANSFORM_WIDTH},\
    {"STYLE_TRANSFORM_HEIGHT", NULL, LV_STYLE_TRANSFORM_HEIGHT},\
    {"STYLE_TRANSLATE_X", NULL, LV_STYLE_TRANSLATE_X},\
    {"STYLE_TRANSLATE_Y", NULL, LV_STYLE_TRANSLATE_Y},\
    {"STYLE_TRANSFORM_ZOOM", NULL, LV_STYLE_TRANSFORM_ZOOM},\
    {"STYLE_TRANSFORM_ANGLE", NULL, LV_STYLE_TRANSFORM_ANGLE},\
    {"STYLE_PAD_TOP", NULL, LV_STYLE_PAD_TOP},\
    {"STYLE_PAD_BOTTOM", NULL, LV_STYLE_PAD_BOTTOM},\
    {"STYLE_PAD_LEFT", NULL, LV_STYLE_PAD_LEFT},\
    {"STYLE_PAD_RIGHT", NULL, LV_STYLE_PAD_RIGHT},\
    {"STYLE_PAD_ROW", NULL, LV_STYLE_PAD_ROW},\
    {"STYLE_PAD_COLUMN", NULL, LV_STYLE_PAD_COLUMN},\
    {"STYLE_BG_COLOR", NULL, LV_STYLE_BG_COLOR},\
    {"STYLE_BG_COLOR_FILTERED", NULL, LV_STYLE_BG_COLOR_FILTERED},\
    {"STYLE_BG_OPA", NULL, LV_STYLE_BG_OPA},\
    {"STYLE_BG_GRAD_COLOR", NULL, LV_STYLE_BG_GRAD_COLOR},\
    {"STYLE_BG_GRAD_COLOR_FILTERED", NULL, LV_STYLE_BG_GRAD_COLOR_FILTERED},\
    {"STYLE_BG_GRAD_DIR", NULL, LV_STYLE_BG_GRAD_DIR},\
    {"STYLE_BG_MAIN_STOP", NULL, LV_STYLE_BG_MAIN_STOP},\
    {"STYLE_BG_GRAD_STOP", NULL, LV_STYLE_BG_GRAD_STOP},\
    {"STYLE_BG_IMG_SRC", NULL, LV_STYLE_BG_IMG_SRC},\
    {"STYLE_BG_IMG_OPA", NULL, LV_STYLE_BG_IMG_OPA},\
    {"STYLE_BG_IMG_RECOLOR", NULL, LV_STYLE_BG_IMG_RECOLOR},\
    {"STYLE_BG_IMG_RECOLOR_FILTERED", NULL, LV_STYLE_BG_IMG_RECOLOR_FILTERED},\
    {"STYLE_BG_IMG_RECOLOR_OPA", NULL, LV_STYLE_BG_IMG_RECOLOR_OPA},\
    {"STYLE_BG_IMG_TILED", NULL, LV_STYLE_BG_IMG_TILED},\
    {"STYLE_BORDER_COLOR", NULL, LV_STYLE_BORDER_COLOR},\
    {"STYLE_BORDER_COLOR_FILTERED", NULL, LV_STYLE_BORDER_COLOR_FILTERED},\
    {"STYLE_BORDER_OPA", NULL, LV_STYLE_BORDER_OPA},\
    {"STYLE_BORDER_WIDTH", NULL, LV_STYLE_BORDER_WIDTH},\
    {"STYLE_BORDER_SIDE", NULL, LV_STYLE_BORDER_SIDE},\
    {"STYLE_BORDER_POST", NULL, LV_STYLE_BORDER_POST},\
    {"STYLE_OUTLINE_WIDTH", NULL, LV_STYLE_OUTLINE_WIDTH},\
    {"STYLE_OUTLINE_COLOR", NULL, LV_STYLE_OUTLINE_COLOR},\
    {"STYLE_OUTLINE_COLOR_FILTERED", NULL, LV_STYLE_OUTLINE_COLOR_FILTERED},\
    {"STYLE_OUTLINE_OPA", NULL, LV_STYLE_OUTLINE_OPA},\
    {"STYLE_OUTLINE_PAD", NULL, LV_STYLE_OUTLINE_PAD},\
    {"STYLE_SHADOW_WIDTH", NULL, LV_STYLE_SHADOW_WIDTH},\
    {"STYLE_SHADOW_OFS_X", NULL, LV_STYLE_SHADOW_OFS_X},\
    {"STYLE_SHADOW_OFS_Y", NULL, LV_STYLE_SHADOW_OFS_Y},\
    {"STYLE_SHADOW_SPREAD", NULL, LV_STYLE_SHADOW_SPREAD},\
    {"STYLE_SHADOW_COLOR", NULL, LV_STYLE_SHADOW_COLOR},\
    {"STYLE_SHADOW_COLOR_FILTERED", NULL, LV_STYLE_SHADOW_COLOR_FILTERED},\
    {"STYLE_SHADOW_OPA", NULL, LV_STYLE_SHADOW_OPA},\
    {"STYLE_IMG_OPA", NULL, LV_STYLE_IMG_OPA},\
    {"STYLE_IMG_RECOLOR", NULL, LV_STYLE_IMG_RECOLOR},\
    {"STYLE_IMG_RECOLOR_FILTERED", NULL, LV_STYLE_IMG_RECOLOR_FILTERED},\
    {"STYLE_IMG_RECOLOR_OPA", NULL, LV_STYLE_IMG_RECOLOR_OPA},\
    {"STYLE_LINE_WIDTH", NULL, LV_STYLE_LINE_WIDTH},\
    {"STYLE_LINE_DASH_WIDTH", NULL, LV_STYLE_LINE_DASH_WIDTH},\
    {"STYLE_LINE_DASH_GAP", NULL, LV_STYLE_LINE_DASH_GAP},\
    {"STYLE_LINE_ROUNDED", NULL, LV_STYLE_LINE_ROUNDED},\
    {"STYLE_LINE_COLOR", NULL, LV_STYLE_LINE_COLOR},\
    {"STYLE_LINE_COLOR_FILTERED", NULL, LV_STYLE_LINE_COLOR_FILTERED},\
    {"STYLE_LINE_OPA", NULL, LV_STYLE_LINE_OPA},\
    {"STYLE_ARC_WIDTH", NULL, LV_STYLE_ARC_WIDTH},\
    {"STYLE_ARC_ROUNDED", NULL, LV_STYLE_ARC_ROUNDED},\
    {"STYLE_ARC_COLOR", NULL, LV_STYLE_ARC_COLOR},\
    {"STYLE_ARC_COLOR_FILTERED", NULL, LV_STYLE_ARC_COLOR_FILTERED},\
    {"STYLE_ARC_OPA", NULL, LV_STYLE_ARC_OPA},\
    {"STYLE_ARC_IMG_SRC", NULL, LV_STYLE_ARC_IMG_SRC},\
    {"STYLE_TEXT_COLOR", NULL, LV_STYLE_TEXT_COLOR},\
    {"STYLE_TEXT_COLOR_FILTERED", NULL, LV_STYLE_TEXT_COLOR_FILTERED},\
    {"STYLE_TEXT_OPA", NULL, LV_STYLE_TEXT_OPA},\
    {"STYLE_TEXT_FONT", NULL, LV_STYLE_TEXT_FONT},\
    {"STYLE_TEXT_LETTER_SPACE", NULL, LV_STYLE_TEXT_LETTER_SPACE},\
    {"STYLE_TEXT_LINE_SPACE", NULL, LV_STYLE_TEXT_LINE_SPACE},\
    {"STYLE_TEXT_DECOR", NULL, LV_STYLE_TEXT_DECOR},\
    {"STYLE_TEXT_ALIGN", NULL, LV_STYLE_TEXT_ALIGN},\
    {"STYLE_RADIUS", NULL, LV_STYLE_RADIUS},\
    {"STYLE_CLIP_CORNER", NULL, LV_STYLE_CLIP_CORNER},\
    {"STYLE_OPA", NULL, LV_STYLE_OPA},\
    {"STYLE_COLOR_FILTER_DSC", NULL, LV_STYLE_COLOR_FILTER_DSC},\
    {"STYLE_COLOR_FILTER_OPA", NULL, LV_STYLE_COLOR_FILTER_OPA},\
    {"STYLE_ANIM_TIME", NULL, LV_STYLE_ANIM_TIME},\
    {"STYLE_ANIM_SPEED", NULL, LV_STYLE_ANIM_SPEED},\
    {"STYLE_TRANSITION", NULL, LV_STYLE_TRANSITION},\
    {"STYLE_BLEND_MODE", NULL, LV_STYLE_BLEND_MODE},\
    {"STYLE_LAYOUT", NULL, LV_STYLE_LAYOUT},\
    {"STYLE_BASE_DIR", NULL, LV_STYLE_BASE_DIR},\
    {"STYLE_PROP_ANY", NULL, LV_STYLE_PROP_ANY},\
    {"STATE_DEFAULT", NULL, LV_STATE_DEFAULT},\
    {"STATE_CHECKED", NULL, LV_STATE_CHECKED},\
    {"STATE_FOCUSED", NULL, LV_STATE_FOCUSED},\
    {"STATE_FOCUS_KEY", NULL, LV_STATE_FOCUS_KEY},\
    {"STATE_EDITED", NULL, LV_STATE_EDITED},\
    {"STATE_HOVERED", NULL, LV_STATE_HOVERED},\
    {"STATE_PRESSED", NULL, LV_STATE_PRESSED},\
    {"STATE_SCROLLED", NULL, LV_STATE_SCROLLED},\
    {"STATE_DISABLED", NULL, LV_STATE_DISABLED},\
    {"STATE_USER_1", NULL, LV_STATE_USER_1},\
    {"STATE_USER_2", NULL, LV_STATE_USER_2},\
    {"STATE_USER_3", NULL, LV_STATE_USER_3},\
    {"STATE_USER_4", NULL, LV_STATE_USER_4},\
    {"STATE_ANY", NULL, LV_STATE_ANY},\
    {"PART_MAIN", NULL, LV_PART_MAIN},\
    {"PART_SCROLLBAR", NULL, LV_PART_SCROLLBAR},\
    {"PART_INDICATOR", NULL, LV_PART_INDICATOR},\
    {"PART_KNOB", NULL, LV_PART_KNOB},\
    {"PART_SELECTED", NULL, LV_PART_SELECTED},\
    {"PART_ITEMS", NULL, LV_PART_ITEMS},\
    {"PART_TICKS", NULL, LV_PART_TICKS},\
    {"PART_CURSOR", NULL, LV_PART_CURSOR},\
    {"PART_CUSTOM_FIRST", NULL, LV_PART_CUSTOM_FIRST},\
    {"PART_ANY", NULL, LV_PART_ANY},\
    {"OBJ_FLAG_HIDDEN", NULL, LV_OBJ_FLAG_HIDDEN},\
    {"OBJ_FLAG_CLICKABLE", NULL, LV_OBJ_FLAG_CLICKABLE},\
    {"OBJ_FLAG_CLICK_FOCUSABLE", NULL, LV_OBJ_FLAG_CLICK_FOCUSABLE},\
    {"OBJ_FLAG_CHECKABLE", NULL, LV_OBJ_FLAG_CHECKABLE},\
    {"OBJ_FLAG_SCROLLABLE", NULL, LV_OBJ_FLAG_SCROLLABLE},\
    {"OBJ_FLAG_SCROLL_ELASTIC", NULL, LV_OBJ_FLAG_SCROLL_ELASTIC},\
    {"OBJ_FLAG_SCROLL_MOMENTUM", NULL, LV_OBJ_FLAG_SCROLL_MOMENTUM},\
    {"OBJ_FLAG_SCROLL_ONE", NULL, LV_OBJ_FLAG_SCROLL_ONE},\
    {"OBJ_FLAG_SCROLL_CHAIN", NULL, LV_OBJ_FLAG_SCROLL_CHAIN},\
    {"OBJ_FLAG_SCROLL_ON_FOCUS", NULL, LV_OBJ_FLAG_SCROLL_ON_FOCUS},\
    {"OBJ_FLAG_SNAPPABLE", NULL, LV_OBJ_FLAG_SNAPPABLE},\
    {"OBJ_FLAG_PRESS_LOCK", NULL, LV_OBJ_FLAG_PRESS_LOCK},\
    {"OBJ_FLAG_EVENT_BUBBLE", NULL, LV_OBJ_FLAG_EVENT_BUBBLE},\
    {"OBJ_FLAG_GESTURE_BUBBLE", NULL, LV_OBJ_FLAG_GESTURE_BUBBLE},\
    {"OBJ_FLAG_ADV_HITTEST", NULL, LV_OBJ_FLAG_ADV_HITTEST},\
    {"OBJ_FLAG_IGNORE_LAYOUT", NULL, LV_OBJ_FLAG_IGNORE_LAYOUT},\
    {"OBJ_FLAG_FLOATING", NULL, LV_OBJ_FLAG_FLOATING},\
    {"OBJ_FLAG_LAYOUT_1", NULL, LV_OBJ_FLAG_LAYOUT_1},\
    {"OBJ_FLAG_LAYOUT_2", NULL, LV_OBJ_FLAG_LAYOUT_2},\
    {"OBJ_FLAG_WIDGET_1", NULL, LV_OBJ_FLAG_WIDGET_1},\
    {"OBJ_FLAG_WIDGET_2", NULL, LV_OBJ_FLAG_WIDGET_2},\
    {"OBJ_FLAG_USER_1", NULL, LV_OBJ_FLAG_USER_1},\
    {"OBJ_FLAG_USER_2", NULL, LV_OBJ_FLAG_USER_2},\
    {"OBJ_FLAG_USER_3", NULL, LV_OBJ_FLAG_USER_3},\
    {"OBJ_FLAG_USER_4", NULL, LV_OBJ_FLAG_USER_4},\
    {"OBJ_DRAW_PART_RECTANGLE", NULL, LV_OBJ_DRAW_PART_RECTANGLE},\
    {"OBJ_DRAW_PART_BORDER_POST", NULL, LV_OBJ_DRAW_PART_BORDER_POST},\
    {"OBJ_DRAW_PART_SCROLLBAR", NULL, LV_OBJ_DRAW_PART_SCROLLBAR},\
    {"OBJ_TREE_WALK_NEXT", NULL, LV_OBJ_TREE_WALK_NEXT},\
    {"OBJ_TREE_WALK_SKIP_CHILDREN", NULL, LV_OBJ_TREE_WALK_SKIP_CHILDREN},\
    {"OBJ_TREE_WALK_END", NULL, LV_OBJ_TREE_WALK_END},\
    {"SCROLLBAR_MODE_OFF", NULL, LV_SCROLLBAR_MODE_OFF},\
    {"SCROLLBAR_MODE_ON", NULL, LV_SCROLLBAR_MODE_ON},\
    {"SCROLLBAR_MODE_ACTIVE", NULL, LV_SCROLLBAR_MODE_ACTIVE},\
    {"SCROLLBAR_MODE_AUTO", NULL, LV_SCROLLBAR_MODE_AUTO},\
    {"SCROLL_SNAP_NONE", NULL, LV_SCROLL_SNAP_NONE},\
    {"SCROLL_SNAP_START", NULL, LV_SCROLL_SNAP_START},\
    {"SCROLL_SNAP_END", NULL, LV_SCROLL_SNAP_END},\
    {"SCROLL_SNAP_CENTER", NULL, LV_SCROLL_SNAP_CENTER},\
    {"FS_RES_OK", NULL, LV_FS_RES_OK},\
    {"FS_RES_HW_ERR", NULL, LV_FS_RES_HW_ERR},\
    {"FS_RES_FS_ERR", NULL, LV_FS_RES_FS_ERR},\
    {"FS_RES_NOT_EX", NULL, LV_FS_RES_NOT_EX},\
    {"FS_RES_FULL", NULL, LV_FS_RES_FULL},\
    {"FS_RES_LOCKED", NULL, LV_FS_RES_LOCKED},\
    {"FS_RES_DENIED", NULL, LV_FS_RES_DENIED},\
    {"FS_RES_BUSY", NULL, LV_FS_RES_BUSY},\
    {"FS_RES_TOUT", NULL, LV_FS_RES_TOUT},\
    {"FS_RES_NOT_IMP", NULL, LV_FS_RES_NOT_IMP},\
    {"FS_RES_OUT_OF_MEM", NULL, LV_FS_RES_OUT_OF_MEM},\
    {"FS_RES_INV_PARAM", NULL, LV_FS_RES_INV_PARAM},\
    {"FS_RES_UNKNOWN", NULL, LV_FS_RES_UNKNOWN},\
    {"FS_MODE_WR", NULL, LV_FS_MODE_WR},\
    {"FS_MODE_RD", NULL, LV_FS_MODE_RD},\
    {"FS_SEEK_SET", NULL, LV_FS_SEEK_SET},\
    {"FS_SEEK_CUR", NULL, LV_FS_SEEK_CUR},\
    {"FS_SEEK_END", NULL, LV_FS_SEEK_END},\
    {"IMG_SRC_VARIABLE", NULL, LV_IMG_SRC_VARIABLE},\
    {"IMG_SRC_FILE", NULL, LV_IMG_SRC_FILE},\
    {"IMG_SRC_SYMBOL", NULL, LV_IMG_SRC_SYMBOL},\
    {"IMG_SRC_UNKNOWN", NULL, LV_IMG_SRC_UNKNOWN},\
    {"DRAW_MASK_RES_TRANSP", NULL, LV_DRAW_MASK_RES_TRANSP},\
    {"DRAW_MASK_RES_FULL_COVER", NULL, LV_DRAW_MASK_RES_FULL_COVER},\
    {"DRAW_MASK_RES_CHANGED", NULL, LV_DRAW_MASK_RES_CHANGED},\
    {"DRAW_MASK_RES_UNKNOWN", NULL, LV_DRAW_MASK_RES_UNKNOWN},\
    {"DRAW_MASK_TYPE_LINE", NULL, LV_DRAW_MASK_TYPE_LINE},\
    {"DRAW_MASK_TYPE_ANGLE", NULL, LV_DRAW_MASK_TYPE_ANGLE},\
    {"DRAW_MASK_TYPE_RADIUS", NULL, LV_DRAW_MASK_TYPE_RADIUS},\
    {"DRAW_MASK_TYPE_FADE", NULL, LV_DRAW_MASK_TYPE_FADE},\
    {"DRAW_MASK_TYPE_MAP", NULL, LV_DRAW_MASK_TYPE_MAP},\
    {"DRAW_MASK_LINE_SIDE_LEFT", NULL, LV_DRAW_MASK_LINE_SIDE_LEFT},\
    {"DRAW_MASK_LINE_SIDE_RIGHT", NULL, LV_DRAW_MASK_LINE_SIDE_RIGHT},\
    {"DRAW_MASK_LINE_SIDE_TOP", NULL, LV_DRAW_MASK_LINE_SIDE_TOP},\
    {"DRAW_MASK_LINE_SIDE_BOTTOM", NULL, LV_DRAW_MASK_LINE_SIDE_BOTTOM},\
    {"COVER_RES_COVER", NULL, LV_COVER_RES_COVER},\
    {"COVER_RES_NOT_COVER", NULL, LV_COVER_RES_NOT_COVER},\
    {"COVER_RES_MASKED", NULL, LV_COVER_RES_MASKED},\
    {"OBJ_CLASS_EDITABLE_INHERIT", NULL, LV_OBJ_CLASS_EDITABLE_INHERIT},\
    {"OBJ_CLASS_EDITABLE_TRUE", NULL, LV_OBJ_CLASS_EDITABLE_TRUE},\
    {"OBJ_CLASS_EDITABLE_FALSE", NULL, LV_OBJ_CLASS_EDITABLE_FALSE},\
    {"OBJ_CLASS_GROUP_DEF_INHERIT", NULL, LV_OBJ_CLASS_GROUP_DEF_INHERIT},\
    {"OBJ_CLASS_GROUP_DEF_TRUE", NULL, LV_OBJ_CLASS_GROUP_DEF_TRUE},\
    {"OBJ_CLASS_GROUP_DEF_FALSE", NULL, LV_OBJ_CLASS_GROUP_DEF_FALSE},\
    {"EVENT_ALL", NULL, LV_EVENT_ALL},\
    {"EVENT_PRESSED", NULL, LV_EVENT_PRESSED},\
    {"EVENT_PRESSING", NULL, LV_EVENT_PRESSING},\
    {"EVENT_PRESS_LOST", NULL, LV_EVENT_PRESS_LOST},\
    {"EVENT_SHORT_CLICKED", NULL, LV_EVENT_SHORT_CLICKED},\
    {"EVENT_LONG_PRESSED", NULL, LV_EVENT_LONG_PRESSED},\
    {"EVENT_LONG_PRESSED_REPEAT", NULL, LV_EVENT_LONG_PRESSED_REPEAT},\
    {"EVENT_CLICKED", NULL, LV_EVENT_CLICKED},\
    {"EVENT_RELEASED", NULL, LV_EVENT_RELEASED},\
    {"EVENT_SCROLL_BEGIN", NULL, LV_EVENT_SCROLL_BEGIN},\
    {"EVENT_SCROLL_END", NULL, LV_EVENT_SCROLL_END},\
    {"EVENT_SCROLL", NULL, LV_EVENT_SCROLL},\
    {"EVENT_GESTURE", NULL, LV_EVENT_GESTURE},\
    {"EVENT_KEY", NULL, LV_EVENT_KEY},\
    {"EVENT_FOCUSED", NULL, LV_EVENT_FOCUSED},\
    {"EVENT_DEFOCUSED", NULL, LV_EVENT_DEFOCUSED},\
    {"EVENT_LEAVE", NULL, LV_EVENT_LEAVE},\
    {"EVENT_HIT_TEST", NULL, LV_EVENT_HIT_TEST},\
    {"EVENT_COVER_CHECK", NULL, LV_EVENT_COVER_CHECK},\
    {"EVENT_REFR_EXT_DRAW_SIZE", NULL, LV_EVENT_REFR_EXT_DRAW_SIZE},\
    {"EVENT_DRAW_MAIN_BEGIN", NULL, LV_EVENT_DRAW_MAIN_BEGIN},\
    {"EVENT_DRAW_MAIN", NULL, LV_EVENT_DRAW_MAIN},\
    {"EVENT_DRAW_MAIN_END", NULL, LV_EVENT_DRAW_MAIN_END},\
    {"EVENT_DRAW_POST_BEGIN", NULL, LV_EVENT_DRAW_POST_BEGIN},\
    {"EVENT_DRAW_POST", NULL, LV_EVENT_DRAW_POST},\
    {"EVENT_DRAW_POST_END", NULL, LV_EVENT_DRAW_POST_END},\
    {"EVENT_DRAW_PART_BEGIN", NULL, LV_EVENT_DRAW_PART_BEGIN},\
    {"EVENT_DRAW_PART_END", NULL, LV_EVENT_DRAW_PART_END},\
    {"EVENT_VALUE_CHANGED", NULL, LV_EVENT_VALUE_CHANGED},\
    {"EVENT_INSERT", NULL, LV_EVENT_INSERT},\
    {"EVENT_REFRESH", NULL, LV_EVENT_REFRESH},\
    {"EVENT_READY", NULL, LV_EVENT_READY},\
    {"EVENT_CANCEL", NULL, LV_EVENT_CANCEL},\
    {"EVENT_DELETE", NULL, LV_EVENT_DELETE},\
    {"EVENT_CHILD_CHANGED", NULL, LV_EVENT_CHILD_CHANGED},\
    {"EVENT_CHILD_CREATED", NULL, LV_EVENT_CHILD_CREATED},\
    {"EVENT_CHILD_DELETED", NULL, LV_EVENT_CHILD_DELETED},\
    {"EVENT_SCREEN_UNLOAD_START", NULL, LV_EVENT_SCREEN_UNLOAD_START},\
    {"EVENT_SCREEN_LOAD_START", NULL, LV_EVENT_SCREEN_LOAD_START},\
    {"EVENT_SCREEN_LOADED", NULL, LV_EVENT_SCREEN_LOADED},\
    {"EVENT_SCREEN_UNLOADED", NULL, LV_EVENT_SCREEN_UNLOADED},\
    {"EVENT_SIZE_CHANGED", NULL, LV_EVENT_SIZE_CHANGED},\
    {"EVENT_STYLE_CHANGED", NULL, LV_EVENT_STYLE_CHANGED},\
    {"EVENT_LAYOUT_CHANGED", NULL, LV_EVENT_LAYOUT_CHANGED},\
    {"EVENT_GET_SELF_SIZE", NULL, LV_EVENT_GET_SELF_SIZE},\
    {"KEY_UP", NULL, LV_KEY_UP},\
    {"KEY_DOWN", NULL, LV_KEY_DOWN},\
    {"KEY_RIGHT", NULL, LV_KEY_RIGHT},\
    {"KEY_LEFT", NULL, LV_KEY_LEFT},\
    {"KEY_ESC", NULL, LV_KEY_ESC},\
    {"KEY_DEL", NULL, LV_KEY_DEL},\
    {"KEY_BACKSPACE", NULL, LV_KEY_BACKSPACE},\
    {"KEY_ENTER", NULL, LV_KEY_ENTER},\
    {"KEY_NEXT", NULL, LV_KEY_NEXT},\
    {"KEY_PREV", NULL, LV_KEY_PREV},\
    {"KEY_HOME", NULL, LV_KEY_HOME},\
    {"KEY_END", NULL, LV_KEY_END},\
    {"GROUP_REFOCUS_POLICY_NEXT", NULL, LV_GROUP_REFOCUS_POLICY_NEXT},\
    {"GROUP_REFOCUS_POLICY_PREV", NULL, LV_GROUP_REFOCUS_POLICY_PREV},\
    {"SCR_LOAD_ANIM_NONE", NULL, LV_SCR_LOAD_ANIM_NONE},\
    {"SCR_LOAD_ANIM_OVER_LEFT", NULL, LV_SCR_LOAD_ANIM_OVER_LEFT},\
    {"SCR_LOAD_ANIM_OVER_RIGHT", NULL, LV_SCR_LOAD_ANIM_OVER_RIGHT},\
    {"SCR_LOAD_ANIM_OVER_TOP", NULL, LV_SCR_LOAD_ANIM_OVER_TOP},\
    {"SCR_LOAD_ANIM_OVER_BOTTOM", NULL, LV_SCR_LOAD_ANIM_OVER_BOTTOM},\
    {"SCR_LOAD_ANIM_MOVE_LEFT", NULL, LV_SCR_LOAD_ANIM_MOVE_LEFT},\
    {"SCR_LOAD_ANIM_MOVE_RIGHT", NULL, LV_SCR_LOAD_ANIM_MOVE_RIGHT},\
    {"SCR_LOAD_ANIM_MOVE_TOP", NULL, LV_SCR_LOAD_ANIM_MOVE_TOP},\
    {"SCR_LOAD_ANIM_MOVE_BOTTOM", NULL, LV_SCR_LOAD_ANIM_MOVE_BOTTOM},\
    {"SCR_LOAD_ANIM_FADE_ON", NULL, LV_SCR_LOAD_ANIM_FADE_ON},\
    {"FONT_FMT_TXT_CMAP_FORMAT0_FULL", NULL, LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL},\
    {"FONT_FMT_TXT_CMAP_SPARSE_FULL", NULL, LV_FONT_FMT_TXT_CMAP_SPARSE_FULL},\
    {"FONT_FMT_TXT_CMAP_FORMAT0_TINY", NULL, LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY},\
    {"FONT_FMT_TXT_CMAP_SPARSE_TINY", NULL, LV_FONT_FMT_TXT_CMAP_SPARSE_TINY},\
    {"FONT_FMT_TXT_PLAIN", NULL, LV_FONT_FMT_TXT_PLAIN},\
    {"FONT_FMT_TXT_COMPRESSED", NULL, LV_FONT_FMT_TXT_COMPRESSED},\
    {"FONT_FMT_TXT_COMPRESSED_NO_PREFILTER", NULL, LV_FONT_FMT_TXT_COMPRESSED_NO_PREFILTER},\
    {"ARC_MODE_NORMAL", NULL, LV_ARC_MODE_NORMAL},\
    {"ARC_MODE_SYMMETRICAL", NULL, LV_ARC_MODE_SYMMETRICAL},\
    {"ARC_MODE_REVERSE", NULL, LV_ARC_MODE_REVERSE},\
    {"ARC_DRAW_PART_BACKGROUND", NULL, LV_ARC_DRAW_PART_BACKGROUND},\
    {"ARC_DRAW_PART_FOREGROUND", NULL, LV_ARC_DRAW_PART_FOREGROUND},\
    {"ARC_DRAW_PART_KNOB", NULL, LV_ARC_DRAW_PART_KNOB},\
    {"LABEL_LONG_WRAP", NULL, LV_LABEL_LONG_WRAP},\
    {"LABEL_LONG_DOT", NULL, LV_LABEL_LONG_DOT},\
    {"LABEL_LONG_SCROLL", NULL, LV_LABEL_LONG_SCROLL},\
    {"LABEL_LONG_SCROLL_CIRCULAR", NULL, LV_LABEL_LONG_SCROLL_CIRCULAR},\
    {"LABEL_LONG_CLIP", NULL, LV_LABEL_LONG_CLIP},\
    {"BAR_MODE_NORMAL", NULL, LV_BAR_MODE_NORMAL},\
    {"BAR_MODE_SYMMETRICAL", NULL, LV_BAR_MODE_SYMMETRICAL},\
    {"BAR_MODE_RANGE", NULL, LV_BAR_MODE_RANGE},\
    {"BAR_DRAW_PART_INDICATOR", NULL, LV_BAR_DRAW_PART_INDICATOR},\
    {"BTNMATRIX_CTRL_HIDDEN", NULL, LV_BTNMATRIX_CTRL_HIDDEN},\
    {"BTNMATRIX_CTRL_NO_REPEAT", NULL, LV_BTNMATRIX_CTRL_NO_REPEAT},\
    {"BTNMATRIX_CTRL_DISABLED", NULL, LV_BTNMATRIX_CTRL_DISABLED},\
    {"BTNMATRIX_CTRL_CHECKABLE", NULL, LV_BTNMATRIX_CTRL_CHECKABLE},\
    {"BTNMATRIX_CTRL_CHECKED", NULL, LV_BTNMATRIX_CTRL_CHECKED},\
    {"BTNMATRIX_CTRL_CLICK_TRIG", NULL, LV_BTNMATRIX_CTRL_CLICK_TRIG},\
    {"BTNMATRIX_CTRL_POPOVER", NULL, LV_BTNMATRIX_CTRL_POPOVER},\
    {"BTNMATRIX_CTRL_RECOLOR", NULL, LV_BTNMATRIX_CTRL_RECOLOR},\
    {"BTNMATRIX_CTRL_CUSTOM_1", NULL, LV_BTNMATRIX_CTRL_CUSTOM_1},\
    {"BTNMATRIX_CTRL_CUSTOM_2", NULL, LV_BTNMATRIX_CTRL_CUSTOM_2},\
    {"BTNMATRIX_DRAW_PART_BTN", NULL, LV_BTNMATRIX_DRAW_PART_BTN},\
    {"IMG_SIZE_MODE_VIRTUAL", NULL, LV_IMG_SIZE_MODE_VIRTUAL},\
    {"IMG_SIZE_MODE_REAL", NULL, LV_IMG_SIZE_MODE_REAL},\
    {"CHECKBOX_DRAW_PART_BOX", NULL, LV_CHECKBOX_DRAW_PART_BOX},\
    {"ROLLER_MODE_NORMAL", NULL, LV_ROLLER_MODE_NORMAL},\
    {"ROLLER_MODE_INFINITE", NULL, LV_ROLLER_MODE_INFINITE},\
    {"SLIDER_MODE_NORMAL", NULL, LV_SLIDER_MODE_NORMAL},\
    {"SLIDER_MODE_SYMMETRICAL", NULL, LV_SLIDER_MODE_SYMMETRICAL},\
    {"SLIDER_MODE_RANGE", NULL, LV_SLIDER_MODE_RANGE},\
    {"SLIDER_DRAW_PART_KNOB", NULL, LV_SLIDER_DRAW_PART_KNOB},\
    {"SLIDER_DRAW_PART_KNOB_LEFT", NULL, LV_SLIDER_DRAW_PART_KNOB_LEFT},\
    {"TABLE_CELL_CTRL_MERGE_RIGHT", NULL, LV_TABLE_CELL_CTRL_MERGE_RIGHT},\
    {"TABLE_CELL_CTRL_TEXT_CROP", NULL, LV_TABLE_CELL_CTRL_TEXT_CROP},\
    {"TABLE_CELL_CTRL_CUSTOM_1", NULL, LV_TABLE_CELL_CTRL_CUSTOM_1},\
    {"TABLE_CELL_CTRL_CUSTOM_2", NULL, LV_TABLE_CELL_CTRL_CUSTOM_2},\
    {"TABLE_CELL_CTRL_CUSTOM_3", NULL, LV_TABLE_CELL_CTRL_CUSTOM_3},\
    {"TABLE_CELL_CTRL_CUSTOM_4", NULL, LV_TABLE_CELL_CTRL_CUSTOM_4},\
    {"TABLE_DRAW_PART_CELL", NULL, LV_TABLE_DRAW_PART_CELL},\
    {"PART_TEXTAREA_PLACEHOLDER", NULL, LV_PART_TEXTAREA_PLACEHOLDER},\
    {"FLEX_ALIGN_START", NULL, LV_FLEX_ALIGN_START},\
    {"FLEX_ALIGN_END", NULL, LV_FLEX_ALIGN_END},\
    {"FLEX_ALIGN_CENTER", NULL, LV_FLEX_ALIGN_CENTER},\
    {"FLEX_ALIGN_SPACE_EVENLY", NULL, LV_FLEX_ALIGN_SPACE_EVENLY},\
    {"FLEX_ALIGN_SPACE_AROUND", NULL, LV_FLEX_ALIGN_SPACE_AROUND},\
    {"FLEX_ALIGN_SPACE_BETWEEN", NULL, LV_FLEX_ALIGN_SPACE_BETWEEN},\
    {"FLEX_FLOW_ROW", NULL, LV_FLEX_FLOW_ROW},\
    {"FLEX_FLOW_COLUMN", NULL, LV_FLEX_FLOW_COLUMN},\
    {"FLEX_FLOW_ROW_WRAP", NULL, LV_FLEX_FLOW_ROW_WRAP},\
    {"FLEX_FLOW_ROW_REVERSE", NULL, LV_FLEX_FLOW_ROW_REVERSE},\
    {"FLEX_FLOW_ROW_WRAP_REVERSE", NULL, LV_FLEX_FLOW_ROW_WRAP_REVERSE},\
    {"FLEX_FLOW_COLUMN_WRAP", NULL, LV_FLEX_FLOW_COLUMN_WRAP},\
    {"FLEX_FLOW_COLUMN_REVERSE", NULL, LV_FLEX_FLOW_COLUMN_REVERSE},\
    {"FLEX_FLOW_COLUMN_WRAP_REVERSE", NULL, LV_FLEX_FLOW_COLUMN_WRAP_REVERSE},\
    {"GRID_ALIGN_START", NULL, LV_GRID_ALIGN_START},\
    {"GRID_ALIGN_CENTER", NULL, LV_GRID_ALIGN_CENTER},\
    {"GRID_ALIGN_END", NULL, LV_GRID_ALIGN_END},\
    {"GRID_ALIGN_STRETCH", NULL, LV_GRID_ALIGN_STRETCH},\
    {"GRID_ALIGN_SPACE_EVENLY", NULL, LV_GRID_ALIGN_SPACE_EVENLY},\
    {"GRID_ALIGN_SPACE_AROUND", NULL, LV_GRID_ALIGN_SPACE_AROUND},\
    {"GRID_ALIGN_SPACE_BETWEEN", NULL, LV_GRID_ALIGN_SPACE_BETWEEN},\
    {"CHART_TYPE_NONE", NULL, LV_CHART_TYPE_NONE},\
    {"CHART_TYPE_LINE", NULL, LV_CHART_TYPE_LINE},\
    {"CHART_TYPE_BAR", NULL, LV_CHART_TYPE_BAR},\
    {"CHART_TYPE_SCATTER", NULL, LV_CHART_TYPE_SCATTER},\
    {"CHART_UPDATE_MODE_SHIFT", NULL, LV_CHART_UPDATE_MODE_SHIFT},\
    {"CHART_UPDATE_MODE_CIRCULAR", NULL, LV_CHART_UPDATE_MODE_CIRCULAR},\
    {"CHART_AXIS_PRIMARY_Y", NULL, LV_CHART_AXIS_PRIMARY_Y},\
    {"CHART_AXIS_SECONDARY_Y", NULL, LV_CHART_AXIS_SECONDARY_Y},\
    {"CHART_AXIS_PRIMARY_X", NULL, LV_CHART_AXIS_PRIMARY_X},\
    {"CHART_AXIS_SECONDARY_X", NULL, LV_CHART_AXIS_SECONDARY_X},\
    {"CHART_DRAW_PART_DIV_LINE_INIT", NULL, LV_CHART_DRAW_PART_DIV_LINE_INIT},\
    {"CHART_DRAW_PART_DIV_LINE_HOR", NULL, LV_CHART_DRAW_PART_DIV_LINE_HOR},\
    {"CHART_DRAW_PART_DIV_LINE_VER", NULL, LV_CHART_DRAW_PART_DIV_LINE_VER},\
    {"CHART_DRAW_PART_LINE_AND_POINT", NULL, LV_CHART_DRAW_PART_LINE_AND_POINT},\
    {"CHART_DRAW_PART_BAR", NULL, LV_CHART_DRAW_PART_BAR},\
    {"CHART_DRAW_PART_CURSOR", NULL, LV_CHART_DRAW_PART_CURSOR},\
    {"CHART_DRAW_PART_TICK_LABEL", NULL, LV_CHART_DRAW_PART_TICK_LABEL},\
    {"KEYBOARD_MODE_TEXT_LOWER", NULL, LV_KEYBOARD_MODE_TEXT_LOWER},\
    {"KEYBOARD_MODE_TEXT_UPPER", NULL, LV_KEYBOARD_MODE_TEXT_UPPER},\
    {"KEYBOARD_MODE_SPECIAL", NULL, LV_KEYBOARD_MODE_SPECIAL},\
    {"KEYBOARD_MODE_NUMBER", NULL, LV_KEYBOARD_MODE_NUMBER},\
    {"KEYBOARD_MODE_USER_1", NULL, LV_KEYBOARD_MODE_USER_1},\
    {"KEYBOARD_MODE_USER_2", NULL, LV_KEYBOARD_MODE_USER_2},\
    {"KEYBOARD_MODE_USER_3", NULL, LV_KEYBOARD_MODE_USER_3},\
    {"KEYBOARD_MODE_USER_4", NULL, LV_KEYBOARD_MODE_USER_4},\
    {"METER_INDICATOR_TYPE_NEEDLE_IMG", NULL, LV_METER_INDICATOR_TYPE_NEEDLE_IMG},\
    {"METER_INDICATOR_TYPE_NEEDLE_LINE", NULL, LV_METER_INDICATOR_TYPE_NEEDLE_LINE},\
    {"METER_INDICATOR_TYPE_SCALE_LINES", NULL, LV_METER_INDICATOR_TYPE_SCALE_LINES},\
    {"METER_INDICATOR_TYPE_ARC", NULL, LV_METER_INDICATOR_TYPE_ARC},\
    {"METER_DRAW_PART_ARC", NULL, LV_METER_DRAW_PART_ARC},\
    {"METER_DRAW_PART_NEEDLE_LINE", NULL, LV_METER_DRAW_PART_NEEDLE_LINE},\
    {"METER_DRAW_PART_NEEDLE_IMG", NULL, LV_METER_DRAW_PART_NEEDLE_IMG},\
    {"METER_DRAW_PART_TICK", NULL, LV_METER_DRAW_PART_TICK},\
    {"COLORWHEEL_MODE_HUE", NULL, LV_COLORWHEEL_MODE_HUE},\
    {"COLORWHEEL_MODE_SATURATION", NULL, LV_COLORWHEEL_MODE_SATURATION},\
    {"COLORWHEEL_MODE_VALUE", NULL, LV_COLORWHEEL_MODE_VALUE},\
    {"LED_DRAW_PART_RECTANGLE", NULL, LV_LED_DRAW_PART_RECTANGLE},\
    {"IMGBTN_STATE_RELEASED", NULL, LV_IMGBTN_STATE_RELEASED},\
    {"IMGBTN_STATE_PRESSED", NULL, LV_IMGBTN_STATE_PRESSED},\
    {"IMGBTN_STATE_DISABLED", NULL, LV_IMGBTN_STATE_DISABLED},\
    {"IMGBTN_STATE_CHECKED_RELEASED", NULL, LV_IMGBTN_STATE_CHECKED_RELEASED},\
    {"IMGBTN_STATE_CHECKED_PRESSED", NULL, LV_IMGBTN_STATE_CHECKED_PRESSED},\
    {"IMGBTN_STATE_CHECKED_DISABLED", NULL, LV_IMGBTN_STATE_CHECKED_DISABLED},\
    {"SPAN_OVERFLOW_CLIP", NULL, LV_SPAN_OVERFLOW_CLIP},\
    {"SPAN_OVERFLOW_ELLIPSIS", NULL, LV_SPAN_OVERFLOW_ELLIPSIS},\
    {"SPAN_MODE_FIXED", NULL, LV_SPAN_MODE_FIXED},\
    {"SPAN_MODE_EXPAND", NULL, LV_SPAN_MODE_EXPAND},\
    {"SPAN_MODE_BREAK", NULL, LV_SPAN_MODE_BREAK},\
    {"ANIM_IMG_PART_MAIN", NULL, LV_ANIM_IMG_PART_MAIN},\


#endif
