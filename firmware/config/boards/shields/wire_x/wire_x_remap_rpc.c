#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/util.h>

#if IS_ENABLED(CONFIG_BT_ZEPHYR_NUS)
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/services/nus.h>
#endif

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/sensor_event.h>
#include <zmk/keymap.h>
#include <zmk/rgb_underglow.h>
#if IS_ENABLED(CONFIG_ZMK_BLE)
#include <zmk/ble.h>
#endif

#include <dt-bindings/zmk/keys.h>

#define WIRE_X_RPC_UART_NODE DT_CHOSEN(zephyr_console)
#define WIRE_X_RPC_MAX_LINE 1024
#define WIRE_X_RPC_POSITIONS 14
#define WIRE_X_RPC_LAYERS 6
#define WIRE_X_RPC_LEGACY_LAYERS 4
#define WIRE_X_DEVICE_LAYER_INDEX 5
#define WIRE_X_RPC_ENCODERS 2
#define WIRE_X_RPC_MACROS 60
#define WIRE_X_RPC_MACRO_NAME_MAX 24
#define WIRE_X_RPC_MACRO_SEQUENCE_MAX 256
#define WIRE_X_RPC_NO_MACRO -1
#define WIRE_X_LAYER_KEY_POSITION 5
#define WIRE_X_RPC_BLE_CHUNK 20
#define WIRE_X_RPC_BLE_SEND_RETRIES 80
#define WIRE_X_RPC_BLE_RETRY_MS 5
#define WIRE_X_RPC_BLE_TX_GAP_MS 3
#define WIRE_X_ENCODER_SETTINGS_KEY "wire_x/encoders"
#define WIRE_X_ENCODER_MACRO_SETTINGS_KEY "wire_x/encoder_macros"
#define WIRE_X_ENCODER_SENSITIVITY_SETTINGS_KEY "wire_x/encoder_sensitivity"
#define WIRE_X_ENCODER_SENSITIVITY_MIN 1
#define WIRE_X_ENCODER_SENSITIVITY_MAX 4
#define WIRE_X_RGB_SETTINGS_KEY "wire_x/rgb"
#define WIRE_X_POWER_SETTINGS_KEY "wire_x/power"
#define WIRE_X_MACRO_BINDINGS_SETTINGS_KEY "wire_x/macro_bindings"
#define WIRE_X_MACRO_SETTINGS_PREFIX "wire_x/macro/"
#define WIRE_X_TAC_W_FACTORY_MIGRATION_SETTINGS_KEY "wire_x/tac_w_factory_v1"
#define WIRE_X_RGB_EFFECTS 4
#define WIRE_X_RGB_LAYER_GLOW_MS 4000
#define WIRE_X_RGB_BLE_GLOW_MS 5000
#define WIRE_X_RGB_BLE_BLINK_MS 450
#define WIRE_X_BLE_ADVERTISING_TIMEOUT_MS 90000
#define WIRE_X_BLE_PROFILE_LONG_PRESS_MS 1100
#define WIRE_X_RGB_BLE_SAT 100
#define WIRE_X_RGB_BLE_BRT_MIN 45
#define WIRE_X_LAYER_LONG_PRESS_MS 850
#define WIRE_X_LAYER_KEY_BINDING "LAYER"
#define WIRE_X_KEEP_BINDING "KEEP"
#define WIRE_X_UNSUPPORTED_BINDING "KEEP"
#define WIRE_X_DEVICE_NAME "TAC-W"
#define WIRE_X_BLE_PROFILE_START_POSITION 2
#define WIRE_X_BLE_PROFILE_COUNT 3
#define WIRE_X_FACTORY_RESET_POSITION 8
#define WIRE_X_FACTORY_RESET_LONG_PRESS_MS 1800

#define WIRE_X_DISPLAY_LABEL_LAYER 0
#define WIRE_X_DISPLAY_LABEL_KEY 1
#define WIRE_X_DISPLAY_LABEL_ENCODER 2

enum wire_x_rpc_transport {
    WIRE_X_RPC_TRANSPORT_UART,
    WIRE_X_RPC_TRANSPORT_BLE,
};

struct wire_x_rpc_target {
    enum wire_x_rpc_transport transport;
    struct bt_conn *conn;
};

enum wire_x_binding_kind {
    WIRE_X_BIND_NONE,
    WIRE_X_BIND_KP,
};

struct wire_x_keycode {
    const char *name;
    enum wire_x_binding_kind kind;
    uint32_t param1;
    uint32_t param2;
};

struct wire_x_rgb_config {
    uint16_t hue;
    uint8_t saturation;
    uint8_t brightness;
    uint8_t effect;
    bool on;
    bool reactive;
    bool layer_mode;
    struct {
        uint16_t hue;
        uint8_t saturation;
    } layer_colors[WIRE_X_RPC_LAYERS];
};

struct wire_x_rgb_config_v1 {
    uint16_t hue;
    uint8_t saturation;
    uint8_t brightness;
    uint8_t effect;
    bool on;
    bool reactive;
};

struct wire_x_rgb_config_v2_legacy {
    uint16_t hue;
    uint8_t saturation;
    uint8_t brightness;
    uint8_t effect;
    bool on;
    bool reactive;
    bool layer_mode;
    struct {
        uint16_t hue;
        uint8_t saturation;
    } layer_colors[WIRE_X_RPC_LEGACY_LAYERS];
};

struct wire_x_macro_slot {
    char name[WIRE_X_RPC_MACRO_NAME_MAX];
    char sequence[WIRE_X_RPC_MACRO_SEQUENCE_MAX];
};

#define WIRE_X_POWER_CONFIG_VERSION 2

struct wire_x_power_config {
    uint8_t version;
    uint8_t rgb_timeout_min;
    uint8_t display_timeout_min;
    bool display_on;
};

__weak int wire_x_display_set_label(uint8_t kind, uint8_t layer, uint8_t index,
                                    const char *text) {
    ARG_UNUSED(kind);
    ARG_UNUSED(layer);
    ARG_UNUSED(index);
    ARG_UNUSED(text);
    return -ENOTSUP;
}

__weak int wire_x_display_save_labels(void) {
    return 0;
}

__weak int wire_x_display_reset_labels(void) {
    return 0;
}

__weak int wire_x_display_test(uint8_t mode) {
    ARG_UNUSED(mode);
    return -ENOTSUP;
}

__weak void wire_x_display_toggle_status(void) {
}

__weak void wire_x_display_backlight_set(bool on) {
    ARG_UNUSED(on);
}

__weak bool wire_x_display_labels_dirty(void) {
    return false;
}

__weak int wire_x_display_settings_set(const char *name, size_t len, settings_read_cb read_cb,
                                       void *cb_arg) {
    ARG_UNUSED(name);
    ARG_UNUSED(len);
    ARG_UNUSED(read_cb);
    ARG_UNUSED(cb_arg);
    return -ENOENT;
}

__weak const char *wire_x_display_layer_label(uint8_t layer) {
    static const char *const defaults[WIRE_X_RPC_LAYERS] = {
        "LAYER 1", "LAYER 2", "LAYER 3", "LAYER 4", "LAYER 5", "DEVICE",
    };

    return layer < WIRE_X_RPC_LAYERS ? defaults[layer] : "";
}

__weak const char *wire_x_display_key_label(uint8_t layer, uint8_t key) {
    static const char *const defaults[WIRE_X_RPC_LAYERS][12] = {
        {"A", "B", "C", "LAYER", "E", "F", "G", "H", "I", "J", "K", "L"},
        {"1", "2", "3", "LAYER", "5", "6", "7", "8", "9", "0", "-", "="},
        {"!", "@", "#", "LAYER", "%", "^", "&", "*", "(", ")", "UP", "DN"},
        {"PLAY", "MUTE", "VDN", "LAYER", "NEXT", "PREV", "HOME", "END", "DEL", "PGUP", "PGDN", "TAB"},
        {"ESC", "SPACE", "F1", "LAYER", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10"},
        {"BLE1", "BLE2", "BLE3", "LAYER", "USB", "BLE", "RESET", "CLEAR", "RGB", "FX",
         "HUE", "BRT"},
    };

    return layer < WIRE_X_RPC_LAYERS && key < ARRAY_SIZE(defaults[0]) ? defaults[layer][key] : "";
}

__weak const char *wire_x_display_encoder_label(uint8_t layer, uint8_t encoder) {
    static const char *const defaults[WIRE_X_RPC_LAYERS][WIRE_X_RPC_ENCODERS] = {
        {"PGUP", "VOL"}, {"SCRL", "ZOOM"}, {"UNDO", "REDO"},
        {"BRT", "TRK"}, {"NAV", "APP"},  {"USB", "BLE"},
    };

    return layer < WIRE_X_RPC_LAYERS && encoder < WIRE_X_RPC_ENCODERS
               ? defaults[layer][encoder]
               : "";
}

#define KP_ENTRY(label, code) {.name = label, .kind = WIRE_X_BIND_KP, .param1 = code}

static const struct wire_x_keycode keycodes[] = {
    {.name = "NONE", .kind = WIRE_X_BIND_NONE},
    KP_ENTRY("A", A),
    KP_ENTRY("B", B),
    KP_ENTRY("C", C),
    KP_ENTRY("D", D),
    KP_ENTRY("E", E),
    KP_ENTRY("F", F),
    KP_ENTRY("G", G),
    KP_ENTRY("H", H),
    KP_ENTRY("I", I),
    KP_ENTRY("J", J),
    KP_ENTRY("K", K),
    KP_ENTRY("L", L),
    KP_ENTRY("M", M),
    KP_ENTRY("N", N),
    KP_ENTRY("O", O),
    KP_ENTRY("P", P),
    KP_ENTRY("Q", Q),
    KP_ENTRY("R", R),
    KP_ENTRY("S", S),
    KP_ENTRY("T", T),
    KP_ENTRY("U", U),
    KP_ENTRY("V", V),
    KP_ENTRY("W", W),
    KP_ENTRY("X", X),
    KP_ENTRY("Y", Y),
    KP_ENTRY("Z", Z),
    KP_ENTRY("N0", N0),
    KP_ENTRY("N1", N1),
    KP_ENTRY("N2", N2),
    KP_ENTRY("N3", N3),
    KP_ENTRY("N4", N4),
    KP_ENTRY("N5", N5),
    KP_ENTRY("N6", N6),
    KP_ENTRY("N7", N7),
    KP_ENTRY("N8", N8),
    KP_ENTRY("N9", N9),
    KP_ENTRY("EXCL", EXCL),
    KP_ENTRY("AT", AT),
    KP_ENTRY("HASH", HASH),
    KP_ENTRY("DLLR", DLLR),
    KP_ENTRY("PRCNT", PRCNT),
    KP_ENTRY("CARET", CARET),
    KP_ENTRY("AMPS", AMPS),
    KP_ENTRY("ASTRK", ASTRK),
    KP_ENTRY("LPAR", LPAR),
    KP_ENTRY("RPAR", RPAR),
    KP_ENTRY("MINUS", MINUS),
    KP_ENTRY("EQUAL", EQUAL),
    KP_ENTRY("PLUS", PLUS),
    KP_ENTRY("LBKT", LBKT),
    KP_ENTRY("RBKT", RBKT),
    KP_ENTRY("BSLH", BSLH),
    KP_ENTRY("SEMI", SEMI),
    KP_ENTRY("SQT", SQT),
    KP_ENTRY("GRAVE", GRAVE),
    KP_ENTRY("COMMA", COMMA),
    KP_ENTRY("DOT", DOT),
    KP_ENTRY("FSLH", FSLH),
    KP_ENTRY("BSPC", BSPC),
    KP_ENTRY("TAB", TAB),
    KP_ENTRY("ENTER", ENTER),
    KP_ENTRY("DEL", DEL),
    KP_ENTRY("ESC", ESC),
    KP_ENTRY("SPACE", SPACE),
    KP_ENTRY("CAPS", CAPS),
    KP_ENTRY("LCTRL", LCTRL),
    KP_ENTRY("LSHFT", LSHFT),
    KP_ENTRY("LALT", LALT),
    KP_ENTRY("LGUI", LGUI),
    KP_ENTRY("RCTRL", RCTRL),
    KP_ENTRY("RSHFT", RSHFT),
    KP_ENTRY("RALT", RALT),
    KP_ENTRY("RGUI", RGUI),
    KP_ENTRY("HOME", HOME),
    KP_ENTRY("END", END),
    KP_ENTRY("INS", INS),
    KP_ENTRY("LEFT", LEFT),
    KP_ENTRY("RIGHT", RIGHT),
    KP_ENTRY("UP", UP),
    KP_ENTRY("DOWN", DOWN),
    KP_ENTRY("PG_UP", PG_UP),
    KP_ENTRY("PG_DN", PG_DN),
    KP_ENTRY("PSCRN", PSCRN),
    KP_ENTRY("SLCK", SLCK),
    KP_ENTRY("PAUSE_BREAK", PAUSE_BREAK),
    KP_ENTRY("F1", F1),
    KP_ENTRY("F2", F2),
    KP_ENTRY("F3", F3),
    KP_ENTRY("F4", F4),
    KP_ENTRY("F5", F5),
    KP_ENTRY("F6", F6),
    KP_ENTRY("F7", F7),
    KP_ENTRY("F8", F8),
    KP_ENTRY("F9", F9),
    KP_ENTRY("F10", F10),
    KP_ENTRY("F11", F11),
    KP_ENTRY("F12", F12),
    KP_ENTRY("KP_NUM", KP_NUM),
    KP_ENTRY("KP_N0", KP_N0),
    KP_ENTRY("KP_N1", KP_N1),
    KP_ENTRY("KP_N2", KP_N2),
    KP_ENTRY("KP_N3", KP_N3),
    KP_ENTRY("KP_N4", KP_N4),
    KP_ENTRY("KP_N5", KP_N5),
    KP_ENTRY("KP_N6", KP_N6),
    KP_ENTRY("KP_N7", KP_N7),
    KP_ENTRY("KP_N8", KP_N8),
    KP_ENTRY("KP_N9", KP_N9),
    KP_ENTRY("KP_DOT", KP_DOT),
    KP_ENTRY("KP_ENTER", KP_ENTER),
    KP_ENTRY("KP_PLUS", KP_PLUS),
    KP_ENTRY("KP_MINUS", KP_MINUS),
    KP_ENTRY("KP_MULTIPLY", KP_MULTIPLY),
    KP_ENTRY("KP_DIVIDE", KP_DIVIDE),
    KP_ENTRY("KP_EQUAL", KP_EQUAL),
    KP_ENTRY("KP_COMMA", KP_COMMA),
    KP_ENTRY("KP_LPAR", KP_LPAR),
    KP_ENTRY("KP_RPAR", KP_RPAR),
    KP_ENTRY("KP_CLEAR", KP_CLEAR),
    KP_ENTRY("C_MUTE", C_MUTE),
    KP_ENTRY("C_VOL_UP", C_VOL_UP),
    KP_ENTRY("C_VOL_DN", C_VOL_DN),
    KP_ENTRY("C_PLAY", C_PLAY),
    KP_ENTRY("C_PAUSE", C_PAUSE),
    KP_ENTRY("C_RECORD", C_RECORD),
    KP_ENTRY("C_FAST_FORWARD", C_FAST_FORWARD),
    KP_ENTRY("C_REWIND", C_REWIND),
    KP_ENTRY("C_PREV", C_PREV),
    KP_ENTRY("C_NEXT", C_NEXT),
    KP_ENTRY("C_STOP", C_STOP),
    KP_ENTRY("C_EJECT", C_EJECT),
    KP_ENTRY("C_RANDOM_PLAY", C_RANDOM_PLAY),
    KP_ENTRY("C_REPEAT", C_REPEAT),
    KP_ENTRY("C_STOP_EJECT", C_STOP_EJECT),
    KP_ENTRY("C_PLAY_PAUSE", C_PLAY_PAUSE),
    KP_ENTRY("C_POWER", C_POWER),
    KP_ENTRY("C_RESET", C_RESET),
    KP_ENTRY("C_SLEEP", C_SLEEP),
    KP_ENTRY("C_SLEEP_MODE", C_SLEEP_MODE),
    KP_ENTRY("C_MENU", C_MENU),
    KP_ENTRY("C_MENU_SELECT", C_MENU_SELECT),
    KP_ENTRY("C_MENU_UP", C_MENU_UP),
    KP_ENTRY("C_MENU_DOWN", C_MENU_DOWN),
    KP_ENTRY("C_MENU_LEFT", C_MENU_LEFT),
    KP_ENTRY("C_MENU_RIGHT", C_MENU_RIGHT),
    KP_ENTRY("C_MENU_ESC", C_MENU_ESC),
    KP_ENTRY("C_MENU_INC", C_MENU_INC),
    KP_ENTRY("C_MENU_DEC", C_MENU_DEC),
    KP_ENTRY("C_SNAPSHOT", C_SNAPSHOT),
    KP_ENTRY("C_BASS_BOOST", C_BASS_BOOST),
    KP_ENTRY("C_MEDIA_STEP", C_MEDIA_STEP),
    KP_ENTRY("C_AL_WORD", C_AL_WORD),
    KP_ENTRY("C_AL_TEXT_EDITOR", C_AL_TEXT_EDITOR),
    KP_ENTRY("C_AL_SHEET", C_AL_SHEET),
    KP_ENTRY("C_AL_PRESENTATION", C_AL_PRESENTATION),
    KP_ENTRY("C_AL_MAIL", C_AL_MAIL),
    KP_ENTRY("C_AL_CALC", C_AL_CALC),
    KP_ENTRY("C_AL_MY_COMPUTER", C_AL_MY_COMPUTER),
    KP_ENTRY("C_AL_WWW", C_AL_WWW),
    KP_ENTRY("C_AL_LOCK", C_AL_LOCK),
    KP_ENTRY("C_AL_CONTROL_PANEL", C_AL_CONTROL_PANEL),
    KP_ENTRY("C_AL_NEXT_TASK", C_AL_NEXT_TASK),
    KP_ENTRY("C_AL_PREV_TASK", C_AL_PREV_TASK),
    KP_ENTRY("C_AL_FILES", C_AL_FILES),
    KP_ENTRY("C_AL_MUSIC", C_AL_MUSIC),
    KP_ENTRY("C_AL_MOVIES", C_AL_MOVIES),
    KP_ENTRY("C_AC_NEW", C_AC_NEW),
    KP_ENTRY("C_AC_OPEN", C_AC_OPEN),
    KP_ENTRY("C_AC_CLOSE", C_AC_CLOSE),
    KP_ENTRY("C_AC_EXIT", C_AC_EXIT),
    KP_ENTRY("C_AC_SAVE", C_AC_SAVE),
    KP_ENTRY("C_AC_PRINT", C_AC_PRINT),
    KP_ENTRY("C_AC_PROPS", C_AC_PROPS),
    KP_ENTRY("C_AC_UNDO", C_AC_UNDO),
    KP_ENTRY("C_AC_REDO", C_AC_REDO),
    KP_ENTRY("C_AC_COPY", C_AC_COPY),
    KP_ENTRY("C_AC_CUT", C_AC_CUT),
    KP_ENTRY("C_AC_PASTE", C_AC_PASTE),
    KP_ENTRY("C_AC_FIND", C_AC_FIND),
    KP_ENTRY("C_AC_SEARCH", C_AC_SEARCH),
    KP_ENTRY("C_AC_HOME", C_AC_HOME),
    KP_ENTRY("C_AC_ZOOM_IN", C_AC_ZOOM_IN),
    KP_ENTRY("C_AC_ZOOM_OUT", C_AC_ZOOM_OUT),
    KP_ENTRY("C_AC_ZOOM", C_AC_ZOOM),
    KP_ENTRY("C_AC_VIEW_TOGGLE", C_AC_VIEW_TOGGLE),
    KP_ENTRY("C_AC_SCROLL_UP", C_AC_SCROLL_UP),
    KP_ENTRY("C_AC_SCROLL_DOWN", C_AC_SCROLL_DOWN),
    KP_ENTRY("C_AC_BACK", C_AC_BACK),
    KP_ENTRY("C_AC_FORWARD", C_AC_FORWARD),
    KP_ENTRY("C_AC_STOP", C_AC_STOP),
    KP_ENTRY("C_AC_REFRESH", C_AC_REFRESH),
    KP_ENTRY("C_AC_BOOKMARKS", C_AC_BOOKMARKS),
    KP_ENTRY("C_AC_EDIT", C_AC_EDIT),
    KP_ENTRY("C_AC_CANCEL", C_AC_CANCEL),
    KP_ENTRY("C_AC_INS", C_AC_INS),
    KP_ENTRY("C_AC_DEL", C_AC_DEL),
    KP_ENTRY("C_AC_REPLY", C_AC_REPLY),
    KP_ENTRY("C_AC_FORWARD_MAIL", C_AC_FORWARD_MAIL),
    KP_ENTRY("C_AC_SEND", C_AC_SEND),
    KP_ENTRY("C_AC_DESKTOP_SHOW_ALL_WINDOWS", C_AC_DESKTOP_SHOW_ALL_WINDOWS),
    KP_ENTRY("C_AC_DESKTOP_SHOW_ALL_APPLICATIONS", C_AC_DESKTOP_SHOW_ALL_APPLICATIONS),
    KP_ENTRY("C_AC_NEXT_KEYBOARD_LAYOUT_SELECT", C_AC_NEXT_KEYBOARD_LAYOUT_SELECT),
    KP_ENTRY("C_BRI_UP", C_BRI_UP),
    KP_ENTRY("C_BRI_DN", C_BRI_DN),
    KP_ENTRY("C_BKLT_TOG", C_BKLT_TOG),
    KP_ENTRY("C_BRI_MIN", C_BRI_MIN),
    KP_ENTRY("C_BRI_MAX", C_BRI_MAX),
    KP_ENTRY("C_BRI_AUTO", C_BRI_AUTO),
    KP_ENTRY("K_APP", K_APP),
    KP_ENTRY("K_POWER", K_POWER),
    KP_ENTRY("K_EXEC", K_EXEC),
    KP_ENTRY("K_HELP", K_HELP),
    KP_ENTRY("K_MENU", K_MENU),
    KP_ENTRY("K_SELECT", K_SELECT),
    KP_ENTRY("K_STOP", K_STOP),
    KP_ENTRY("K_REDO", K_REDO),
    KP_ENTRY("K_UNDO", K_UNDO),
    KP_ENTRY("K_CUT", K_CUT),
    KP_ENTRY("K_COPY", K_COPY),
    KP_ENTRY("K_PASTE", K_PASTE),
    KP_ENTRY("K_FIND", K_FIND),
    KP_ENTRY("K_MUTE", K_MUTE),
    KP_ENTRY("K_VOL_UP", K_VOL_UP),
    KP_ENTRY("K_VOL_DN", K_VOL_DN),
    KP_ENTRY("K_PLAY_PAUSE", K_PLAY_PAUSE),
    KP_ENTRY("K_PREV", K_PREV),
    KP_ENTRY("K_NEXT", K_NEXT),
    KP_ENTRY("K_EJECT", K_EJECT),
    KP_ENTRY("K_WWW", K_WWW),
    KP_ENTRY("K_BACK", K_BACK),
    KP_ENTRY("K_FORWARD", K_FORWARD),
    KP_ENTRY("K_REFRESH", K_REFRESH),
    KP_ENTRY("K_CALC", K_CALC),
    KP_ENTRY("K_SLEEP", K_SLEEP),
    KP_ENTRY("K_LOCK", K_LOCK),
    KP_ENTRY("SYSTEM_POWER", SYSTEM_POWER),
    KP_ENTRY("SYSTEM_SLEEP", SYSTEM_SLEEP),
    KP_ENTRY("SYSTEM_WAKE_UP", SYSTEM_WAKE_UP),
};

static const struct device *const rpc_uart = DEVICE_DT_GET(WIRE_X_RPC_UART_NODE);
static const uint32_t encoder_default_params[WIRE_X_RPC_ENCODERS][2] = {
    {PG_DN, PG_UP},
    {C_VOL_DN, C_VOL_UP},
};
static uint32_t encoder_params[WIRE_X_RPC_ENCODERS][2] = {
    {PG_DN, PG_UP},
    {C_VOL_DN, C_VOL_UP},
};
static int8_t encoder_macro_bindings[WIRE_X_RPC_ENCODERS][2] = {
    {WIRE_X_RPC_NO_MACRO, WIRE_X_RPC_NO_MACRO},
    {WIRE_X_RPC_NO_MACRO, WIRE_X_RPC_NO_MACRO},
};
static uint8_t encoder_sensitivity[WIRE_X_RPC_ENCODERS] = {1, 1};
static const struct wire_x_rgb_config rgb_default_config = {
    .hue = 0,
    .saturation = 100,
    .brightness = 25,
    .effect = 3,
    .on = true,
    .reactive = false,
    .layer_mode = true,
    .layer_colors = {
        {.hue = 0, .saturation = 0},
        {.hue = 0, .saturation = 100},
        {.hue = 120, .saturation = 100},
        {.hue = 240, .saturation = 100},
        {.hue = 45, .saturation = 100},
        {.hue = 285, .saturation = 100},
    },
};
static struct wire_x_rgb_config rgb_config = {
    .hue = 0,
    .saturation = 100,
    .brightness = 25,
    .effect = 3,
    .on = true,
    .reactive = false,
    .layer_mode = true,
    .layer_colors = {
        {.hue = 0, .saturation = 0},
        {.hue = 0, .saturation = 100},
        {.hue = 120, .saturation = 100},
        {.hue = 240, .saturation = 100},
        {.hue = 45, .saturation = 100},
        {.hue = 285, .saturation = 100},
    },
};
static const struct wire_x_power_config power_default_config = {
    .version = WIRE_X_POWER_CONFIG_VERSION,
    .rgb_timeout_min = 10,
    .display_timeout_min = 10,
    .display_on = true,
};
static struct wire_x_power_config power_config = {
    .version = WIRE_X_POWER_CONFIG_VERSION,
    .rgb_timeout_min = 10,
    .display_timeout_min = 10,
    .display_on = true,
};
static bool encoder_params_dirty;
static bool encoder_macro_bindings_dirty;
static bool encoder_sensitivity_dirty;
static bool rgb_config_dirty;
static bool power_config_dirty;
static bool rgb_layer_indicator_active;
static bool rgb_timed_out;
static bool display_timed_out;
static bool rgb_ble_feedback_active;
static bool rgb_ble_feedback_blink;
static bool rgb_ble_feedback_on;
static uint8_t rgb_ble_feedback_profile;
static bool macro_bindings_dirty;
static bool macro_slot_dirty[WIRE_X_RPC_MACROS];
static struct wire_x_macro_slot macro_slots[WIRE_X_RPC_MACROS];
static bool tac_w_factory_migration_done;
static char wire_x_device_name[] = WIRE_X_DEVICE_NAME;

#define WIRE_X_MACRO_EMPTY_ROW                                                                    \
    {                                                                                              \
        WIRE_X_RPC_NO_MACRO, WIRE_X_RPC_NO_MACRO, WIRE_X_RPC_NO_MACRO, WIRE_X_RPC_NO_MACRO,       \
            WIRE_X_RPC_NO_MACRO, WIRE_X_RPC_NO_MACRO, WIRE_X_RPC_NO_MACRO, WIRE_X_RPC_NO_MACRO,   \
            WIRE_X_RPC_NO_MACRO, WIRE_X_RPC_NO_MACRO, WIRE_X_RPC_NO_MACRO, WIRE_X_RPC_NO_MACRO,   \
            WIRE_X_RPC_NO_MACRO, WIRE_X_RPC_NO_MACRO                                              \
    }

static int8_t macro_bindings[WIRE_X_RPC_LAYERS][WIRE_X_RPC_POSITIONS] = {
    WIRE_X_MACRO_EMPTY_ROW, WIRE_X_MACRO_EMPTY_ROW, WIRE_X_MACRO_EMPTY_ROW,
    WIRE_X_MACRO_EMPTY_ROW, WIRE_X_MACRO_EMPTY_ROW, WIRE_X_MACRO_EMPTY_ROW,
};

#undef WIRE_X_MACRO_EMPTY_ROW

#if IS_ENABLED(CONFIG_BT_ZEPHYR_NUS)
static uint8_t ble_tx_chunk[WIRE_X_RPC_BLE_CHUNK];
static size_t ble_tx_len;

static bool rpc_ble_flush(const struct wire_x_rpc_target *target) {
    if (ble_tx_len == 0) {
        return true;
    }

    for (int attempt = 0; attempt < WIRE_X_RPC_BLE_SEND_RETRIES; attempt++) {
        int ret = bt_nus_send(target->conn, ble_tx_chunk, ble_tx_len);

        if (ret >= 0) {
            ble_tx_len = 0;
            k_sleep(K_MSEC(WIRE_X_RPC_BLE_TX_GAP_MS));
            return true;
        }

        if (ret != -ENOMEM && ret != -EAGAIN) {
            ble_tx_len = 0;
            return false;
        }

        k_sleep(K_MSEC(WIRE_X_RPC_BLE_RETRY_MS));
    }

    ble_tx_len = 0;
    return false;
}
#endif

static void rpc_puts(const struct wire_x_rpc_target *target, const char *text) {
#if IS_ENABLED(CONFIG_BT_ZEPHYR_NUS)
    if (target->transport == WIRE_X_RPC_TRANSPORT_BLE) {
        while (*text != '\0') {
            char c = *text++;
            ble_tx_chunk[ble_tx_len++] = c;

            if (ble_tx_len == sizeof(ble_tx_chunk) || c == '\n') {
                if (!rpc_ble_flush(target)) {
                    return;
                }
            }
        }

        return;
    }
#else
    ARG_UNUSED(target);
#endif

    while (*text != '\0') {
        uart_poll_out(rpc_uart, *text++);
    }
}

static void rpc_reply_ok(const struct wire_x_rpc_target *target, int id, const char *extra) {
    char msg[160];

    if (extra != NULL && extra[0] != '\0') {
        snprintk(msg, sizeof(msg), "{\"id\":%d,\"ok\":true,%s}\n", id, extra);
    } else {
        snprintk(msg, sizeof(msg), "{\"id\":%d,\"ok\":true}\n", id);
    }

    rpc_puts(target, msg);
}

static void rpc_reply_error(const struct wire_x_rpc_target *target, int id, const char *error) {
    char msg[192];

    snprintk(msg, sizeof(msg), "{\"id\":%d,\"ok\":false,\"error\":\"%s\"}\n", id, error);
    rpc_puts(target, msg);
}

static void rpc_puts_uart_event(const char *text) {
    if (!device_is_ready(rpc_uart)) {
        return;
    }

    while (*text != '\0') {
        uart_poll_out(rpc_uart, *text++);
    }
}

static void rpc_notify_matrix_event(uint8_t position, bool pressed) {
    if (position >= WIRE_X_RPC_POSITIONS) {
        return;
    }

    char msg[96];
    uint8_t layer = MIN(zmk_keymap_highest_layer_active(), WIRE_X_RPC_LAYERS - 1);
    snprintk(msg, sizeof(msg),
             "{\"event\":\"matrix\",\"position\":%u,\"pressed\":%s,\"layer\":%u}\n", position,
             pressed ? "true" : "false", layer);
    rpc_puts_uart_event(msg);
}

static int sensor_event_delta(const struct zmk_sensor_event *event) {
    if (event == NULL || event->channel_data_size == 0) {
        return 0;
    }

    struct sensor_value value = event->channel_data[0].value;
    return value.val1 == 0 ? value.val2 : value.val1;
}

static void rpc_notify_encoder_event(uint8_t encoder, int delta) {
    if (encoder >= WIRE_X_RPC_ENCODERS || delta == 0) {
        return;
    }

    char msg[112];
    uint8_t layer = MIN(zmk_keymap_highest_layer_active(), WIRE_X_RPC_LAYERS - 1);
    snprintk(msg, sizeof(msg),
             "{\"event\":\"encoder\",\"encoder\":%u,\"direction\":\"%s\",\"value\":%d,"
             "\"layer\":%u}\n",
             encoder, delta > 0 ? "CW" : "CCW", delta, layer);
    rpc_puts_uart_event(msg);
}

static const char *skip_ws(const char *p) {
    while (*p != '\0' && isspace((unsigned char)*p)) {
        p++;
    }

    return p;
}

static bool json_get_int(const char *json, const char *name, int *value) {
    const char *p = strstr(json, name);
    if (p == NULL) {
        return false;
    }

    p = strchr(p, ':');
    if (p == NULL) {
        return false;
    }

    p = skip_ws(p + 1);
    bool neg = false;
    if (*p == '-') {
        neg = true;
        p++;
    }

    if (!isdigit((unsigned char)*p)) {
        return false;
    }

    int parsed = 0;
    while (isdigit((unsigned char)*p)) {
        parsed = (parsed * 10) + (*p - '0');
        p++;
    }

    *value = neg ? -parsed : parsed;
    return true;
}

static bool json_get_bool(const char *json, const char *name, bool *value) {
    const char *p = strstr(json, name);
    if (p == NULL) {
        return false;
    }

    p = strchr(p, ':');
    if (p == NULL) {
        return false;
    }

    p = skip_ws(p + 1);
    if (strncmp(p, "true", 4) == 0) {
        *value = true;
        return true;
    }
    if (strncmp(p, "false", 5) == 0) {
        *value = false;
        return true;
    }

    return false;
}

static bool json_get_string(const char *json, const char *name, char *out, size_t out_len) {
    const char *p = strstr(json, name);
    if (p == NULL || out_len == 0) {
        return false;
    }

    p = strchr(p, ':');
    if (p == NULL) {
        return false;
    }

    p = skip_ws(p + 1);
    if (*p != '"') {
        return false;
    }
    p++;

    size_t i = 0;
    while (*p != '\0' && *p != '"') {
        if (*p == '\\' && p[1] != '\0') {
            p++;
        }

        if (i + 1 < out_len) {
            out[i++] = *p;
        }
        p++;
    }

    if (*p != '"') {
        return false;
    }

    out[i] = '\0';
    return true;
}

static void rpc_put_json_string(const struct wire_x_rpc_target *target, const char *text) {
    rpc_puts(target, "\"");
    while (*text != '\0') {
        char c = *text++;
        if (c == '"' || c == '\\') {
            char escaped[3] = {'\\', c, '\0'};
            rpc_puts(target, escaped);
        } else if ((unsigned char)c >= 0x20) {
            char out[2] = {c, '\0'};
            rpc_puts(target, out);
        }
    }
    rpc_puts(target, "\"");
}

static void rpc_copy_string(char *dst, size_t dst_len, const char *src) {
    if (dst_len == 0) {
        return;
    }

    memset(dst, 0, dst_len);
    if (src == NULL) {
        return;
    }

    for (size_t i = 0; src[i] != '\0' && i + 1 < dst_len; i++) {
        dst[i] = src[i];
    }
}

static bool macro_keycode_to_index(const char *name, int *index) {
    if (name == NULL || name[0] != 'M' || !isdigit((unsigned char)name[1])) {
        return false;
    }

    int parsed = 0;
    for (const char *p = name + 1; *p != '\0'; p++) {
        if (!isdigit((unsigned char)*p)) {
            return false;
        }
        parsed = (parsed * 10) + (*p - '0');
    }

    if (parsed < 0 || parsed >= WIRE_X_RPC_MACROS) {
        return false;
    }

    *index = parsed;
    return true;
}

static void macro_default_name(int index, char *out, size_t out_len) {
    snprintk(out, out_len, "M%d", index);
}

static const char *macro_name_or_default(int index, char *fallback, size_t fallback_len) {
    if (macro_slots[index].name[0] != '\0') {
        return macro_slots[index].name;
    }

    macro_default_name(index, fallback, fallback_len);
    return fallback;
}

static const struct wire_x_keycode *find_keycode(const char *name) {
    for (size_t i = 0; i < ARRAY_SIZE(keycodes); i++) {
        if (strcmp(name, keycodes[i].name) == 0) {
            return &keycodes[i];
        }
    }

    return NULL;
}

static const char *keycode_name_from_param(uint32_t param) {
    for (size_t i = 0; i < ARRAY_SIZE(keycodes); i++) {
        if (keycodes[i].kind == WIRE_X_BIND_KP && keycodes[i].param1 == param) {
            return keycodes[i].name;
        }
    }

    return NULL;
}

static const char *keycode_name_from_binding(const struct zmk_behavior_binding *binding) {
    if (binding == NULL || binding->behavior_dev == NULL) {
        return WIRE_X_UNSUPPORTED_BINDING;
    }

    if (strcmp(binding->behavior_dev, DEVICE_DT_NAME(DT_NODELABEL(none))) == 0) {
        return "NONE";
    }

    if (strcmp(binding->behavior_dev, DEVICE_DT_NAME(DT_NODELABEL(kp))) == 0) {
        const char *keycode = keycode_name_from_param(binding->param1);
        return keycode != NULL ? keycode : WIRE_X_UNSUPPORTED_BINDING;
    }

    return WIRE_X_UNSUPPORTED_BINDING;
}

static int binding_from_keycode(const char *name, struct zmk_behavior_binding *binding) {
    const struct wire_x_keycode *keycode = find_keycode(name);
    if (keycode == NULL) {
        return -EINVAL;
    }

    switch (keycode->kind) {
    case WIRE_X_BIND_NONE:
        *binding = (struct zmk_behavior_binding){
            .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(none)),
        };
        return 0;

    case WIRE_X_BIND_KP:
        *binding = (struct zmk_behavior_binding){
            .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(kp)),
            .param1 = keycode->param1,
        };
        return 0;

    }

    return -EINVAL;
}

static int apply_keycode(int layer, int position, const char *keycode_name) {
    if (layer < 0 || layer >= WIRE_X_RPC_LAYERS || position < 0 || position >= WIRE_X_RPC_POSITIONS) {
        return -EINVAL;
    }

    if (position == WIRE_X_LAYER_KEY_POSITION || layer == WIRE_X_DEVICE_LAYER_INDEX ||
        strcmp(keycode_name, WIRE_X_KEEP_BINDING) == 0) {
        return 0;
    }

    int macro_index;
    if (macro_keycode_to_index(keycode_name, &macro_index)) {
        struct zmk_behavior_binding none_binding = {
            .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(none)),
        };
        int ret = zmk_keymap_set_layer_binding_at_idx(layer, position, none_binding);
        if (ret >= 0) {
            if (macro_bindings[layer][position] != macro_index) {
                macro_bindings[layer][position] = macro_index;
                macro_bindings_dirty = true;
            }
        }
        return ret;
    }

    struct zmk_behavior_binding binding;
    int ret = binding_from_keycode(keycode_name, &binding);
    if (ret < 0) {
        return ret;
    }

    ret = zmk_behavior_validate_binding(&binding);
    if (ret < 0) {
        return ret;
    }

    ret = zmk_keymap_set_layer_binding_at_idx(layer, position, binding);
    if (ret >= 0) {
        if (macro_bindings[layer][position] != WIRE_X_RPC_NO_MACRO) {
            macro_bindings[layer][position] = WIRE_X_RPC_NO_MACRO;
            macro_bindings_dirty = true;
        }
    }

    return ret;
}

static int handle_set_key(const struct wire_x_rpc_target *target, int id, const char *line) {
    int layer;
    int position;
    char keycode[32];

    if (!json_get_int(line, "\"layer\"", &layer) ||
        !json_get_int(line, "\"position\"", &position) ||
        !json_get_string(line, "\"keycode\"", keycode, sizeof(keycode))) {
        rpc_reply_error(target, id, "bad set_key request");
        return -EINVAL;
    }

    if (layer == WIRE_X_DEVICE_LAYER_INDEX) {
        rpc_reply_error(target, id, "device layer is locked");
        return -EACCES;
    }

    int ret = apply_keycode(layer, position, keycode);
    if (ret < 0) {
        rpc_reply_error(target, id, "unsupported keycode or location");
        return ret;
    }

    rpc_reply_ok(target, id, "\"cmd\":\"set_key\"");
    return 0;
}

static int apply_encoder_binding(int encoder) {
    struct zmk_behavior_binding binding = {
        .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(inc_dec_kp)),
        .param1 = encoder_params[encoder][0],
        .param2 = encoder_params[encoder][1],
    };

    for (int layer = 0; layer < WIRE_X_DEVICE_LAYER_INDEX; layer++) {
        int ret = zmk_keymap_set_sensor_binding_at_idx(layer, encoder, binding);
        if (ret < 0) {
            return ret;
        }
    }

    return 0;
}

static int apply_all_encoder_bindings(void) {
    for (int encoder = 0; encoder < WIRE_X_RPC_ENCODERS; encoder++) {
        int ret = apply_encoder_binding(encoder);
        if (ret < 0) {
            return ret;
        }
    }

    return 0;
}

static int apply_encoder_keycode(int encoder, const char *direction, const char *keycode_name) {
    if (encoder < 0 || encoder >= WIRE_X_RPC_ENCODERS) {
        return -EINVAL;
    }

    int direction_idx;
    if (strcmp(direction, "ccw") == 0) {
        direction_idx = 0;
    } else if (strcmp(direction, "cw") == 0) {
        direction_idx = 1;
    } else {
        return -EINVAL;
    }

    int macro_index;
    if (macro_keycode_to_index(keycode_name, &macro_index)) {
        encoder_macro_bindings[encoder][direction_idx] = macro_index;
        encoder_params[encoder][direction_idx] = 0;
        encoder_macro_bindings_dirty = true;
        encoder_params_dirty = true;
        return apply_encoder_binding(encoder);
    }

    const struct wire_x_keycode *keycode = find_keycode(keycode_name);
    if (keycode == NULL || keycode->kind != WIRE_X_BIND_KP) {
        return -EINVAL;
    }

    encoder_macro_bindings[encoder][direction_idx] = WIRE_X_RPC_NO_MACRO;
    encoder_params[encoder][direction_idx] = keycode->param1;
    encoder_macro_bindings_dirty = true;
    encoder_params_dirty = true;
    return apply_encoder_binding(encoder);
}

static int handle_set_encoder(const struct wire_x_rpc_target *target, int id, const char *line) {
    int encoder;
    char direction[8];
    char keycode[32];

    if (!json_get_int(line, "\"encoder\"", &encoder) ||
        !json_get_string(line, "\"direction\"", direction, sizeof(direction)) ||
        !json_get_string(line, "\"keycode\"", keycode, sizeof(keycode))) {
        rpc_reply_error(target, id, "bad set_encoder request");
        return -EINVAL;
    }

    int ret = apply_encoder_keycode(encoder, direction, keycode);
    if (ret < 0) {
        rpc_reply_error(target, id, "unsupported encoder mapping");
        return ret;
    }

    rpc_reply_ok(target, id, "\"cmd\":\"set_encoder\"");
    return 0;
}

static int handle_set_encoder_sensitivity(const struct wire_x_rpc_target *target, int id,
                                          const char *line) {
    int encoder;
    int sensitivity;

    if (!json_get_int(line, "\"encoder\"", &encoder) ||
        !json_get_int(line, "\"sensitivity\"", &sensitivity)) {
        rpc_reply_error(target, id, "bad encoder sensitivity request");
        return -EINVAL;
    }

    if (encoder < 0 || encoder >= WIRE_X_RPC_ENCODERS ||
        sensitivity < WIRE_X_ENCODER_SENSITIVITY_MIN ||
        sensitivity > WIRE_X_ENCODER_SENSITIVITY_MAX) {
        rpc_reply_error(target, id, "unsupported encoder sensitivity");
        return -EINVAL;
    }

    encoder_sensitivity[encoder] = sensitivity;
    encoder_sensitivity_dirty = true;
    rpc_reply_ok(target, id, "\"cmd\":\"set_encoder_sensitivity\"");
    return 0;
}

static int handle_set_macro(const struct wire_x_rpc_target *target, int id, const char *line) {
    int index;
    char name[WIRE_X_RPC_MACRO_NAME_MAX];
    char sequence[WIRE_X_RPC_MACRO_SEQUENCE_MAX];

    if (!json_get_int(line, "\"index\"", &index) ||
        !json_get_string(line, "\"sequence\"", sequence, sizeof(sequence))) {
        rpc_reply_error(target, id, "bad set_macro request");
        return -EINVAL;
    }

    if (index < 0 || index >= WIRE_X_RPC_MACROS) {
        rpc_reply_error(target, id, "unsupported macro index");
        return -EINVAL;
    }

    if (!json_get_string(line, "\"name\"", name, sizeof(name)) || name[0] == '\0') {
        macro_default_name(index, name, sizeof(name));
    }

    if (strcmp(macro_slots[index].name, name) != 0 ||
        strcmp(macro_slots[index].sequence, sequence) != 0) {
        rpc_copy_string(macro_slots[index].name, sizeof(macro_slots[index].name), name);
        rpc_copy_string(macro_slots[index].sequence, sizeof(macro_slots[index].sequence), sequence);
        macro_slot_dirty[index] = true;
    }

    rpc_reply_ok(target, id, "\"cmd\":\"set_macro\"");
    return 0;
}

static int handle_set_display_label(const struct wire_x_rpc_target *target, int id,
                                    const char *line) {
    int layer;
    int index = 0;
    char kind_name[16];
    char text[16];

    if (!json_get_int(line, "\"layer\"", &layer) ||
        !json_get_string(line, "\"target\"", kind_name, sizeof(kind_name)) ||
        !json_get_string(line, "\"text\"", text, sizeof(text))) {
        rpc_reply_error(target, id, "bad display label request");
        return -EINVAL;
    }

    uint8_t kind;
    if (strcmp(kind_name, "layer") == 0) {
        kind = WIRE_X_DISPLAY_LABEL_LAYER;
    } else if (strcmp(kind_name, "key") == 0) {
        kind = WIRE_X_DISPLAY_LABEL_KEY;
        if (!json_get_int(line, "\"index\"", &index)) {
            rpc_reply_error(target, id, "missing display key index");
            return -EINVAL;
        }
    } else if (strcmp(kind_name, "encoder") == 0) {
        kind = WIRE_X_DISPLAY_LABEL_ENCODER;
        if (!json_get_int(line, "\"index\"", &index)) {
            rpc_reply_error(target, id, "missing display encoder index");
            return -EINVAL;
        }
    } else {
        rpc_reply_error(target, id, "unsupported display label target");
        return -EINVAL;
    }

    int ret = wire_x_display_set_label(kind, (uint8_t)layer, (uint8_t)index, text);
    if (ret < 0) {
        rpc_reply_error(target, id,
                        ret == -EACCES ? "device layer display is locked"
                                       : "unsupported display label");
        return ret;
    }

    rpc_reply_ok(target, id, "\"cmd\":\"set_display_label\"");
    return 0;
}

static int handle_display_test(const struct wire_x_rpc_target *target, int id, const char *line) {
    char mode_name[16];
    int mode = 0;

    if (json_get_string(line, "\"mode\"", mode_name, sizeof(mode_name))) {
        if (strcmp(mode_name, "reset") == 0) {
            mode = 0;
        } else if (strcmp(mode_name, "red") == 0) {
            mode = 1;
        } else if (strcmp(mode_name, "green") == 0) {
            mode = 2;
        } else if (strcmp(mode_name, "blue") == 0) {
            mode = 3;
        } else {
            rpc_reply_error(target, id, "unsupported display test");
            return -EINVAL;
        }
    } else if (!json_get_int(line, "\"mode\"", &mode)) {
        rpc_reply_error(target, id, "bad display test request");
        return -EINVAL;
    }

    if (mode < 0 || mode > 3) {
        rpc_reply_error(target, id, "unsupported display test");
        return -EINVAL;
    }

    int ret = wire_x_display_test((uint8_t)mode);
    if (ret < 0) {
        rpc_reply_error(target, id, "display test unavailable");
        return ret;
    }

    rpc_reply_ok(target, id, "\"cmd\":\"display_test\"");
    return 0;
}

static bool macro_delay_from_token(const char *token, uint32_t *delay) {
    if (strncmp(token, "DLY(", 4) != 0) {
        return false;
    }

    const char *p = token + 4;
    if (!isdigit((unsigned char)*p)) {
        return false;
    }

    uint32_t parsed = 0;
    while (isdigit((unsigned char)*p)) {
        parsed = (parsed * 10U) + (uint32_t)(*p - '0');
        p++;
    }

    if (*p != ')' || p[1] != '\0') {
        return false;
    }

    *delay = CLAMP(parsed, 10U, 5000U);
    return true;
}

static int queue_wait(const struct zmk_behavior_binding_event *event, uint32_t wait) {
    const struct zmk_behavior_binding none_binding = {
        .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(none)),
    };

    return zmk_behavior_queue_add(event, none_binding, false, wait);
}

static int key_binding_from_token(const char *token, struct zmk_behavior_binding *binding) {
    const struct wire_x_keycode *keycode = find_keycode(token);
    if (keycode == NULL || keycode->kind != WIRE_X_BIND_KP) {
        return -EINVAL;
    }

    binding->behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(kp));
    binding->param1 = keycode->param1;
    binding->param2 = 0;
    return 0;
}

static int queue_macro_token(const struct zmk_behavior_binding_event *event, const char *token) {
    uint32_t delay;
    if (macro_delay_from_token(token, &delay)) {
        return queue_wait(event, delay);
    }

    char token_buf[72];
    rpc_copy_string(token_buf, sizeof(token_buf), token);

    struct zmk_behavior_binding bindings[8];
    size_t binding_count = 0;
    char *saveptr;
    char *part = strtok_r(token_buf, "+", &saveptr);

    while (part != NULL && binding_count < ARRAY_SIZE(bindings)) {
        while (*part != '\0' && isspace((unsigned char)*part)) {
            part++;
        }

        if (*part != '\0' && key_binding_from_token(part, &bindings[binding_count]) == 0) {
            binding_count++;
        }

        part = strtok_r(NULL, "+", &saveptr);
    }

    if (binding_count == 0) {
        return -EINVAL;
    }

    for (size_t i = 0; i < binding_count; i++) {
        int ret = zmk_behavior_queue_add(event, bindings[i], true, 4);
        if (ret < 0) {
            return ret;
        }
    }

    for (size_t i = binding_count; i > 0; i--) {
        uint32_t wait = i == 1 ? 16 : 4;
        int ret = zmk_behavior_queue_add(event, bindings[i - 1], false, wait);
        if (ret < 0) {
            return ret;
        }
    }

    return 0;
}

static void play_macro_slot(int index, const struct zmk_behavior_binding_event *event) {
    if (index < 0 || index >= WIRE_X_RPC_MACROS || macro_slots[index].sequence[0] == '\0') {
        return;
    }

    const char *p = macro_slots[index].sequence;
    int token_count = 0;
    while (*p != '\0' && token_count < 96) {
        while (*p != '\0' && (isspace((unsigned char)*p) || *p == ',')) {
            p++;
        }

        if (*p == '\0') {
            break;
        }

        char token[72];
        size_t i = 0;
        while (*p != '\0' && !isspace((unsigned char)*p) && *p != ',') {
            if (i + 1 < sizeof(token)) {
                token[i++] = *p;
            }
            p++;
        }
        token[i] = '\0';

        if (token[0] != '\0') {
            (void)queue_macro_token(event, token);
            token_count++;
        }
    }
}

static struct zmk_led_hsb rgb_display_color(void) {
    struct zmk_led_hsb color = {
        .h = rgb_config.hue,
        .s = rgb_config.saturation,
        .b = rgb_config.brightness,
    };

    if (rgb_ble_feedback_active) {
        static const uint16_t profile_hues[WIRE_X_BLE_PROFILE_COUNT] = {220, 180, 275};
        color.h = profile_hues[MIN(rgb_ble_feedback_profile, WIRE_X_BLE_PROFILE_COUNT - 1)];
        color.s = WIRE_X_RGB_BLE_SAT;
        color.b = MAX(rgb_config.brightness, WIRE_X_RGB_BLE_BRT_MIN);
        if (rgb_ble_feedback_blink && !rgb_ble_feedback_on) {
            color.b = 0;
        }
        return color;
    }

    if (!rgb_config.layer_mode || !rgb_layer_indicator_active) {
        return color;
    }

    uint8_t layer = MIN(zmk_keymap_highest_layer_active(), WIRE_X_RPC_LAYERS - 1);
    color.h = rgb_config.layer_colors[layer].hue;
    color.s = rgb_config.layer_colors[layer].saturation;
    return color;
}

static int apply_rgb_color(void) {
    return zmk_rgb_underglow_set_hsb(rgb_display_color());
}

static void restore_rgb_after_ble_feedback(struct k_work *work);
static void blink_rgb_for_ble_profile(struct k_work *work);
static void timeout_ble_profile_feedback(struct k_work *work);

K_WORK_DELAYABLE_DEFINE(wire_x_rgb_ble_restore_work, restore_rgb_after_ble_feedback);
K_WORK_DELAYABLE_DEFINE(wire_x_rgb_ble_blink_work, blink_rgb_for_ble_profile);
K_WORK_DELAYABLE_DEFINE(wire_x_rgb_ble_timeout_work, timeout_ble_profile_feedback);

static int apply_rgb_config(void) {
    rgb_layer_indicator_active = false;
    rgb_ble_feedback_active = false;
    rgb_ble_feedback_blink = false;
    rgb_ble_feedback_on = false;
    (void)k_work_cancel_delayable(&wire_x_rgb_ble_restore_work);
    (void)k_work_cancel_delayable(&wire_x_rgb_ble_blink_work);
    (void)k_work_cancel_delayable(&wire_x_rgb_ble_timeout_work);
    int ret = apply_rgb_color();
    if (ret < 0) {
        return ret;
    }

    ret = zmk_rgb_underglow_select_effect(rgb_config.effect);
    if (ret < 0) {
        return ret;
    }

    return rgb_config.on ? zmk_rgb_underglow_on() : zmk_rgb_underglow_off();
}

static void restore_rgb_after_ble_feedback(struct k_work *work) {
    ARG_UNUSED(work);

    rgb_ble_feedback_active = false;
    rgb_ble_feedback_blink = false;
    rgb_ble_feedback_on = false;

    (void)k_work_cancel_delayable(&wire_x_rgb_ble_timeout_work);

    if (rgb_config.on && !rgb_timed_out) {
        (void)apply_rgb_color();
        (void)zmk_rgb_underglow_on();
    } else {
        (void)zmk_rgb_underglow_off();
    }
}

static void apply_ble_feedback_rgb(void) {
    (void)apply_rgb_color();
    (void)zmk_rgb_underglow_on();
}

static void start_ble_connected_glow(uint8_t profile) {
    rgb_ble_feedback_profile = profile;
    rgb_ble_feedback_active = true;
    rgb_ble_feedback_blink = false;
    rgb_ble_feedback_on = true;

    (void)k_work_cancel_delayable(&wire_x_rgb_ble_blink_work);
    (void)k_work_cancel_delayable(&wire_x_rgb_ble_timeout_work);
#if IS_ENABLED(CONFIG_ZMK_BLE)
    (void)zmk_ble_stop_advertising_window();
#endif
    apply_ble_feedback_rgb();
    (void)k_work_reschedule(&wire_x_rgb_ble_restore_work, K_MSEC(WIRE_X_RGB_BLE_GLOW_MS));
}

static void start_ble_pending_blink(uint8_t profile) {
    rgb_ble_feedback_profile = profile;
    rgb_ble_feedback_active = true;
    rgb_ble_feedback_blink = true;
    rgb_ble_feedback_on = true;

    (void)k_work_cancel_delayable(&wire_x_rgb_ble_restore_work);
    apply_ble_feedback_rgb();
    (void)k_work_reschedule(&wire_x_rgb_ble_blink_work, K_MSEC(WIRE_X_RGB_BLE_BLINK_MS));
    (void)k_work_reschedule(&wire_x_rgb_ble_timeout_work,
                            K_MSEC(WIRE_X_BLE_ADVERTISING_TIMEOUT_MS));
}

static void blink_rgb_for_ble_profile(struct k_work *work) {
    ARG_UNUSED(work);

    if (!rgb_ble_feedback_active || !rgb_ble_feedback_blink) {
        return;
    }

#if IS_ENABLED(CONFIG_ZMK_BLE)
    if (zmk_ble_profile_is_connected(rgb_ble_feedback_profile)) {
        start_ble_connected_glow(rgb_ble_feedback_profile);
        return;
    }
#endif

    rgb_ble_feedback_on = !rgb_ble_feedback_on;
    apply_ble_feedback_rgb();
    (void)k_work_reschedule(&wire_x_rgb_ble_blink_work, K_MSEC(WIRE_X_RGB_BLE_BLINK_MS));
}

static void timeout_ble_profile_feedback(struct k_work *work) {
    ARG_UNUSED(work);

    if (!rgb_ble_feedback_active || !rgb_ble_feedback_blink) {
        return;
    }

#if IS_ENABLED(CONFIG_ZMK_BLE)
    if (zmk_ble_profile_is_connected(rgb_ble_feedback_profile)) {
        start_ble_connected_glow(rgb_ble_feedback_profile);
        return;
    }

    (void)zmk_ble_stop_advertising_window();
#endif

    restore_rgb_after_ble_feedback(work);
}

static void rgb_timeout_work_cb(struct k_work *work);
static void display_timeout_work_cb(struct k_work *work);

K_WORK_DELAYABLE_DEFINE(wire_x_rgb_timeout_work, rgb_timeout_work_cb);
K_WORK_DELAYABLE_DEFINE(wire_x_display_timeout_work, display_timeout_work_cb);

static k_timeout_t power_timeout_minutes(uint8_t minutes) {
    return K_SECONDS((uint32_t)CLAMP(minutes, 1, 30) * 60U);
}

static void schedule_power_timeouts(void) {
    (void)k_work_reschedule(&wire_x_rgb_timeout_work,
                            power_timeout_minutes(power_config.rgb_timeout_min));

    if (power_config.display_on) {
        (void)k_work_reschedule(&wire_x_display_timeout_work,
                                power_timeout_minutes(power_config.display_timeout_min));
    } else {
        (void)k_work_cancel_delayable(&wire_x_display_timeout_work);
    }
}

static void rgb_timeout_work_cb(struct k_work *work) {
    ARG_UNUSED(work);

    if (rgb_config.on) {
        rgb_timed_out = true;
        if (!rgb_ble_feedback_active) {
            (void)zmk_rgb_underglow_off();
        }
    }
}

static void display_timeout_work_cb(struct k_work *work) {
    ARG_UNUSED(work);

    if (power_config.display_on) {
        display_timed_out = true;
        wire_x_display_backlight_set(false);
    }
}

static void wire_x_power_note_activity(void) {
    if (rgb_timed_out && rgb_config.on) {
        rgb_timed_out = false;
        if (rgb_ble_feedback_active) {
            (void)apply_rgb_color();
            (void)zmk_rgb_underglow_on();
        } else {
            (void)apply_rgb_config();
        }
    }

    if (display_timed_out && power_config.display_on) {
        display_timed_out = false;
        wire_x_display_backlight_set(true);
    }

    schedule_power_timeouts();
}

static void sanitize_power_config(void) {
    if (power_config.version != WIRE_X_POWER_CONFIG_VERSION) {
        power_config.version = WIRE_X_POWER_CONFIG_VERSION;
    }

    power_config.rgb_timeout_min = CLAMP(power_config.rgb_timeout_min, 1, 30);
    power_config.display_timeout_min = CLAMP(power_config.display_timeout_min, 1, 30);
    power_config.display_on = power_config.display_on ? true : false;
}

static int set_power_config(int rgb_timeout, int display_timeout, bool display_on) {
    if (rgb_timeout < 1 || rgb_timeout > 30 || display_timeout < 1 || display_timeout > 30) {
        return -EINVAL;
    }

    power_config.version = WIRE_X_POWER_CONFIG_VERSION;
    power_config.rgb_timeout_min = (uint8_t)rgb_timeout;
    power_config.display_timeout_min = (uint8_t)display_timeout;
    power_config.display_on = display_on;
    display_timed_out = false;
    power_config_dirty = true;
    wire_x_display_backlight_set(power_config.display_on);
    wire_x_power_note_activity();
    return 0;
}

static int set_rgb_config(int hue, int saturation, int brightness, int effect, bool on,
                          bool reactive, bool layer_mode) {
    if (hue < 0 || hue > 360 || saturation < 0 || saturation > 100 || brightness < 0 ||
        brightness > 100 || effect < 0 || effect >= WIRE_X_RGB_EFFECTS) {
        return -EINVAL;
    }

    ARG_UNUSED(reactive);

    rgb_config = (struct wire_x_rgb_config){
        .hue = hue,
        .saturation = saturation,
        .brightness = brightness,
        .effect = effect,
        .on = on,
        .reactive = false,
        .layer_mode = layer_mode,
    };
    memcpy(rgb_config.layer_colors, rgb_default_config.layer_colors, sizeof(rgb_config.layer_colors));
    rgb_config_dirty = true;
    rgb_timed_out = false;
    schedule_power_timeouts();
    return apply_rgb_config();
}

static int handle_set_rgb(const struct wire_x_rpc_target *target, int id, const char *line) {
    int hue;
    int saturation;
    int brightness;
    int effect;
    bool on;
    bool reactive;
    bool layer_mode;

    if (!json_get_int(line, "\"hue\"", &hue) ||
        !json_get_int(line, "\"saturation\"", &saturation) ||
        !json_get_int(line, "\"brightness\"", &brightness) ||
        !json_get_int(line, "\"effect\"", &effect) || !json_get_bool(line, "\"on\"", &on) ||
        !json_get_bool(line, "\"reactive\"", &reactive) ||
        !json_get_bool(line, "\"layerMode\"", &layer_mode)) {
        rpc_reply_error(target, id, "bad set_rgb request");
        return -EINVAL;
    }

    struct {
        uint16_t hue;
        uint8_t saturation;
    } layer_colors[WIRE_X_RPC_LAYERS];
    memcpy(layer_colors, rgb_config.layer_colors, sizeof(layer_colors));

    int ret = set_rgb_config(hue, saturation, brightness, effect, on, reactive, layer_mode);
    if (ret < 0) {
        rpc_reply_error(target, id, "unsupported rgb option");
        return ret;
    }

    memcpy(rgb_config.layer_colors, layer_colors, sizeof(rgb_config.layer_colors));
    rpc_reply_ok(target, id, "\"cmd\":\"set_rgb\"");
    return 0;
}

static int handle_set_layer_rgb(const struct wire_x_rpc_target *target, int id, const char *line) {
    int layer;
    int hue;
    int saturation;

    if (!json_get_int(line, "\"layer\"", &layer) || !json_get_int(line, "\"hue\"", &hue) ||
        !json_get_int(line, "\"saturation\"", &saturation)) {
        rpc_reply_error(target, id, "bad set_layer_rgb request");
        return -EINVAL;
    }

    if (layer < 0 || layer >= WIRE_X_RPC_LAYERS || hue < 0 || hue > 360 || saturation < 0 ||
        saturation > 100) {
        rpc_reply_error(target, id, "unsupported layer rgb option");
        return -EINVAL;
    }

    rgb_config.layer_colors[layer].hue = hue;
    rgb_config.layer_colors[layer].saturation = saturation;
    rgb_config_dirty = true;
    int ret = apply_rgb_color();
    if (ret < 0) {
        rpc_reply_error(target, id, "layer rgb update failed");
        return ret;
    }

    rpc_reply_ok(target, id, "\"cmd\":\"set_layer_rgb\"");
    return 0;
}

static int handle_set_power(const struct wire_x_rpc_target *target, int id, const char *line) {
    int rgb_timeout;
    int display_timeout = power_config.display_timeout_min;
    bool display_on = power_config.display_on;

    if (!json_get_int(line, "\"rgbTimeout\"", &rgb_timeout)) {
        rpc_reply_error(target, id, "bad set_power request");
        return -EINVAL;
    }

    (void)json_get_int(line, "\"displayTimeout\"", &display_timeout);
    (void)json_get_bool(line, "\"displayOn\"", &display_on);

    int ret = set_power_config(rgb_timeout, display_timeout, display_on);
    if (ret < 0) {
        rpc_reply_error(target, id, "unsupported power timeout");
        return ret;
    }

    rpc_reply_ok(target, id, "\"cmd\":\"set_power\"");
    return 0;
}

static int handle_apply_config(const struct wire_x_rpc_target *target, int id, const char *line) {
    const char *p = strstr(line, "\"layers\"");
    if (p == NULL) {
        rpc_reply_error(target, id, "missing layers");
        return -EINVAL;
    }

    p = strchr(p, '[');
    if (p == NULL) {
        rpc_reply_error(target, id, "bad layers");
        return -EINVAL;
    }

    int count = 0;
    int applied = 0;
    char keycode[32];

    while (*p != '\0' && count < WIRE_X_RPC_LAYERS * WIRE_X_RPC_POSITIONS) {
        if (*p++ != '"') {
            continue;
        }

        size_t i = 0;
        while (*p != '\0' && *p != '"') {
            if (*p == '\\' && p[1] != '\0') {
                p++;
            }
            if (i + 1 < sizeof(keycode)) {
                keycode[i++] = *p;
            }
            p++;
        }
        keycode[i] = '\0';

        if (*p != '"') {
            break;
        }

        int layer = count / WIRE_X_RPC_POSITIONS;
        int position = count % WIRE_X_RPC_POSITIONS;
        if (apply_keycode(layer, position, keycode) == 0) {
            applied++;
        }
        count++;
    }

    if (count != WIRE_X_RPC_LAYERS * WIRE_X_RPC_POSITIONS) {
        rpc_reply_error(target, id, "incomplete layer map");
        return -EINVAL;
    }

    char extra[64];
    snprintk(extra, sizeof(extra), "\"cmd\":\"apply_config\",\"applied\":%d", applied);
    rpc_reply_ok(target, id, extra);
    return 0;
}

static void rpc_reply_current_config(const struct wire_x_rpc_target *target, int id) {
    char prefix[80];

    snprintk(prefix, sizeof(prefix),
             "{\"id\":%d,\"ok\":true,\"name\":\"" WIRE_X_DEVICE_NAME
             "\",\"config\":{\"version\":1,\"layers\":[",
             id);
    rpc_puts(target, prefix);

    for (int layer = 0; layer < WIRE_X_RPC_LAYERS; layer++) {
        rpc_puts(target, layer == 0 ? "[" : ",[");

        for (int position = 0; position < WIRE_X_RPC_POSITIONS; position++) {
            const char *keycode = WIRE_X_LAYER_KEY_BINDING;
            char macro_keycode[8];
            if (position != WIRE_X_LAYER_KEY_POSITION) {
                if (macro_bindings[layer][position] >= 0) {
                    snprintk(macro_keycode, sizeof(macro_keycode), "M%d",
                             macro_bindings[layer][position]);
                    keycode = macro_keycode;
                } else {
                    const struct zmk_behavior_binding *binding =
                        zmk_keymap_get_layer_binding_at_idx(layer, position);
                    keycode = keycode_name_from_binding(binding);
                }
            }

            if (position > 0) {
                rpc_puts(target, ",");
            }
            rpc_puts(target, "\"");
            rpc_puts(target, keycode);
            rpc_puts(target, "\"");
        }

        rpc_puts(target, "]");
    }

    rpc_puts(target, "],\"encoders\":[");
    for (int encoder = 0; encoder < WIRE_X_RPC_ENCODERS; encoder++) {
        if (encoder > 0) {
            rpc_puts(target, ",");
        }

        rpc_puts(target, "{\"ccw\":\"");
        if (encoder_macro_bindings[encoder][0] >= 0) {
            char macro_keycode[8];
            snprintk(macro_keycode, sizeof(macro_keycode), "M%d",
                     encoder_macro_bindings[encoder][0]);
            rpc_puts(target, macro_keycode);
        } else {
            const char *ccw_keycode = keycode_name_from_param(encoder_params[encoder][0]);
            rpc_puts(target, ccw_keycode != NULL ? ccw_keycode : "NONE");
        }
        rpc_puts(target, "\",\"cw\":\"");
        if (encoder_macro_bindings[encoder][1] >= 0) {
            char macro_keycode[8];
            snprintk(macro_keycode, sizeof(macro_keycode), "M%d",
                     encoder_macro_bindings[encoder][1]);
            rpc_puts(target, macro_keycode);
        } else {
            const char *cw_keycode = keycode_name_from_param(encoder_params[encoder][1]);
            rpc_puts(target, cw_keycode != NULL ? cw_keycode : "NONE");
        }
        char sensitivity[32];
        snprintk(sensitivity, sizeof(sensitivity), "\",\"sensitivity\":%u}",
                 encoder_sensitivity[encoder]);
        rpc_puts(target, sensitivity);
    }

    rpc_puts(target, "],\"macros\":[");
    for (int macro = 0; macro < WIRE_X_RPC_MACROS; macro++) {
        if (macro > 0) {
            rpc_puts(target, ",");
        }

        char fallback[8];
        rpc_puts(target, "{\"name\":");
        rpc_put_json_string(target, macro_name_or_default(macro, fallback, sizeof(fallback)));
        rpc_puts(target, ",\"sequence\":");
        rpc_put_json_string(target, macro_slots[macro].sequence);
        rpc_puts(target, "}");
    }

    char rgb[160];
    snprintk(rgb, sizeof(rgb),
             "],\"rgb\":{\"on\":%s,\"reactive\":%s,\"layerMode\":%s,\"effect\":%u,\"hue\":%u,"
             "\"saturation\":%u,\"brightness\":%u,\"layerColors\":[",
             rgb_config.on ? "true" : "false", rgb_config.reactive ? "true" : "false",
             rgb_config.layer_mode ? "true" : "false", rgb_config.effect, rgb_config.hue,
             rgb_config.saturation, rgb_config.brightness);
    rpc_puts(target, rgb);

    for (int layer = 0; layer < WIRE_X_RPC_LAYERS; layer++) {
        if (layer > 0) {
            rpc_puts(target, ",");
        }

        char layer_color[48];
        snprintk(layer_color, sizeof(layer_color), "{\"hue\":%u,\"saturation\":%u}",
                 rgb_config.layer_colors[layer].hue, rgb_config.layer_colors[layer].saturation);
        rpc_puts(target, layer_color);
    }

    char power[128];
    snprintk(power, sizeof(power),
             "]},\"power\":{\"rgbTimeout\":%u,\"displayTimeout\":%u,\"displayOn\":%s},"
             "\"display\":{\"layers\":[",
             power_config.rgb_timeout_min, power_config.display_timeout_min,
             power_config.display_on ? "true" : "false");
    rpc_puts(target, power);

    for (int layer = 0; layer < WIRE_X_RPC_LAYERS; layer++) {
        if (layer > 0) {
            rpc_puts(target, ",");
        }
        rpc_put_json_string(target, wire_x_display_layer_label(layer));
    }

    rpc_puts(target, "],\"encoders\":[");
    for (int layer = 0; layer < WIRE_X_RPC_LAYERS; layer++) {
        if (layer > 0) {
            rpc_puts(target, ",");
        }
        rpc_puts(target, "[");
        for (int encoder = 0; encoder < WIRE_X_RPC_ENCODERS; encoder++) {
            if (encoder > 0) {
                rpc_puts(target, ",");
            }
            rpc_put_json_string(target, wire_x_display_encoder_label(layer, encoder));
        }
        rpc_puts(target, "]");
    }

    rpc_puts(target, "],\"keys\":[");
    for (int layer = 0; layer < WIRE_X_RPC_LAYERS; layer++) {
        if (layer > 0) {
            rpc_puts(target, ",");
        }
        rpc_puts(target, "[");
        for (int key = 0; key < 12; key++) {
            if (key > 0) {
                rpc_puts(target, ",");
            }
            rpc_put_json_string(target, wire_x_display_key_label(layer, key));
        }
        rpc_puts(target, "]");
    }

    rpc_puts(target, "]}}}\n");
}

static int save_encoder_settings(void) {
    int ret = 0;
    if (encoder_params_dirty) {
        ret = settings_save_one(WIRE_X_ENCODER_SETTINGS_KEY, encoder_params,
                                sizeof(encoder_params));
    }
    if (ret >= 0 && encoder_macro_bindings_dirty) {
        ret = settings_save_one(WIRE_X_ENCODER_MACRO_SETTINGS_KEY, encoder_macro_bindings,
                                sizeof(encoder_macro_bindings));
    }
    if (ret >= 0 && encoder_sensitivity_dirty) {
        ret = settings_save_one(WIRE_X_ENCODER_SENSITIVITY_SETTINGS_KEY, encoder_sensitivity,
                                sizeof(encoder_sensitivity));
    }
    if (ret >= 0) {
        encoder_params_dirty = false;
        encoder_macro_bindings_dirty = false;
        encoder_sensitivity_dirty = false;
    }

    return ret;
}

static int reset_encoder_settings(void) {
    int ret = settings_delete(WIRE_X_ENCODER_SETTINGS_KEY);
    if (ret < 0 && ret != -ENOENT) {
        return ret;
    }
    ret = settings_delete(WIRE_X_ENCODER_MACRO_SETTINGS_KEY);
    if (ret < 0 && ret != -ENOENT) {
        return ret;
    }
    ret = settings_delete(WIRE_X_ENCODER_SENSITIVITY_SETTINGS_KEY);
    if (ret < 0 && ret != -ENOENT) {
        return ret;
    }

    memcpy(encoder_params, encoder_default_params, sizeof(encoder_params));
    memset(encoder_macro_bindings, WIRE_X_RPC_NO_MACRO, sizeof(encoder_macro_bindings));
    memset(encoder_sensitivity, 1, sizeof(encoder_sensitivity));
    encoder_params_dirty = false;
    encoder_macro_bindings_dirty = false;
    encoder_sensitivity_dirty = false;
    return apply_all_encoder_bindings();
}

static int save_rgb_settings(void) {
    int ret = settings_save_one(WIRE_X_RGB_SETTINGS_KEY, &rgb_config, sizeof(rgb_config));
    if (ret >= 0) {
        rgb_config_dirty = false;
    }

    return ret;
}

static int reset_rgb_settings(void) {
    int ret = settings_delete(WIRE_X_RGB_SETTINGS_KEY);
    if (ret < 0 && ret != -ENOENT) {
        return ret;
    }

    rgb_config = rgb_default_config;
    rgb_config_dirty = false;
    return apply_rgb_config();
}

static int save_power_settings(void) {
    int ret = settings_save_one(WIRE_X_POWER_SETTINGS_KEY, &power_config, sizeof(power_config));
    if (ret >= 0) {
        power_config_dirty = false;
    }

    return ret;
}

static int reset_power_settings(void) {
    int ret = settings_delete(WIRE_X_POWER_SETTINGS_KEY);
    if (ret < 0 && ret != -ENOENT) {
        return ret;
    }

    power_config = power_default_config;
    power_config_dirty = false;
    display_timed_out = false;
    wire_x_display_backlight_set(power_config.display_on);
    wire_x_power_note_activity();
    return 0;
}

static void clear_macro_bindings(void) {
    for (int layer = 0; layer < WIRE_X_RPC_LAYERS; layer++) {
        for (int position = 0; position < WIRE_X_RPC_POSITIONS; position++) {
            macro_bindings[layer][position] = WIRE_X_RPC_NO_MACRO;
        }
    }
}

static bool macro_config_is_dirty(void) {
    if (macro_bindings_dirty) {
        return true;
    }

    for (int macro = 0; macro < WIRE_X_RPC_MACROS; macro++) {
        if (macro_slot_dirty[macro]) {
            return true;
        }
    }

    return false;
}

static int save_macro_settings(void) {
    int ret = 0;
    if (macro_bindings_dirty) {
        ret = settings_save_one(WIRE_X_MACRO_BINDINGS_SETTINGS_KEY, macro_bindings,
                                sizeof(macro_bindings));
        if (ret < 0) {
            return ret;
        }
        macro_bindings_dirty = false;
    }

    for (int macro = 0; macro < WIRE_X_RPC_MACROS; macro++) {
        if (!macro_slot_dirty[macro]) {
            continue;
        }

        char setting_name[32];
        snprintk(setting_name, sizeof(setting_name), WIRE_X_MACRO_SETTINGS_PREFIX "%d", macro);

        if (macro_slots[macro].name[0] == '\0' && macro_slots[macro].sequence[0] == '\0') {
            ret = settings_delete(setting_name);
        } else {
            ret = settings_save_one(setting_name, &macro_slots[macro], sizeof(macro_slots[macro]));
        }

        if (ret < 0) {
            return ret;
        }

        macro_slot_dirty[macro] = false;
    }

    return 0;
}

static int reset_macro_settings(void) {
    int ret = settings_delete(WIRE_X_MACRO_BINDINGS_SETTINGS_KEY);
    if (ret < 0 && ret != -ENOENT) {
        return ret;
    }

    for (int macro = 0; macro < WIRE_X_RPC_MACROS; macro++) {
        char setting_name[32];
        snprintk(setting_name, sizeof(setting_name), WIRE_X_MACRO_SETTINGS_PREFIX "%d", macro);
        ret = settings_delete(setting_name);
        if (ret < 0 && ret != -ENOENT) {
            return ret;
        }
    }

    memset(macro_slots, 0, sizeof(macro_slots));
    memset(macro_slot_dirty, 0, sizeof(macro_slot_dirty));
    clear_macro_bindings();
    macro_bindings_dirty = false;
    return 0;
}

static int wire_x_factory_reset(bool clear_ble) {
    int ret = zmk_keymap_reset_settings();
    if (ret >= 0) {
        ret = reset_encoder_settings();
    }
    if (ret >= 0) {
        ret = reset_rgb_settings();
    }
    if (ret >= 0) {
        ret = reset_power_settings();
    }
    if (ret >= 0) {
        ret = reset_macro_settings();
    }
    if (ret >= 0) {
        ret = wire_x_display_reset_labels();
    }

#if IS_ENABLED(CONFIG_ZMK_BLE)
    if (ret >= 0 && clear_ble) {
        zmk_ble_clear_all_bonds();
    }
#else
    ARG_UNUSED(clear_ble);
#endif

    return ret;
}

static void wire_x_run_tac_w_factory_migration(void) {
#if IS_ENABLED(CONFIG_SETTINGS)
    if (!tac_w_factory_migration_done) {
        int ret = wire_x_factory_reset(true);
        if (ret >= 0) {
            uint8_t complete = 1;
            ret = settings_save_one(WIRE_X_TAC_W_FACTORY_MIGRATION_SETTINGS_KEY, &complete,
                                    sizeof(complete));
            if (ret >= 0) {
                tac_w_factory_migration_done = true;
            }
        }
    }
#endif

#if IS_ENABLED(CONFIG_ZMK_BLE)
    (void)zmk_ble_set_device_name(wire_x_device_name);
#endif
}

static void handle_line(const char *line, const struct wire_x_rpc_target *target) {
    char cmd[32];
    int id = 0;

    (void)json_get_int(line, "\"id\"", &id);
    if (!json_get_string(line, "\"cmd\"", cmd, sizeof(cmd))) {
        rpc_reply_error(target, id, "missing cmd");
        return;
    }

    wire_x_power_note_activity();

    if (strcmp(cmd, "ping") == 0) {
        rpc_reply_ok(target, id, "\"name\":\"" WIRE_X_DEVICE_NAME "\",\"protocol\":1");
    } else if (strcmp(cmd, "set_key") == 0) {
        (void)handle_set_key(target, id, line);
    } else if (strcmp(cmd, "apply_config") == 0) {
        (void)handle_apply_config(target, id, line);
    } else if (strcmp(cmd, "set_macro") == 0) {
        (void)handle_set_macro(target, id, line);
    } else if (strcmp(cmd, "set_display_label") == 0) {
        (void)handle_set_display_label(target, id, line);
    } else if (strcmp(cmd, "display_test") == 0) {
        (void)handle_display_test(target, id, line);
    } else if (strcmp(cmd, "save_config") == 0) {
        int ret = zmk_keymap_save_changes();
        if (ret >= 0 &&
            (encoder_params_dirty || encoder_macro_bindings_dirty || encoder_sensitivity_dirty)) {
            ret = save_encoder_settings();
        }
        if (ret >= 0 && rgb_config_dirty) {
            ret = save_rgb_settings();
        }
        if (ret >= 0 && power_config_dirty) {
            ret = save_power_settings();
        }
        if (ret >= 0 && macro_config_is_dirty()) {
            ret = save_macro_settings();
        }
        if (ret >= 0 && wire_x_display_labels_dirty()) {
            ret = wire_x_display_save_labels();
        }
        if (ret < 0) {
            rpc_reply_error(target, id, "save failed");
        } else {
            rpc_reply_ok(target, id, "\"cmd\":\"save_config\"");
        }
    } else if (strcmp(cmd, "reset_config") == 0) {
        int ret = wire_x_factory_reset(false);
        if (ret < 0) {
            rpc_reply_error(target, id, "reset failed");
        } else {
            rpc_reply_ok(target, id, "\"cmd\":\"reset_config\"");
        }
    } else if (strcmp(cmd, "hard_reset") == 0) {
        int ret = wire_x_factory_reset(true);
        if (ret < 0) {
            rpc_reply_error(target, id, "hard reset failed");
        } else {
            rpc_reply_ok(target, id, "\"cmd\":\"hard_reset\"");
        }
    } else if (strcmp(cmd, "get_config") == 0) {
        rpc_reply_current_config(target, id);
    } else if (strcmp(cmd, "set_encoder") == 0) {
        (void)handle_set_encoder(target, id, line);
    } else if (strcmp(cmd, "set_encoder_sensitivity") == 0) {
        (void)handle_set_encoder_sensitivity(target, id, line);
    } else if (strcmp(cmd, "set_rgb") == 0) {
        (void)handle_set_rgb(target, id, line);
    } else if (strcmp(cmd, "set_layer_rgb") == 0) {
        (void)handle_set_layer_rgb(target, id, line);
    } else if (strcmp(cmd, "set_power") == 0) {
        (void)handle_set_power(target, id, line);
    } else if (strcmp(cmd, "set_option") == 0) {
        rpc_reply_error(target, id, "device options not implemented yet");
    } else {
        rpc_reply_error(target, id, "unknown cmd");
    }
}

#if IS_ENABLED(CONFIG_SETTINGS)
static int wire_x_settings_set(const char *name, size_t len, settings_read_cb read_cb,
                               void *cb_arg) {
    const char *next;
    if (settings_name_steq(name, "tac_w_factory_v1", &next) && !next) {
        if (len != sizeof(uint8_t)) {
            return -EINVAL;
        }

        uint8_t complete;
        int ret = read_cb(cb_arg, &complete, sizeof(complete));
        if (ret <= 0) {
            return ret;
        }

        tac_w_factory_migration_done = complete == 1;
        return 0;
    }

    if (settings_name_steq(name, "encoders", &next) && !next) {
        if (len != sizeof(encoder_params)) {
            return -EINVAL;
        }

        int ret = read_cb(cb_arg, encoder_params, sizeof(encoder_params));
        if (ret <= 0) {
            return ret;
        }

        encoder_params_dirty = false;
        return apply_all_encoder_bindings();
    }

    if (settings_name_steq(name, "encoder_macros", &next) && !next) {
        if (len != sizeof(encoder_macro_bindings)) {
            return -EINVAL;
        }

        int ret = read_cb(cb_arg, encoder_macro_bindings, sizeof(encoder_macro_bindings));
        if (ret <= 0) {
            return ret;
        }

        for (int encoder = 0; encoder < WIRE_X_RPC_ENCODERS; encoder++) {
            for (int direction = 0; direction < 2; direction++) {
                int macro = encoder_macro_bindings[encoder][direction];
                if (macro < WIRE_X_RPC_NO_MACRO || macro >= WIRE_X_RPC_MACROS) {
                    encoder_macro_bindings[encoder][direction] = WIRE_X_RPC_NO_MACRO;
                } else if (macro >= 0) {
                    encoder_params[encoder][direction] = 0;
                }
            }
        }

        encoder_macro_bindings_dirty = false;
        return apply_all_encoder_bindings();
    }

    if (settings_name_steq(name, "encoder_sensitivity", &next) && !next) {
        if (len != sizeof(encoder_sensitivity)) {
            return -EINVAL;
        }

        int ret = read_cb(cb_arg, encoder_sensitivity, sizeof(encoder_sensitivity));
        if (ret <= 0) {
            return ret;
        }

        for (int encoder = 0; encoder < WIRE_X_RPC_ENCODERS; encoder++) {
            encoder_sensitivity[encoder] =
                CLAMP(encoder_sensitivity[encoder], WIRE_X_ENCODER_SENSITIVITY_MIN,
                      WIRE_X_ENCODER_SENSITIVITY_MAX);
        }
        encoder_sensitivity_dirty = false;
        return 0;
    }

    if (settings_name_steq(name, "rgb", &next) && !next) {
        int ret;
        if (len == sizeof(rgb_config)) {
            ret = read_cb(cb_arg, &rgb_config, sizeof(rgb_config));
        } else if (len == sizeof(struct wire_x_rgb_config_v2_legacy)) {
            struct wire_x_rgb_config_v2_legacy old_rgb;
            ret = read_cb(cb_arg, &old_rgb, sizeof(old_rgb));
            if (ret > 0) {
                rgb_config = rgb_default_config;
                rgb_config.hue = old_rgb.hue;
                rgb_config.saturation = old_rgb.saturation;
                rgb_config.brightness = old_rgb.brightness;
                rgb_config.effect = old_rgb.effect;
                rgb_config.on = old_rgb.on;
                rgb_config.reactive = old_rgb.reactive;
                rgb_config.layer_mode = old_rgb.layer_mode;
                memcpy(rgb_config.layer_colors, old_rgb.layer_colors,
                       sizeof(old_rgb.layer_colors));
            }
        } else if (len == sizeof(struct wire_x_rgb_config_v1)) {
            struct wire_x_rgb_config_v1 old_rgb;
            ret = read_cb(cb_arg, &old_rgb, sizeof(old_rgb));
            if (ret > 0) {
                rgb_config = rgb_default_config;
                rgb_config.hue = old_rgb.hue;
                rgb_config.saturation = old_rgb.saturation;
                rgb_config.brightness = old_rgb.brightness;
                rgb_config.effect = 0;
                rgb_config.on = old_rgb.on;
                rgb_config.reactive = old_rgb.reactive;
            }
        } else {
            return -EINVAL;
        }

        if (ret <= 0) {
            return ret;
        }

        rgb_config.reactive = false;
        rgb_config_dirty = false;
        return apply_rgb_config();
    }

    if (settings_name_steq(name, "power", &next) && !next) {
        if (len == sizeof(power_config)) {
            int ret = read_cb(cb_arg, &power_config, sizeof(power_config));
            if (ret <= 0) {
                return ret;
            }
        } else if (len == 1) {
            uint8_t legacy_rgb_timeout;
            int ret = read_cb(cb_arg, &legacy_rgb_timeout, sizeof(legacy_rgb_timeout));
            if (ret <= 0) {
                return ret;
            }
            power_config = power_default_config;
            power_config.rgb_timeout_min = legacy_rgb_timeout;
        } else if (len == 3) {
            uint8_t legacy_power[3];
            int ret = read_cb(cb_arg, legacy_power, sizeof(legacy_power));
            if (ret <= 0) {
                return ret;
            }
            power_config = power_default_config;
            power_config.rgb_timeout_min = legacy_power[1];
        } else {
            return -EINVAL;
        }

        sanitize_power_config();
        power_config_dirty = false;
        display_timed_out = false;
        wire_x_display_backlight_set(power_config.display_on);
        wire_x_power_note_activity();
        return 0;
    }

    if (settings_name_steq(name, "macro_bindings", &next) && !next) {
        if (len == sizeof(macro_bindings)) {
            int ret = read_cb(cb_arg, macro_bindings, sizeof(macro_bindings));
            if (ret <= 0) {
                return ret;
            }

            macro_bindings_dirty = false;
            return 0;
        }

        if (len == WIRE_X_RPC_LEGACY_LAYERS * WIRE_X_RPC_POSITIONS * sizeof(int8_t)) {
            int8_t legacy[WIRE_X_RPC_LEGACY_LAYERS][WIRE_X_RPC_POSITIONS];
            int ret = read_cb(cb_arg, legacy, sizeof(legacy));
            if (ret <= 0) {
                return ret;
            }

            clear_macro_bindings();
            memcpy(macro_bindings, legacy, sizeof(legacy));
            macro_bindings_dirty = false;
            return 0;
        }

        return -EINVAL;
    }

    int display_ret = wire_x_display_settings_set(name, len, read_cb, cb_arg);
    if (display_ret != -ENOENT) {
        return display_ret;
    }

    if (settings_name_steq(name, "macro", &next) && next) {
        char *endptr;
        int macro = strtol(next, &endptr, 10);
        if (*endptr != '\0' || macro < 0 || macro >= WIRE_X_RPC_MACROS ||
            len != sizeof(macro_slots[macro])) {
            return -EINVAL;
        }

        int ret = read_cb(cb_arg, &macro_slots[macro], sizeof(macro_slots[macro]));
        if (ret <= 0) {
            return ret;
        }

        macro_slots[macro].name[sizeof(macro_slots[macro].name) - 1] = '\0';
        macro_slots[macro].sequence[sizeof(macro_slots[macro].sequence) - 1] = '\0';
        macro_slot_dirty[macro] = false;
        return 0;
    }

    return -ENOENT;
}

SETTINGS_STATIC_HANDLER_DEFINE(wire_x_encoder_map, "wire_x", NULL, wire_x_settings_set, NULL,
                               NULL);
#endif

static void restore_default_rgb_after_layer(struct k_work *work) {
    ARG_UNUSED(work);

    rgb_layer_indicator_active = false;
    if (rgb_config.on) {
        (void)apply_rgb_color();
    }
}

K_WORK_DELAYABLE_DEFINE(wire_x_rgb_layer_restore_work, restore_default_rgb_after_layer);

static int wire_x_rgb_layer_listener(const zmk_event_t *eh) {
    if (as_zmk_layer_state_changed(eh) == NULL || !rgb_config.on || !rgb_config.layer_mode) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    rgb_layer_indicator_active = true;
    (void)apply_rgb_color();
    (void)k_work_reschedule(&wire_x_rgb_layer_restore_work, K_MSEC(WIRE_X_RGB_LAYER_GLOW_MS));
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(wire_x_rgb_layer, wire_x_rgb_layer_listener);
ZMK_SUBSCRIPTION(wire_x_rgb_layer, zmk_layer_state_changed);

static bool ble_profile_key_pressed;
static bool ble_profile_key_long_handled;
static uint8_t ble_profile_key_profile;

static void wire_x_ble_profile_short_action(uint8_t profile) {
#if IS_ENABLED(CONFIG_ZMK_BLE)
    if (zmk_ble_profile_is_connected(profile)) {
        start_ble_connected_glow(profile);
    } else {
        (void)zmk_ble_request_advertising_window(profile, WIRE_X_BLE_ADVERTISING_TIMEOUT_MS);
        start_ble_pending_blink(profile);
    }
#else
    ARG_UNUSED(profile);
#endif
}

static void wire_x_ble_profile_long_press_cb(struct k_work *work) {
    ARG_UNUSED(work);

    if (!ble_profile_key_pressed) {
        return;
    }

    ble_profile_key_long_handled = true;

#if IS_ENABLED(CONFIG_ZMK_BLE)
    (void)zmk_ble_prof_select(ble_profile_key_profile);
    zmk_ble_clear_bonds();
    (void)zmk_ble_request_advertising_window(ble_profile_key_profile,
                                             WIRE_X_BLE_ADVERTISING_TIMEOUT_MS);
    start_ble_pending_blink(ble_profile_key_profile);
#endif
}

K_WORK_DELAYABLE_DEFINE(wire_x_ble_profile_long_press_work, wire_x_ble_profile_long_press_cb);

static int wire_x_ble_profile_rgb_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *event = as_zmk_position_state_changed(eh);
    if (event == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (event->position < WIRE_X_BLE_PROFILE_START_POSITION ||
        event->position >= WIRE_X_BLE_PROFILE_START_POSITION + WIRE_X_BLE_PROFILE_COUNT) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    uint8_t layer = MIN(zmk_keymap_highest_layer_active(), WIRE_X_RPC_LAYERS - 1);
    if (layer != WIRE_X_DEVICE_LAYER_INDEX) {
        return ZMK_EV_EVENT_BUBBLE;
    }

#if IS_ENABLED(CONFIG_ZMK_BLE)
    uint8_t profile = event->position - WIRE_X_BLE_PROFILE_START_POSITION;

    if (event->state) {
        ble_profile_key_pressed = true;
        ble_profile_key_long_handled = false;
        ble_profile_key_profile = profile;
        (void)k_work_reschedule(&wire_x_ble_profile_long_press_work,
                                 K_MSEC(WIRE_X_BLE_PROFILE_LONG_PRESS_MS));
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (!ble_profile_key_pressed || ble_profile_key_profile != profile) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    ble_profile_key_pressed = false;
    (void)k_work_cancel_delayable(&wire_x_ble_profile_long_press_work);

    if (!ble_profile_key_long_handled) {
        wire_x_ble_profile_short_action(profile);
    }
#endif

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(wire_x_ble_profile_rgb, wire_x_ble_profile_rgb_listener);
ZMK_SUBSCRIPTION(wire_x_ble_profile_rgb, zmk_position_state_changed);

static bool factory_reset_key_pressed;

static void wire_x_factory_reset_long_press_cb(struct k_work *work) {
    ARG_UNUSED(work);

    if (!factory_reset_key_pressed) {
        return;
    }

    (void)wire_x_factory_reset(true);
}

K_WORK_DELAYABLE_DEFINE(wire_x_factory_reset_long_press_work,
                        wire_x_factory_reset_long_press_cb);

static int wire_x_factory_reset_key_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *event = as_zmk_position_state_changed(eh);
    if (event == NULL || event->position != WIRE_X_FACTORY_RESET_POSITION) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    uint8_t layer = MIN(zmk_keymap_highest_layer_active(), WIRE_X_RPC_LAYERS - 1);
    if (layer != WIRE_X_DEVICE_LAYER_INDEX) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (event->state) {
        factory_reset_key_pressed = true;
        (void)k_work_reschedule(&wire_x_factory_reset_long_press_work,
                                 K_MSEC(WIRE_X_FACTORY_RESET_LONG_PRESS_MS));
    } else {
        factory_reset_key_pressed = false;
        (void)k_work_cancel_delayable(&wire_x_factory_reset_long_press_work);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(wire_x_factory_reset_key, wire_x_factory_reset_key_listener);
ZMK_SUBSCRIPTION(wire_x_factory_reset_key, zmk_position_state_changed);

static int wire_x_tester_matrix_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *event = as_zmk_position_state_changed(eh);
    if (event == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    wire_x_power_note_activity();
    rpc_notify_matrix_event(event->position, event->state);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(wire_x_tester_matrix, wire_x_tester_matrix_listener);
ZMK_SUBSCRIPTION(wire_x_tester_matrix, zmk_position_state_changed);

static int wire_x_tester_encoder_listener(const zmk_event_t *eh) {
    const struct zmk_sensor_event *event = as_zmk_sensor_event(eh);
    if (event == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    int delta = sensor_event_delta(event);
    wire_x_power_note_activity();
    rpc_notify_encoder_event(event->sensor_index, delta);

    if (event->sensor_index < WIRE_X_RPC_ENCODERS && delta != 0) {
        int direction = delta > 0 ? 1 : 0;
        int macro = encoder_macro_bindings[event->sensor_index][direction];
        int steps = delta > 0 ? delta : -delta;
        int repeats = steps * encoder_sensitivity[event->sensor_index];
        if (macro >= 0 && macro < WIRE_X_RPC_MACROS) {
            struct zmk_behavior_binding_event binding_event = {
                .layer = MIN(zmk_keymap_highest_layer_active(), WIRE_X_RPC_LAYERS - 1),
                .position = WIRE_X_RPC_POSITIONS + event->sensor_index,
                .timestamp = event->timestamp,
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
                .source = 0,
#endif
            };

            for (int step = 0; step < repeats; step++) {
                play_macro_slot(macro, &binding_event);
            }
        } else if (encoder_sensitivity[event->sensor_index] > 1) {
            struct zmk_behavior_binding_event binding_event = {
                .layer = MIN(zmk_keymap_highest_layer_active(), WIRE_X_RPC_LAYERS - 1),
                .position = WIRE_X_RPC_POSITIONS + event->sensor_index,
                .timestamp = event->timestamp,
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
                .source = 0,
#endif
            };
            struct zmk_behavior_binding binding = {
                .behavior_dev = DEVICE_DT_NAME(DT_NODELABEL(kp)),
                .param1 = encoder_params[event->sensor_index][direction],
            };
            int extra_taps = steps * (encoder_sensitivity[event->sensor_index] - 1);
            for (int step = 0; step < extra_taps; step++) {
                (void)zmk_behavior_queue_add(&binding_event, binding, true, 4);
                (void)zmk_behavior_queue_add(&binding_event, binding, false, 8);
            }
        }
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(wire_x_tester_encoder, wire_x_tester_encoder_listener);
ZMK_SUBSCRIPTION(wire_x_tester_encoder, zmk_sensor_event);

static int wire_x_macro_key_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *event = as_zmk_position_state_changed(eh);
    if (event == NULL || !event->state || event->position >= WIRE_X_RPC_POSITIONS) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    uint8_t layer = MIN(zmk_keymap_highest_layer_active(), WIRE_X_RPC_LAYERS - 1);
    int macro = macro_bindings[layer][event->position];
    if (macro < 0 || macro >= WIRE_X_RPC_MACROS) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    struct zmk_behavior_binding_event binding_event = {
        .layer = layer,
        .position = event->position,
        .timestamp = event->timestamp,
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
        .source = event->source,
#endif
    };

    play_macro_slot(macro, &binding_event);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(wire_x_macro_key, wire_x_macro_key_listener);
ZMK_SUBSCRIPTION(wire_x_macro_key, zmk_position_state_changed);

static void cycle_wire_x_layer(void) {
    uint8_t current = MIN(zmk_keymap_highest_layer_active(), WIRE_X_RPC_LAYERS - 1);
    (void)zmk_keymap_layer_to((current + 1) % WIRE_X_RPC_LAYERS, true);
}

static bool layer_key_pressed;
static bool layer_key_long_handled;

static void wire_x_layer_long_press_cb(struct k_work *work) {
    ARG_UNUSED(work);

    if (!layer_key_pressed) {
        return;
    }

    layer_key_long_handled = true;
    wire_x_display_toggle_status();
}

K_WORK_DELAYABLE_DEFINE(wire_x_layer_long_press_work, wire_x_layer_long_press_cb);

static int wire_x_layer_key_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *event = as_zmk_position_state_changed(eh);
    if (event == NULL || event->position != WIRE_X_LAYER_KEY_POSITION) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (event->state) {
        layer_key_pressed = true;
        layer_key_long_handled = false;
        (void)k_work_reschedule(&wire_x_layer_long_press_work,
                                 K_MSEC(WIRE_X_LAYER_LONG_PRESS_MS));
        return ZMK_EV_EVENT_BUBBLE;
    }

    layer_key_pressed = false;
    (void)k_work_cancel_delayable(&wire_x_layer_long_press_work);

    if (!layer_key_long_handled) {
        cycle_wire_x_layer();
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(wire_x_layer_key, wire_x_layer_key_listener);
ZMK_SUBSCRIPTION(wire_x_layer_key, zmk_position_state_changed);

#if IS_ENABLED(CONFIG_BT_ZEPHYR_NUS)
static K_MUTEX_DEFINE(ble_rx_mutex);
static char ble_rx_line[WIRE_X_RPC_MAX_LINE];
static size_t ble_rx_len;

struct wire_x_ble_request {
    struct bt_conn *conn;
    char line[WIRE_X_RPC_MAX_LINE];
};

K_MSGQ_DEFINE(ble_line_queue, sizeof(struct wire_x_ble_request), 4, 4);
static struct wire_x_ble_request ble_rx_request;
static struct wire_x_ble_request ble_work_request;

static bool process_pending_ble_line(void) {
    if (k_msgq_get(&ble_line_queue, &ble_work_request, K_NO_WAIT) < 0) {
        return false;
    }

    struct wire_x_rpc_target target = {
        .transport = WIRE_X_RPC_TRANSPORT_BLE,
        .conn = ble_work_request.conn,
    };

    handle_line(ble_work_request.line, &target);
    bt_conn_unref(ble_work_request.conn);
    return true;
}

static void ble_received(struct bt_conn *conn, const void *data, uint16_t len, void *ctx) {
    ARG_UNUSED(ctx);

    const uint8_t *bytes = data;

    k_mutex_lock(&ble_rx_mutex, K_FOREVER);

    for (uint16_t i = 0; i < len; i++) {
        uint8_t c = bytes[i];

        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            ble_rx_line[ble_rx_len] = '\0';
            if (ble_rx_len > 0) {
                memcpy(ble_rx_request.line, ble_rx_line, ble_rx_len + 1);
                ble_rx_request.conn = bt_conn_ref(conn);
                if (k_msgq_put(&ble_line_queue, &ble_rx_request, K_NO_WAIT) < 0) {
                    bt_conn_unref(ble_rx_request.conn);
                }
            }
            ble_rx_len = 0;
            continue;
        }

        if (ble_rx_len + 1 < sizeof(ble_rx_line)) {
            ble_rx_line[ble_rx_len++] = c;
        } else {
            ble_rx_len = 0;
        }
    }

    k_mutex_unlock(&ble_rx_mutex);
}

static void ble_notif_enabled(bool enabled, void *ctx) {
    ARG_UNUSED(enabled);
    ARG_UNUSED(ctx);
}

static struct bt_nus_cb wire_x_nus_cb = {
    .notif_enabled = ble_notif_enabled,
    .received = ble_received,
};
#endif

static void remap_rpc_thread(void *unused1, void *unused2, void *unused3) {
    ARG_UNUSED(unused1);
    ARG_UNUSED(unused2);
    ARG_UNUSED(unused3);

#if IS_ENABLED(CONFIG_BT_ZEPHYR_NUS)
    (void)bt_nus_cb_register(&wire_x_nus_cb, NULL);
#endif

    wire_x_run_tac_w_factory_migration();
    wire_x_power_note_activity();

    if (!device_is_ready(rpc_uart)) {
        return;
    }

    const struct wire_x_rpc_target uart_target = {
        .transport = WIRE_X_RPC_TRANSPORT_UART,
        .conn = NULL,
    };
    char line[WIRE_X_RPC_MAX_LINE];
    size_t len = 0;

    while (true) {
#if IS_ENABLED(CONFIG_BT_ZEPHYR_NUS)
        if (process_pending_ble_line()) {
            continue;
        }
#endif

        unsigned char c;
        if (uart_poll_in(rpc_uart, &c) != 0) {
            k_sleep(K_MSEC(5));
            continue;
        }

        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            line[len] = '\0';
            if (len > 0) {
                handle_line(line, &uart_target);
            }
            len = 0;
            continue;
        }

        if (len + 1 < sizeof(line)) {
            line[len++] = c;
        } else {
            len = 0;
            rpc_reply_error(&uart_target, 0, "line too long");
        }
    }
}

K_THREAD_DEFINE(wire_x_remap_rpc_thread, 4096, remap_rpc_thread, NULL, NULL, NULL, 9, 0, 0);
