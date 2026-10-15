const DEVICE_NAME = "TAC-W";
const STORAGE_KEY = "braxon-studio-tac-w-profile-v1";
const LEGACY_STORAGE_KEY = "braxon-studio-wire-x-profile-v1";
const SERIAL_BAUD = 115200;
const BLE_CHUNK_SIZE = 20;
const NUS_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const NUS_RX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const NUS_TX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";
const IS_WINDOWS = /Windows/i.test(navigator.userAgentData?.platform || navigator.platform || navigator.userAgent);
const DEVICE_KEEP = "KEEP";
const DEVICE_LAYER = "LAYER";
const LAYER_KEY_POSITION = 5;
const LAYER_COUNT = 6;
const DEVICE_LAYER_INDEX = 5;
const DISPLAY_TEXT_MAX = 7;
const DEFAULT_MACRO_SLOT_COUNT = 11;
const MAX_MACRO_SLOT_COUNT = 60;
const PROFILE_FORMAT = "braxon-studio-profile";
const MAX_FIRMWARE_MACRO_SEQUENCE_LENGTH = 255;
const USB_FILTERS = [
  { usbVendorId: 0x1d50 },
  { usbVendorId: 0x239a },
  { usbVendorId: 0x36ae },
  { usbVendorId: 0x2fe3 },
  { usbVendorId: 0x1915 },
];

const POSITION_META = [
  { id: "0.0", name: "Encoder 0 switch", role: "Encoder press" },
  { id: "0.1", name: "Encoder 1 switch", role: "Encoder press" },
  { id: "1.0", name: "Matrix switch", role: "Row 1" },
  { id: "1.1", name: "Matrix switch", role: "Row 1" },
  { id: "1.2", name: "Matrix switch", role: "Row 1" },
  { id: "1.3", name: "Layer key", role: "Layer cycle" },
  { id: "2.0", name: "Matrix switch", role: "Row 2" },
  { id: "2.1", name: "Matrix switch", role: "Row 2" },
  { id: "2.2", name: "Matrix switch", role: "Row 2" },
  { id: "2.3", name: "Matrix switch", role: "Row 2" },
  { id: "3.0", name: "Matrix switch", role: "Row 3" },
  { id: "3.1", name: "Matrix switch", role: "Row 3" },
  { id: "3.2", name: "Matrix switch", role: "Row 3" },
  { id: "3.3", name: "Matrix switch", role: "Row 3" },
];

const LAYER_LABELS = ["Layer 1", "Layer 2", "Layer 3", "Layer 4", "Layer 5", "Device"];
const DEFAULT_DISPLAY = {
  layers: ["LAYER 1", "LAYER 2", "LAYER 3", "LAYER 4", "LAYER 5", "DEVICE"],
  encoders: [
    ["PGUP", "VOL"],
    ["SCRL", "ZOOM"],
    ["UNDO", "REDO"],
    ["BRT", "TRK"],
    ["NAV", "APP"],
    ["USB", "BLE"],
  ],
  keys: [
    ["A", "B", "C", "LAYER", "E", "F", "G", "H", "I", "J", "K", "L"],
    ["1", "2", "3", "LAYER", "5", "6", "7", "8", "9", "0", "-", "="],
    ["!", "@", "#", "LAYER", "%", "^", "&", "*", "(", ")", "UP", "DN"],
    ["PLAY", "MUTE", "VDN", "LAYER", "NEXT", "PREV", "HOME", "END", "DEL", "PGUP", "PGDN", "TAB"],
    ["ESC", "SPACE", "F1", "LAYER", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10"],
    ["BLE1", "BLE2", "BLE3", "LAYER", "USB", "BLE", "RESET", "CLEAR", "RGB", "FX", "HUE", "BRT"],
  ],
};

const LEGACY_DEFAULT_DISPLAY_KEYS = [
  ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L"],
  ["1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "="],
  ["EX", "AT", "HS", "DL", "PR", "CR", "AM", "AS", "LP", "RP", "UP", "DN"],
  ["PLY", "MUT", "VDN", "VUP", "NXT", "PRV", "HOM", "END", "DEL", "PGU", "PGD", "TAB"],
  ["ESC", "SPC", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10"],
  ["BLE1", "BLE2", "BLE3", "LAYER", "USB", "BLE", "RESET", "CLEAR", "RGB", "FX", "HUE", "BRT"],
];

function cloneDefaultDisplay() {
  return JSON.parse(JSON.stringify(DEFAULT_DISPLAY));
}

function cloneDefaultDisplayCustom() {
  return {
    layers: Array.from({ length: LAYER_COUNT }, () => false),
    encoders: Array.from({ length: LAYER_COUNT }, () => Array.from({ length: 2 }, () => false)),
    keys: Array.from({ length: LAYER_COUNT }, () => Array.from({ length: 12 }, () => false)),
  };
}

const DEFAULT_CONFIG = {
  version: 1,
  layers: [
    ["A", "B", "C", "D", "E", "LAYER", "F", "G", "H", "I", "J", "K", "L", "M"],
    ["N1", "N2", "N3", "N4", "N5", "LAYER", "N6", "N7", "N8", "N9", "N0", "MINUS", "EQUAL", "BSPC"],
    ["EXCL", "AT", "HASH", "DLLR", "PRCNT", "LAYER", "CARET", "AMPS", "ASTRK", "LPAR", "RPAR", "LEFT", "DOWN", "UP"],
    ["C_PLAY_PAUSE", "C_MUTE", "C_VOL_DN", "C_VOL_UP", "C_NEXT", "LAYER", "C_PREV", "HOME", "END", "DEL", "PG_UP", "PG_DN", "TAB", "ENTER"],
    ["ESC", "SPACE", "F1", "F2", "F3", "LAYER", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11"],
    ["KEEP", "KEEP", "KEEP", "KEEP", "KEEP", "LAYER", "KEEP", "KEEP", "KEEP", "KEEP", "KEEP", "KEEP", "KEEP", "KEEP"],
  ],
  encoders: [
    { ccw: "PG_DN", cw: "PG_UP", sensitivity: 1 },
    { ccw: "C_VOL_DN", cw: "C_VOL_UP", sensitivity: 1 },
  ],
  rgb: {
    on: true,
    reactive: false,
    layerMode: true,
    effect: 3,
    hue: 285,
    saturation: 100,
    brightness: 30,
    layerColors: [
      { hue: 0, saturation: 0 },
      { hue: 0, saturation: 100 },
      { hue: 120, saturation: 100 },
      { hue: 240, saturation: 100 },
      { hue: 45, saturation: 100 },
      { hue: 285, saturation: 100 },
    ],
  },
  display: cloneDefaultDisplay(),
  displayCustom: cloneDefaultDisplayCustom(),
  power: {
    rgbTimeout: 10,
    displayTimeout: 10,
    displayOn: true,
  },
  macros: Array.from({ length: DEFAULT_MACRO_SLOT_COUNT }, (_, index) => ({
    name: `M${index}`,
    sequence: [],
  })),
};

const RGB_EFFECTS = ["Solid", "Breathe", "Spectrum", "Swirl"];
const ENCODER_PRESETS = [
  { label: "Volume", icon: "VOL", group: "Everyday", ccw: "C_VOL_DN", cw: "C_VOL_UP" },
  { label: "Windows Zoom", icon: "ZOOM", group: "Everyday", ccwSequence: ["LCTRL+MINUS"], cwSequence: ["LCTRL+PLUS"] },
  { label: "macOS Zoom", icon: "CMD", group: "Everyday", ccwSequence: ["LGUI+MINUS"], cwSequence: ["LGUI+PLUS"] },
  { label: "Brightness", icon: "SUN", group: "Everyday", ccw: "C_BRI_DN", cw: "C_BRI_UP" },
  { label: "Vertical Scroll", icon: "V", group: "Navigation", ccw: "C_AC_SCROLL_DOWN", cw: "C_AC_SCROLL_UP" },
  { label: "Horizontal Scroll", icon: "H", group: "Navigation", ccwSequence: ["LSHFT+C_AC_SCROLL_DOWN"], cwSequence: ["LSHFT+C_AC_SCROLL_UP"] },
  { label: "Pages", icon: "PG", group: "Navigation", ccw: "PG_DN", cw: "PG_UP" },
  { label: "Previous / Next", icon: "NEXT", group: "Navigation", ccw: "C_PREV", cw: "C_NEXT" },
  { label: "Tabs", icon: "TAB", group: "Navigation", ccwSequence: ["LCTRL+LSHFT+TAB"], cwSequence: ["LCTRL+TAB"] },
  { label: "Undo / Redo", icon: "UNDO", group: "Editing", ccw: "C_AC_UNDO", cw: "C_AC_REDO" },
  { label: "Brush Size", icon: "BRUSH", group: "Creative", ccw: "LBKT", cw: "RBKT" },
  { label: "Adobe Brush Hardness", icon: "HARD", group: "Creative", ccwSequence: ["LSHFT+LBKT"], cwSequence: ["LSHFT+RBKT"] },
  { label: "Adobe Canvas Zoom", icon: "ADOBE", group: "Creative", ccwSequence: ["LCTRL+MINUS"], cwSequence: ["LCTRL+PLUS"] },
  { label: "Media Track", icon: "TRACK", group: "Media", ccw: "C_PREV", cw: "C_NEXT" },
  { label: "Media Seek", icon: "SEEK", group: "Media", ccw: "C_REWIND", cw: "C_FAST_FORWARD" },
  { label: "Timeline Jog", icon: "JOG", group: "Video", ccw: "LEFT", cw: "RIGHT" },
  { label: "Frame Nudge", icon: "FRAME", group: "Video", ccw: "LEFT", cw: "RIGHT" },
  { label: "Premiere Timeline Zoom", icon: "PR", group: "Video", ccw: "MINUS", cw: "EQUAL" },
  { label: "Premiere Clip Nudge", icon: "PR", group: "Video", ccwSequence: ["LALT+LEFT"], cwSequence: ["LALT+RIGHT"] },
  { label: "DaVinci Timeline Zoom", icon: "DV", group: "Video", ccwSequence: ["LCTRL+MINUS"], cwSequence: ["LCTRL+PLUS"] },
  { label: "DaVinci Frame Step", icon: "DV", group: "Video", ccw: "LEFT", cw: "RIGHT" },
  { label: "DaVinci Clip Nudge", icon: "DV", group: "Video", ccw: "COMMA", cw: "DOT" },
  { label: "Figma Zoom", icon: "FIG", group: "Design", ccw: "MINUS", cw: "PLUS" },
  { label: "Figma Layer Order", icon: "FIG", group: "Design", ccwSequence: ["LCTRL+LBKT"], cwSequence: ["LCTRL+RBKT"] },
  { label: "Figma Nudge", icon: "FIG", group: "Design", ccw: "LEFT", cw: "RIGHT" },
  { label: "Blender Zoom", icon: "BLEND", group: "3D", ccw: "KP_MINUS", cw: "KP_PLUS" },
  { label: "Blender Orbit X", icon: "X", group: "3D", ccw: "KP_N4", cw: "KP_N6" },
  { label: "Blender Orbit Y", icon: "Y", group: "3D", ccw: "KP_N2", cw: "KP_N8" },
  { label: "Blender Roll", icon: "ROLL", group: "3D", ccw: "KP_N1", cw: "KP_N3" },
  { label: "Desktop Switch", icon: "DESK", group: "System", ccwSequence: ["LCTRL+LGUI+LEFT"], cwSequence: ["LCTRL+LGUI+RIGHT"] },
];

const ENCODER_OPTION_GROUPS = [
  { label: "Media", source: "Media" },
  { label: "Navigation", source: "Navigation" },
  { label: "Editing", source: "Editing" },
  { label: "Windows & Apps", source: "Windows" },
  { label: "System", source: "System" },
  { label: "DAW / MIDI", source: "DAW/MIDI" },
  { label: "Numpad", source: "Numpad" },
  { label: "Letters", source: "Letters" },
  { label: "Numbers", source: "Numbers" },
  { label: "Symbols", source: "Symbol" },
  { label: "Special", source: "Special" },
];

const WEBSITE_LAUNCHER_PRESETS = [
  { label: "Custom website", value: "" },
  { label: "Google", value: "https://google.com" },
  { label: "YouTube", value: "https://youtube.com" },
  { label: "Gmail", value: "https://mail.google.com" },
  { label: "GitHub", value: "https://github.com" },
];

const APP_LAUNCHER_PRESETS = [
  { label: "Paste app path", value: "" },
  { label: "Calculator", value: "calc" },
  { label: "File Explorer", value: "explorer" },
  { label: "Notepad", value: "notepad" },
  { label: "Settings", value: "ms-settings:" },
  { label: "Task Manager", value: "taskmgr" },
];

const WINDOWS_LAUNCHER_CHARACTERS = {
  " ": "SPACE",
  "!": "LSHFT+N1",
  "@": "LSHFT+N2",
  "#": "LSHFT+N3",
  "$": "LSHFT+N4",
  "%": "LSHFT+N5",
  "^": "LSHFT+N6",
  "&": "LSHFT+N7",
  "*": "LSHFT+N8",
  "(": "LSHFT+N9",
  ")": "LSHFT+N0",
  "-": "MINUS",
  "_": "LSHFT+MINUS",
  "=": "EQUAL",
  "+": "LSHFT+EQUAL",
  "[": "LBKT",
  "{": "LSHFT+LBKT",
  "]": "RBKT",
  "}": "LSHFT+RBKT",
  "\\": "BSLH",
  "|": "LSHFT+BSLH",
  ";": "SEMI",
  ":": "LSHFT+SEMI",
  "'": "SQT",
  "\"": "LSHFT+SQT",
  "`": "GRAVE",
  "~": "LSHFT+GRAVE",
  ",": "COMMA",
  "<": "LSHFT+COMMA",
  ".": "DOT",
  ">": "LSHFT+DOT",
  "/": "FSLH",
  "?": "LSHFT+FSLH",
};

const FIRMWARE_KEYCODES = [
  "NONE", "TAB", "ENTER", "BSPC", "DEL", "ESC", "SPACE", "CAPS",
  "LCTRL", "LSHFT", "LALT", "LGUI", "RCTRL", "RSHFT", "RALT", "RGUI",
  "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
  "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
  "N0", "N1", "N2", "N3", "N4", "N5", "N6", "N7", "N8", "N9",
  "EXCL", "AT", "HASH", "DLLR", "PRCNT", "CARET", "AMPS", "ASTRK", "LPAR", "RPAR", "MINUS", "EQUAL", "PLUS", "LBKT", "RBKT", "BSLH", "SEMI", "SQT", "GRAVE", "COMMA", "DOT", "FSLH",
  "HOME", "END", "INS", "LEFT", "RIGHT", "UP", "DOWN", "PG_UP", "PG_DN", "PSCRN", "SLCK", "PAUSE_BREAK",
  "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
  "KP_NUM", "KP_N0", "KP_N1", "KP_N2", "KP_N3", "KP_N4", "KP_N5", "KP_N6", "KP_N7", "KP_N8", "KP_N9",
  "KP_DOT", "KP_ENTER", "KP_PLUS", "KP_MINUS", "KP_MULTIPLY", "KP_DIVIDE", "KP_EQUAL", "KP_COMMA", "KP_LPAR", "KP_RPAR", "KP_CLEAR",
  "C_MUTE", "C_VOL_UP", "C_VOL_DN", "C_PLAY", "C_PAUSE", "C_RECORD", "C_FAST_FORWARD", "C_REWIND",
  "C_PREV", "C_NEXT", "C_STOP", "C_EJECT", "C_RANDOM_PLAY", "C_REPEAT", "C_STOP_EJECT", "C_PLAY_PAUSE",
  "C_POWER", "C_RESET", "C_SLEEP", "C_SLEEP_MODE", "C_MENU", "C_MENU_SELECT", "C_MENU_UP", "C_MENU_DOWN", "C_MENU_LEFT", "C_MENU_RIGHT", "C_MENU_ESC", "C_MENU_INC", "C_MENU_DEC", "C_SNAPSHOT", "C_BASS_BOOST", "C_MEDIA_STEP",
  "C_AL_WORD", "C_AL_TEXT_EDITOR", "C_AL_SHEET", "C_AL_PRESENTATION", "C_AL_MAIL", "C_AL_CALC", "C_AL_MY_COMPUTER", "C_AL_WWW", "C_AL_LOCK", "C_AL_CONTROL_PANEL", "C_AL_NEXT_TASK", "C_AL_PREV_TASK", "C_AL_FILES", "C_AL_MUSIC", "C_AL_MOVIES",
  "C_AC_NEW", "C_AC_OPEN", "C_AC_CLOSE", "C_AC_EXIT", "C_AC_SAVE", "C_AC_PRINT", "C_AC_PROPS", "C_AC_UNDO", "C_AC_REDO", "C_AC_COPY", "C_AC_CUT", "C_AC_PASTE", "C_AC_FIND", "C_AC_SEARCH", "C_AC_HOME",
  "C_AC_ZOOM_IN", "C_AC_ZOOM_OUT", "C_AC_ZOOM", "C_AC_VIEW_TOGGLE", "C_AC_SCROLL_UP", "C_AC_SCROLL_DOWN", "C_AC_BACK", "C_AC_FORWARD", "C_AC_STOP", "C_AC_REFRESH", "C_AC_BOOKMARKS", "C_AC_EDIT", "C_AC_CANCEL", "C_AC_INS", "C_AC_DEL", "C_AC_REPLY", "C_AC_FORWARD_MAIL", "C_AC_SEND", "C_AC_DESKTOP_SHOW_ALL_WINDOWS", "C_AC_DESKTOP_SHOW_ALL_APPLICATIONS", "C_AC_NEXT_KEYBOARD_LAYOUT_SELECT",
  "C_BRI_UP", "C_BRI_DN", "C_BKLT_TOG", "C_BRI_MIN", "C_BRI_MAX", "C_BRI_AUTO",
  "K_APP", "K_POWER", "K_EXEC", "K_HELP", "K_MENU", "K_SELECT", "K_STOP", "K_REDO", "K_UNDO", "K_CUT", "K_COPY", "K_PASTE", "K_FIND", "K_MUTE", "K_VOL_UP", "K_VOL_DN", "K_PLAY_PAUSE", "K_PREV", "K_NEXT", "K_EJECT", "K_WWW", "K_BACK", "K_FORWARD", "K_REFRESH", "K_CALC", "K_SLEEP", "K_LOCK",
  "SYSTEM_POWER", "SYSTEM_SLEEP", "SYSTEM_WAKE_UP",
];
const MACRO_KEYCODES = Array.from({ length: MAX_MACRO_SLOT_COUNT }, (_, index) => `M${index}`);
const EMOJI_KEYCODES = ["EMOJI_SMILE", "EMOJI_HEART", "EMOJI_FIRE", "EMOJI_OK", "EMOJI_THUMBS_UP", "EMOJI_STAR"];
const GAMEPAD_PROFILE_KEYS = ["GAMEPAD_A", "GAMEPAD_B", "GAMEPAD_X", "GAMEPAD_Y", "GAMEPAD_LB", "GAMEPAD_RB", "GAMEPAD_LT", "GAMEPAD_RT", "GAMEPAD_START", "GAMEPAD_SELECT", "GAMEPAD_UP", "GAMEPAD_DOWN", "GAMEPAD_LEFT", "GAMEPAD_RIGHT"];
const MACOS_SHORTCUTS = {
  MAC_COPY: { label: "Copy", glyph: "\u2318 C", sequence: ["LGUI+C"] },
  MAC_PASTE: { label: "Paste", glyph: "\u2318 V", sequence: ["LGUI+V"] },
  MAC_CUT: { label: "Cut", glyph: "\u2318 X", sequence: ["LGUI+X"] },
  MAC_UNDO: { label: "Undo", glyph: "\u2318 Z", sequence: ["LGUI+Z"] },
  MAC_REDO: { label: "Redo", glyph: "\u21E7 \u2318 Z", sequence: ["LSHFT+LGUI+Z"] },
  MAC_SELECT_ALL: { label: "Select All", glyph: "\u2318 A", sequence: ["LGUI+A"] },
  MAC_SAVE: { label: "Save", glyph: "\u2318 S", sequence: ["LGUI+S"] },
  MAC_FIND: { label: "Find", glyph: "\u2318 F", sequence: ["LGUI+F"] },
  MAC_NEW: { label: "New", glyph: "\u2318 N", sequence: ["LGUI+N"] },
  MAC_OPEN: { label: "Open", glyph: "\u2318 O", sequence: ["LGUI+O"] },
  MAC_CLOSE: { label: "Close Window", glyph: "\u2318 W", sequence: ["LGUI+W"] },
  MAC_QUIT: { label: "Quit App", glyph: "\u2318 Q", sequence: ["LGUI+Q"] },
  MAC_SPOTLIGHT: { label: "Spotlight", glyph: "\u2318 Space", sequence: ["LGUI+SPACE"] },
  MAC_SWITCH_APP: { label: "Switch App", glyph: "\u2318 Tab", sequence: ["LGUI+TAB"] },
  MAC_FORCE_QUIT: { label: "Force Quit", glyph: "\u2325 \u2318 Esc", sequence: ["LALT+LGUI+ESC"] },
  MAC_LOCK: { label: "Lock Screen", glyph: "\u2303 \u2318 Q", sequence: ["LCTRL+LGUI+Q"] },
  MAC_MISSION: { label: "Mission Control", glyph: "\u2303 Up", sequence: ["LCTRL+UP"] },
  MAC_APP_WINDOWS: { label: "App Windows", glyph: "\u2303 Down", sequence: ["LCTRL+DOWN"] },
  MAC_SCREENSHOT: { label: "Screenshot", glyph: "\u21E7 \u2318 3", sequence: ["LSHFT+LGUI+N3"] },
  MAC_SCREENSHOT_AREA: { label: "Capture Area", glyph: "\u21E7 \u2318 4", sequence: ["LSHFT+LGUI+N4"] },
  MAC_NEW_TAB: { label: "New Tab", glyph: "\u2318 T", sequence: ["LGUI+T"] },
  MAC_REOPEN_TAB: { label: "Reopen Tab", glyph: "\u21E7 \u2318 T", sequence: ["LSHFT+LGUI+T"] },
  MAC_NEXT_TAB: { label: "Next Tab", glyph: "\u2303 Tab", sequence: ["LCTRL+TAB"] },
  MAC_PREV_TAB: { label: "Previous Tab", glyph: "\u2303 \u21E7 Tab", sequence: ["LCTRL+LSHFT+TAB"] },
};

const QUICK_MACRO_PRESETS = [
  { label: "Copy", shortcut: "Ctrl + C", glyph: "COPY", sequence: ["LCTRL+C"] },
  { label: "Paste", shortcut: "Ctrl + V", glyph: "PASTE", sequence: ["LCTRL+V"] },
  { label: "Cut", shortcut: "Ctrl + X", glyph: "CUT", sequence: ["LCTRL+X"] },
  { label: "Undo", shortcut: "Ctrl + Z", glyph: "UNDO", sequence: ["LCTRL+Z"] },
  { label: "Redo", shortcut: "Ctrl + Y", glyph: "REDO", sequence: ["LCTRL+Y"] },
  { label: "Select all", shortcut: "Ctrl + A", glyph: "ALL", sequence: ["LCTRL+A"] },
  { label: "Save", shortcut: "Ctrl + S", glyph: "SAVE", sequence: ["LCTRL+S"] },
  { label: "Find", shortcut: "Ctrl + F", glyph: "FIND", sequence: ["LCTRL+F"] },
  { label: "Bold", shortcut: "Ctrl + B", glyph: "B", sequence: ["LCTRL+B"] },
  { label: "Italic", shortcut: "Ctrl + I", glyph: "I", sequence: ["LCTRL+I"] },
  { label: "Delete permanently", shortcut: "Shift + Delete", glyph: "DEL", sequence: ["LSHFT+DEL"] },
  { label: "New tab", shortcut: "Ctrl + T", glyph: "TAB+", sequence: ["LCTRL+T"] },
  { label: "Close tab", shortcut: "Ctrl + W", glyph: "TAB-", sequence: ["LCTRL+W"] },
  { label: "Reopen tab", shortcut: "Ctrl + Shift + T", glyph: "REOPEN", sequence: ["LCTRL+LSHFT+T"] },
  { label: "Next tab", shortcut: "Ctrl + Tab", glyph: "TAB>", sequence: ["LCTRL+TAB"] },
  { label: "Previous tab", shortcut: "Ctrl + Shift + Tab", glyph: "<TAB", sequence: ["LCTRL+LSHFT+TAB"] },
  { label: "Address bar", shortcut: "Ctrl + L", glyph: "URL", sequence: ["LCTRL+L"] },
  { label: "Refresh", shortcut: "Ctrl + R", glyph: "RFRSH", sequence: ["LCTRL+R"] },
  { label: "Hard refresh", shortcut: "Ctrl + Shift + R", glyph: "HARD", sequence: ["LCTRL+LSHFT+R"] },
  { label: "Downloads", shortcut: "Ctrl + J", glyph: "DOWN", sequence: ["LCTRL+J"] },
  { label: "History", shortcut: "Ctrl + H", glyph: "HIST", sequence: ["LCTRL+H"] },
  { label: "Zoom in", shortcut: "Ctrl + Plus", glyph: "ZOOM+", sequence: ["LCTRL+PLUS"] },
  { label: "Zoom out", shortcut: "Ctrl + Minus", glyph: "ZOOM-", sequence: ["LCTRL+MINUS"] },
  { label: "Reset zoom", shortcut: "Ctrl + 0", glyph: "100%", sequence: ["LCTRL+N0"] },
  { label: "Full screen", shortcut: "F11", glyph: "FULL", sequence: ["F11"] },
  { label: "Switch app", shortcut: "Alt + Tab", glyph: "APP", sequence: ["LALT+TAB"] },
  { label: "Close app", shortcut: "Alt + F4", glyph: "CLOSE", sequence: ["LALT+F4"] },
  { label: "Lock PC", shortcut: "Win + L", glyph: "LOCK", sequence: ["LGUI+L"] },
  { label: "File Explorer", shortcut: "Win + E", glyph: "FILES", sequence: ["LGUI+E"] },
  { label: "Settings", shortcut: "Win + I", glyph: "SET", sequence: ["LGUI+I"] },
  { label: "Run", shortcut: "Win + R", glyph: "RUN", sequence: ["LGUI+R"] },
  { label: "Show desktop", shortcut: "Win + D", glyph: "DESK", sequence: ["LGUI+D"] },
  { label: "Screenshot", shortcut: "Win + Shift + S", glyph: "SHOT", sequence: ["LGUI+LSHFT+S"] },
  { label: "Task Manager", shortcut: "Ctrl + Shift + Esc", glyph: "TASK", sequence: ["LCTRL+LSHFT+ESC"] },
  { label: "Clipboard history", shortcut: "Win + V", glyph: "CLIP", sequence: ["LGUI+V"] },
  { label: "Emoji panel", shortcut: "Win + Period", glyph: "EMOJI", sequence: ["LGUI+DOT"] },
  { label: "Windows search", shortcut: "Win + S", glyph: "SEARCH", sequence: ["LGUI+S"] },
  { label: "Quick Link menu", shortcut: "Win + X", glyph: "LINK", sequence: ["LGUI+X"] },
  { label: "Project display", shortcut: "Win + P", glyph: "DISPLAY", sequence: ["LGUI+P"] },
];

const KEYCODE_GROUPS = {
  Letters: ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"],
  Numbers: ["N0", "N1", "N2", "N3", "N4", "N5", "N6", "N7", "N8", "N9"],
  Numpad: ["KP_NUM", "KP_N7", "KP_N8", "KP_N9", "KP_DIVIDE", "KP_N4", "KP_N5", "KP_N6", "KP_MULTIPLY", "KP_N1", "KP_N2", "KP_N3", "KP_MINUS", "KP_N0", "KP_DOT", "KP_ENTER", "KP_PLUS", "KP_EQUAL", "KP_COMMA", "KP_LPAR", "KP_RPAR", "KP_CLEAR"],
  "Quick Macros": [],
  Macro: MACRO_KEYCODES,
  Media: ["C_PLAY_PAUSE", "C_PLAY", "C_PAUSE", "C_STOP", "C_RECORD", "C_PREV", "C_NEXT", "C_REWIND", "C_FAST_FORWARD", "C_MUTE", "C_VOL_UP", "C_VOL_DN", "C_EJECT", "C_RANDOM_PLAY", "C_REPEAT", "C_STOP_EJECT", "C_BASS_BOOST", "C_MEDIA_STEP"],
  "DAW/MIDI": ["C_RECORD", "C_PLAY", "C_STOP", "C_REWIND", "C_FAST_FORWARD", "C_PLAY_PAUSE", "C_PREV", "C_NEXT", "C_AL_MUSIC", "C_AC_UNDO", "C_AC_REDO", "C_AC_SAVE", "C_AC_CUT", "C_AC_COPY", "C_AC_PASTE", "C_AC_ZOOM_IN", "C_AC_ZOOM_OUT"],
  Symbol: ["EXCL", "AT", "HASH", "DLLR", "PRCNT", "CARET", "AMPS", "ASTRK", "LPAR", "RPAR", "MINUS", "EQUAL", "PLUS", "LBKT", "RBKT", "BSLH", "SEMI", "SQT", "GRAVE", "COMMA", "DOT", "FSLH", "KP_LPAR", "KP_RPAR"],
  Navigation: ["LEFT", "RIGHT", "UP", "DOWN", "HOME", "END", "PG_UP", "PG_DN", "INS", "C_AC_HOME", "C_AC_BACK", "C_AC_FORWARD", "C_AC_STOP", "C_AC_REFRESH", "C_AC_BOOKMARKS", "C_AC_SCROLL_UP", "C_AC_SCROLL_DOWN"],
  Editing: ["C_AC_UNDO", "C_AC_REDO", "C_AC_CUT", "C_AC_COPY", "C_AC_PASTE", "C_AC_FIND", "C_AC_SEARCH", "C_AC_SAVE", "C_AC_PRINT", "K_UNDO", "K_REDO", "K_CUT", "K_COPY", "K_PASTE", "K_FIND"],
  macOS: Object.keys(MACOS_SHORTCUTS),
  Windows: ["LGUI", "RGUI", "K_APP", "C_AL_MY_COMPUTER", "C_AL_FILES", "C_AL_WWW", "C_AL_MAIL", "C_AL_CALC", "C_AL_CONTROL_PANEL", "C_AL_LOCK", "C_AL_NEXT_TASK", "C_AL_PREV_TASK", "C_AC_DESKTOP_SHOW_ALL_WINDOWS", "C_AC_DESKTOP_SHOW_ALL_APPLICATIONS", "C_AC_NEXT_KEYBOARD_LAYOUT_SELECT"],
  Game: ["W", "A", "S", "D", "SPACE", "LCTRL", "LSHFT", "LALT", "ESC", "TAB", "R", "E", "Q", "F", "G", "N1", "N2", "N3", "N4", "N5", "LEFT", "RIGHT", "UP", "DOWN", ...GAMEPAD_PROFILE_KEYS],
  System: ["SYSTEM_POWER", "SYSTEM_SLEEP", "SYSTEM_WAKE_UP", "C_POWER", "C_RESET", "C_SLEEP", "C_SLEEP_MODE", "K_POWER", "K_SLEEP", "K_LOCK", "PSCRN", "SLCK", "PAUSE_BREAK", "C_BRI_UP", "C_BRI_DN", "C_BKLT_TOG", "C_BRI_MIN", "C_BRI_MAX", "C_BRI_AUTO"],
  Emoji: EMOJI_KEYCODES,
  Special: ["NONE", "TAB", "ENTER", "BSPC", "DEL", "ESC", "SPACE", "CAPS", "LCTRL", "LSHFT", "LALT", "LGUI", "RCTRL", "RSHFT", "RALT", "RGUI", "HOME", "END", "INS", "LEFT", "RIGHT", "UP", "DOWN", "PG_UP", "PG_DN", "PSCRN", "SLCK", "PAUSE_BREAK", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12"],
};

const ALL_KEYCODES = [...new Set(Object.values(KEYCODE_GROUPS).flat())];
const FIRMWARE_KEYCODE_SET = new Set(FIRMWARE_KEYCODES);
const PROFILE_ONLY_KEYCODE_SET = new Set([...EMOJI_KEYCODES, ...GAMEPAD_PROFILE_KEYS]);
const MACRO_INSERT_KEYS = [
  "A", "B", "C", "D", "E", "F", "G",
  "N1", "N2", "N3", "N4", "N5",
  "TAB", "ENTER", "BSPC", "DEL", "ESC", "SPACE",
  "PG_UP", "PG_DN", "C_VOL_DN", "C_VOL_UP", "C_PLAY_PAUSE", "C_STOP", "C_RECORD", "C_REWIND", "C_FAST_FORWARD",
];
const MACRO_STEP_KEYCODES = [...new Set(FIRMWARE_KEYCODES.filter((keycode) => keycode !== "NONE"))];
const KEY_LABELS = {
  BSPC: "Backspace",
  C_AC_BACK: "Back",
  C_AC_FORWARD: "Forward",
  C_AC_SCROLL_DOWN: "Scroll Down",
  C_AC_SCROLL_UP: "Scroll Up",
  C_AC_ZOOM_IN: "Zoom In",
  C_AC_ZOOM_OUT: "Zoom Out",
  C_BRI_DN: "Brightness Down",
  C_BRI_UP: "Brightness Up",
  C_MUTE: "Mute",
  C_NEXT: "Next",
  C_PLAY_PAUSE: "Play",
  C_PREV: "Previous",
  C_VOL_DN: "Volume Down",
  C_VOL_UP: "Volume Up",
  DEL: "Delete",
  EMOJI_FIRE: "\u{1F525} Fire",
  EMOJI_HEART: "\u2764\uFE0F Heart",
  EMOJI_OK: "\u{1F44C} OK",
  EMOJI_SMILE: "\u{1F600} Smile",
  EMOJI_STAR: "\u2B50 Star",
  EMOJI_THUMBS_UP: "\u{1F44D} Like",
  ESC: "Escape",
  KP_DIVIDE: "Numpad /",
  KP_DOT: "Numpad .",
  KP_ENTER: "Numpad Enter",
  KP_EQUAL: "Numpad =",
  KP_MINUS: "Numpad -",
  KP_MULTIPLY: "Numpad *",
  KP_N0: "Numpad 0",
  KP_N1: "Numpad 1",
  KP_N2: "Numpad 2",
  KP_N3: "Numpad 3",
  KP_N4: "Numpad 4",
  KP_N5: "Numpad 5",
  KP_N6: "Numpad 6",
  KP_N7: "Numpad 7",
  KP_N8: "Numpad 8",
  KP_N9: "Numpad 9",
  KP_NUM: "Num Lock",
  KP_PLUS: "Numpad +",
  INS: "Insert",
  LALT: "Left Alt",
  LCTRL: "Left Ctrl",
  LGUI: "Left GUI",
  LSHFT: "Left Shift",
  PG_DN: "Page Down",
  PG_UP: "Page Up",
  RALT: "Right Alt",
  RCTRL: "Right Ctrl",
  RGUI: "Right GUI",
  RSHFT: "Right Shift",
  SPACE: "Space",
  C_AC_BOOKMARKS: "Bookmarks",
  C_AC_CANCEL: "Cancel",
  C_AC_CLOSE: "Close",
  C_AC_COPY: "Copy",
  C_AC_CUT: "Cut",
  C_AC_DEL: "Delete",
  C_AC_DESKTOP_SHOW_ALL_APPLICATIONS: "Show Apps",
  C_AC_DESKTOP_SHOW_ALL_WINDOWS: "Task View",
  C_AC_EDIT: "Edit",
  C_AC_EXIT: "Exit",
  C_AC_FIND: "Find",
  C_AC_HOME: "Browser Home",
  C_AC_NEW: "New",
  C_AC_OPEN: "Open",
  C_AC_PASTE: "Paste",
  C_AC_PRINT: "Print",
  C_AC_PROPS: "Properties",
  C_AC_REDO: "Redo",
  C_AC_REFRESH: "Refresh",
  C_AC_SAVE: "Save",
  C_AC_SEARCH: "Search",
  C_AC_STOP: "Stop Loading",
  C_AL_CALC: "Calculator",
  C_AL_CONTROL_PANEL: "Control Panel",
  C_AL_FILES: "Files",
  C_AL_LOCK: "Lock Screen",
  C_AL_MAIL: "Mail",
  C_AL_MUSIC: "Music",
  C_AL_MY_COMPUTER: "This PC",
  C_BKLT_TOG: "Backlight Toggle",
  C_EJECT: "Eject",
  C_FAST_FORWARD: "Fast Forward",
  C_BASS_BOOST: "Bass Boost",
  C_MEDIA_STEP: "Media Step",
  C_PLAY: "Play",
  C_PAUSE: "Pause",
  C_PLAY_PAUSE: "Play / Pause",
  C_RANDOM_PLAY: "Shuffle",
  C_RECORD: "Record",
  C_REPEAT: "Repeat",
  C_RESET: "Consumer Reset",
  C_REWIND: "Rewind",
  C_SLEEP: "Sleep",
  C_STOP: "Stop",
  C_STOP_EJECT: "Stop / Eject",
  GAMEPAD_A: "Gamepad A (profile only)",
  GAMEPAD_B: "Gamepad B (profile only)",
  GAMEPAD_X: "Gamepad X (profile only)",
  GAMEPAD_Y: "Gamepad Y (profile only)",
  K_APP: "Context Menu",
  K_CALC: "Calculator",
  K_LOCK: "Lock",
  K_POWER: "Keyboard Power",
  K_SLEEP: "Keyboard Sleep",
  KP_CLEAR: "Numpad Clear",
  KP_COMMA: "Numpad ,",
  KP_LPAR: "Numpad (",
  KP_RPAR: "Numpad )",
  PAUSE_BREAK: "Pause Break",
  PSCRN: "Print Screen",
  SLCK: "Scroll Lock",
  SYSTEM_POWER: "System Power",
  SYSTEM_SLEEP: "System Sleep",
  SYSTEM_WAKE_UP: "System Wake",
};

const KEY_GLYPHS = {
  EXCL: "!",
  AT: "@",
  HASH: "#",
  DLLR: "$",
  PRCNT: "%",
  CARET: "^",
  AMPS: "&",
  ASTRK: "*",
  LPAR: "(",
  RPAR: ")",
  MINUS: "-",
  EQUAL: "=",
  PLUS: "+",
  LBKT: "[",
  RBKT: "]",
  BSLH: "\\",
  SEMI: ";",
  SQT: "'",
  GRAVE: "`",
  COMMA: ",",
  DOT: ".",
  FSLH: "/",
  KP_LPAR: "(",
  KP_RPAR: ")",
  C_PLAY_PAUSE: "\u25B6 II",
  C_PLAY: "\u25B6",
  C_PAUSE: "II",
  C_STOP: "\u25A0",
  C_RECORD: "\u25CF",
  C_PREV: "|<",
  C_NEXT: ">|",
  C_REWIND: "<<",
  C_FAST_FORWARD: ">>",
  C_MUTE: "VOL X",
  C_VOL_UP: "VOL +",
  C_VOL_DN: "VOL -",
  C_EJECT: "EJECT",
  C_RANDOM_PLAY: "SHUFFLE",
  C_REPEAT: "REPEAT",
  C_STOP_EJECT: "STOP / EJECT",
  C_BASS_BOOST: "B+",
  C_MEDIA_STEP: "STEP",
};

const DISPLAY_SHORT_LABELS = {
  AMPS: "&",
  ASTRK: "*",
  AT: "@",
  BSPC: "BSPC",
  CAPS: "CAPS",
  CARET: "^",
  BSLH: "\\",
  COMMA: ",",
  C_AC_BACK: "BACK",
  C_AC_BOOKMARKS: "BOOK",
  C_AC_COPY: "COPY",
  C_AC_CUT: "CUT",
  C_AC_HOME: "HOME",
  C_AC_PASTE: "PASTE",
  C_AC_REDO: "REDO",
  C_AC_REFRESH: "RFRSH",
  C_AC_SAVE: "SAVE",
  C_AC_SEARCH: "SRCH",
  C_AC_STOP: "STOP",
  C_AC_FORWARD: "FWD",
  C_AC_SCROLL_DOWN: "SDN",
  C_AC_SCROLL_UP: "SUP",
  C_AC_ZOOM_IN: "ZIN",
  C_AC_ZOOM_OUT: "ZOUT",
  C_AL_CALC: "CALC",
  C_AL_FILES: "FILES",
  C_AL_MAIL: "MAIL",
  C_AL_MUSIC: "MUSIC",
  C_BRI_DN: "BRI-",
  C_BRI_UP: "BRI+",
  C_EJECT: "EJECT",
  C_FAST_FORWARD: "FFWD",
  C_MUTE: "MUTE",
  C_NEXT: "NEXT",
  C_PLAY: "PLAY",
  C_PLAY_PAUSE: "PLAY",
  C_PREV: "PREV",
  C_RECORD: "REC",
  C_REWIND: "REW",
  C_STOP: "STOP",
  C_VOL_DN: "VDN",
  C_VOL_UP: "VUP",
  DEL: "DEL",
  DLLR: "$",
  DOT: ".",
  ENTER: "ENTER",
  EQUAL: "=",
  ESC: "ESC",
  EXCL: "!",
  FSLH: "/",
  GRAVE: "`",
  HASH: "#",
  INS: "INS",
  LAYER: "LAYER",
  LALT: "LALT",
  LCTRL: "LCTRL",
  LGUI: "LGUI",
  LPAR: "(",
  LBKT: "[",
  LSHFT: "LSHFT",
  MINUS: "-",
  NONE: "NONE",
  PAUSE_BREAK: "PAUSE",
  PG_DN: "PGDN",
  PG_UP: "PGUP",
  PLUS: "+",
  PRCNT: "%",
  PSCRN: "PSCRN",
  RALT: "RALT",
  RCTRL: "RCTRL",
  RGUI: "RGUI",
  RPAR: ")",
  RBKT: "]",
  RSHFT: "RSHFT",
  SEMI: ";",
  SLCK: "SLCK",
  SQT: "'",
  SPACE: "SPACE",
  TAB: "TAB",
};

const CATEGORY_ICONS = {
  Letters: "ABC",
  Numbers: "123",
  Numpad: "NUM",
  "Quick Macros": "FAST",
  Macro: "M",
  Media: "\u25B6",
  "DAW/MIDI": "MIDI",
  Symbol: "#",
  Navigation: "NAV",
  Editing: "EDIT",
  macOS: "\u2318",
  Windows: "WIN",
  Game: "GAME",
  System: "SYS",
  Emoji: ":-)",
  Special: "FN",
};

const $ = (selector) => document.querySelector(selector);
const $$ = (selector) => [...document.querySelectorAll(selector)];

function escapeHtml(value) {
  return String(value).replace(/[&<>"']/g, (character) => ({
    "&": "&amp;",
    "<": "&lt;",
    ">": "&gt;",
    "\"": "&quot;",
    "'": "&#39;",
  })[character]);
}

const elements = {
  authorizationShell: $("#authorizationShell"),
  authorizeBleButton: $("#authorizeBleButton"),
  authorizeUsbButton: $("#authorizeUsbButton"),
  bleConnectButton: $("#bleConnectButton"),
  categoryTabs: $("#categoryTabs"),
  clearLogButton: $("#clearLogButton"),
  connectionBadge: $("#connectionBadge"),
  disconnectButton: $("#disconnectButton"),
  displayEditorNote: $("#displayEditorNote"),
  displayEncoderLabels: $("#displayEncoderLabels"),
  displayKeyLabels: $("#displayKeyLabels"),
  displayLayerName: $("#displayLayerName"),
  displayPower: $("#displayPower"),
  displayTimeout: $("#displayTimeout"),
  displayTimeoutValue: $("#displayTimeoutValue"),
  encoderGrid: $("#encoderGrid"),
  keyGrid: $("#keyGrid"),
  keyLibrary: $("#keyLibrary"),
  keySearch: $("#keySearch"),
  libraryBackdrop: $("#libraryBackdrop"),
  libraryCloseButton: $("#libraryCloseButton"),
  libraryExpandButton: $("#libraryExpandButton"),
  libraryPanel: $("#libraryPanel"),
  layerTabs: $("#layerTabs"),
  launcherBuildButton: $("#launcherBuildButton"),
  launcherPreset: $("#launcherPreset"),
  launcherTarget: $("#launcherTarget"),
  launcherType: $("#launcherType"),
  macroAddButton: $("#macroAddButton"),
  macroAddDelayButton: $("#macroAddDelayButton"),
  macroClearButton: $("#macroClearButton"),
  macroDeleteButton: $("#macroDeleteButton"),
  macroDelay: $("#macroDelay"),
  macroKeyPalette: $("#macroKeyPalette"),
  macroList: $("#macroList"),
  macroName: $("#macroName"),
  macroRecordButton: $("#macroRecordButton"),
  macroRecorder: $("#macroRecorder"),
  macroSaveButton: $("#macroSaveButton"),
  macroSequence: $("#macroSequence"),
  profileImportFile: $("#profileImportFile"),
  protocolLog: $("#protocolLog"),
  recordedKeys: $("#recordedKeys"),
  recorderTitle: $("#recorderTitle"),
  recordCancelButton: $("#recordCancelButton"),
  recordingCount: $("#recordingCount"),
  recordStopButton: $("#recordStopButton"),
  rgbBrightness: $("#rgbBrightness"),
  rgbBrightnessValue: $("#rgbBrightnessValue"),
  rgbEffectName: $("#rgbEffectName"),
  rgbEffects: $("#rgbEffects"),
  rgbHue: $("#rgbHue"),
  rgbHueValue: $("#rgbHueValue"),
  rgbLayerColors: $("#rgbLayerColors"),
  rgbLayerMode: $("#rgbLayerMode"),
  rgbPower: $("#rgbPower"),
  rgbSaturation: $("#rgbSaturation"),
  rgbSaturationValue: $("#rgbSaturationValue"),
  rgbSwatch: $("#rgbSwatch"),
  rgbTimeout: $("#rgbTimeout"),
  rgbTimeoutValue: $("#rgbTimeoutValue"),
  selectedKeycode: $("#selectedKeycode"),
  selectedName: $("#selectedName"),
  selectedPosition: $("#selectedPosition"),
  studioShell: $("#studioShell"),
  testerCapturePad: $("#testerCapturePad"),
  testerClearButton: $("#testerClearButton"),
  testerEncoderGrid: $("#testerEncoderGrid"),
  testerEventLog: $("#testerEventLog"),
  testerFocusButton: $("#testerFocusButton"),
  testerHeldKeys: $("#testerHeldKeys"),
  testerKeyGrid: $("#testerKeyGrid"),
  testerLastInput: $("#testerLastInput"),
  toolbarExportButton: $("#toolbarExportButton"),
  toolbarHardResetButton: $("#toolbarHardResetButton"),
  toolbarImportButton: $("#toolbarImportButton"),
  toolbarResetButton: $("#toolbarResetButton"),
  toolbarSaveButton: $("#toolbarSaveButton"),
  toast: $("#toast"),
  usbConnectButton: $("#usbConnectButton"),
};

const state = {
  activeCategory: "Letters",
  activeLayer: 0,
  activePanel: "keymap",
  config: loadConfig(),
  displayDirty: new Set(),
  fullSyncRequired: false,
  knownBleDevices: [],
  libraryExpanded: false,
  logLines: [],
  selectedMacro: 0,
  messageId: 0,
  pending: new Map(),
  recording: null,
  selectedPosition: 0,
  encoderAngles: [0, 0],
  testerActiveEncoders: new Set(),
  testerActivePositions: new Set(),
  testerEvents: [],
  testerHeldKeys: new Set(),
  transport: null,
  authorized: false,
};

let toastTimer = 0;
let autoSaveTimer = 0;
let autoSaveInFlight = false;
let autoSaveQueued = false;
const displayInputTimers = new Map();
const encoderMotionTimers = new Map();
const encoderSensitivityWriteChains = [Promise.resolve(), Promise.resolve()];

function clone(value) {
  return JSON.parse(JSON.stringify(value));
}

function loadConfig() {
  try {
    const data = JSON.parse(localStorage.getItem(STORAGE_KEY) || localStorage.getItem(LEGACY_STORAGE_KEY) || "null");
    return normalizeConfig(data);
  } catch {
    return clone(DEFAULT_CONFIG);
  }
}

function normalizeConfig(data) {
  if (!data || !Array.isArray(data.layers)) {
    return clone(DEFAULT_CONFIG);
  }

  const layers = Array.from({ length: LAYER_COUNT }, (_, index) => {
    const layer = Array.isArray(data.layers[index]) ? data.layers[index] : [];
    const fallback = DEFAULT_CONFIG.layers[index];
    return Array.from({ length: 14 }, (_, position) => (
      position === LAYER_KEY_POSITION
        ? DEVICE_LAYER
        : index === DEVICE_LAYER_INDEX
          ? fallback[position]
          : normalizeKeycode(layer[position], fallback[position])
    ));
  });

  const encoders = Array.from({ length: 2 }, (_, index) => ({
    ccw: normalizeKeycode(data.encoders?.[index]?.ccw, DEFAULT_CONFIG.encoders[index].ccw),
    cw: normalizeKeycode(data.encoders?.[index]?.cw, DEFAULT_CONFIG.encoders[index].cw),
    sensitivity: normalizeRange(
      data.encoders?.[index]?.sensitivity,
      1,
      4,
      DEFAULT_CONFIG.encoders[index].sensitivity,
    ),
  }));

  const fallbackRgb = DEFAULT_CONFIG.rgb;
  const rgb = {
    on: typeof data.rgb?.on === "boolean" ? data.rgb.on : fallbackRgb.on,
    reactive: false,
    layerMode: typeof data.rgb?.layerMode === "boolean" ? data.rgb.layerMode : fallbackRgb.layerMode,
    effect: normalizeRange(data.rgb?.effect, 0, RGB_EFFECTS.length - 1, fallbackRgb.effect),
    hue: normalizeRange(data.rgb?.hue, 0, 360, fallbackRgb.hue),
    saturation: normalizeRange(data.rgb?.saturation, 0, 100, fallbackRgb.saturation),
    brightness: normalizeRange(data.rgb?.brightness, 0, 100, fallbackRgb.brightness),
    layerColors: Array.from({ length: LAYER_COUNT }, (_, index) => ({
      hue: normalizeRange(data.rgb?.layerColors?.[index]?.hue, 0, 360, fallbackRgb.layerColors[index].hue),
      saturation: normalizeRange(
        data.rgb?.layerColors?.[index]?.saturation,
        0,
        100,
        fallbackRgb.layerColors[index].saturation,
      ),
    })),
  };

  const macros = normalizeMacros(data.macros, highestMacroSlotInLayers(layers));
  const display = normalizeDisplay(data.display);
  const power = normalizePower(data.power);
  const config = {
    version: 1,
    layers,
    encoders,
    rgb,
    display,
    power,
    macros,
  };
  config.displayCustom = normalizeDisplayCustom(data.displayCustom, config);
  refreshAutoDisplayLabels(config, false);
  return config;
}

function normalizePower(power) {
  const fallback = DEFAULT_CONFIG.power;
  return {
    rgbTimeout: normalizeRange(power?.rgbTimeout, 1, 30, fallback.rgbTimeout),
    displayTimeout: normalizeRange(power?.displayTimeout, 1, 30, fallback.displayTimeout),
    displayOn: typeof power?.displayOn === "boolean" ? power.displayOn : fallback.displayOn,
  };
}

function sanitizeDisplayInput(value) {
  return String(value ?? "")
    .replace(/[^\x20-\x7e]/g, " ")
    .toUpperCase()
    .slice(0, DISPLAY_TEXT_MAX);
}

function normalizeDisplayText(value, fallback = "") {
  const text = sanitizeDisplayInput(value).trim();
  return text || fallback.slice(0, DISPLAY_TEXT_MAX);
}

function normalizeDisplay(display) {
  const source = display || {};
  return {
    layers: Array.from({ length: LAYER_COUNT }, (_, index) => (
      index === DEVICE_LAYER_INDEX
        ? DEFAULT_DISPLAY.layers[index]
        : normalizeDisplayText(source.layers?.[index], DEFAULT_DISPLAY.layers[index])
    )),
    encoders: Array.from({ length: LAYER_COUNT }, (_, layer) => (
      Array.from({ length: 2 }, (_, index) => (
        layer === DEVICE_LAYER_INDEX
          ? DEFAULT_DISPLAY.encoders[layer][index]
          : normalizeDisplayText(source.encoders?.[layer]?.[index], DEFAULT_DISPLAY.encoders[layer][index])
      ))
    )),
    keys: Array.from({ length: LAYER_COUNT }, (_, layer) => (
      Array.from({ length: 12 }, (_, index) => (
        layer === DEVICE_LAYER_INDEX
          ? DEFAULT_DISPLAY.keys[layer][index]
          : normalizeDisplayText(source.keys?.[layer]?.[index], DEFAULT_DISPLAY.keys[layer][index])
      ))
    )),
  };
}

function normalizeDisplayCustom(displayCustom, config) {
  const source = displayCustom || {};
  const sourceHasFlags = Boolean(displayCustom);
  return {
    layers: Array.from({ length: LAYER_COUNT }, (_, layer) => (
      layer === DEVICE_LAYER_INDEX
        ? false
        : normalizeCustomFlag(source.layers?.[layer], sourceHasFlags ? false : inferLayerDisplayCustom(config, layer))
    )),
    encoders: Array.from({ length: LAYER_COUNT }, (_, layer) => (
      Array.from({ length: 2 }, (_, index) => (
        layer === DEVICE_LAYER_INDEX
          ? false
          : normalizeCustomFlag(
              source.encoders?.[layer]?.[index],
              sourceHasFlags ? false : inferEncoderDisplayCustom(config, layer, index),
            )
      ))
    )),
    keys: Array.from({ length: LAYER_COUNT }, (_, layer) => (
      Array.from({ length: 12 }, (_, index) => (
        layer === DEVICE_LAYER_INDEX
          ? false
          : normalizeCustomFlag(
              source.keys?.[layer]?.[index],
              sourceHasFlags ? false : inferKeyDisplayCustom(config, layer, index),
            )
      ))
    )),
  };
}

function normalizeCustomFlag(value, fallback = false) {
  return typeof value === "boolean" ? value : fallback;
}

function inferLayerDisplayCustom(config, layer) {
  const label = config.display.layers[layer];
  return label !== DEFAULT_DISPLAY.layers[layer];
}

function inferEncoderDisplayCustom(config, layer, index) {
  const label = config.display.encoders[layer][index];
  const fallback = DEFAULT_DISPLAY.encoders[layer][index];
  const auto = autoDisplayEncoderLabelFromConfig(config, index);
  return label !== fallback && label !== auto;
}

function inferKeyDisplayCustom(config, layer, index) {
  const label = config.display.keys[layer][index];
  const fallback = DEFAULT_DISPLAY.keys[layer][index];
  const legacyFallback = LEGACY_DEFAULT_DISPLAY_KEYS[layer]?.[index];
  const auto = autoDisplayKeyLabelFromConfig(config, layer, index);
  return label !== fallback && label !== legacyFallback && label !== auto;
}

function normalizeKeycode(value, fallback = "NONE") {
  const keycode = String(value || "").trim().toUpperCase();
  return keycode === DEVICE_KEEP || keycode === DEVICE_LAYER || ALL_KEYCODES.includes(keycode) ? keycode : fallback;
}

function normalizeRange(value, min, max, fallback) {
  if (value === null || value === undefined || value === "") {
    return fallback;
  }

  const numericValue = Number(value);
  return Number.isInteger(numericValue) && numericValue >= min && numericValue <= max
    ? numericValue
    : fallback;
}

function isFirmwareKeycode(keycode) {
  return keycode === DEVICE_LAYER || FIRMWARE_KEYCODE_SET.has(keycode) || macroIndexFromKeycode(keycode) >= 0;
}

function isProfileOnlyKeycode(keycode) {
  return PROFILE_ONLY_KEYCODE_SET.has(keycode) || !isFirmwareKeycode(keycode);
}

function keyLabel(keycode) {
  const macShortcut = MACOS_SHORTCUTS[keycode];
  if (macShortcut) {
    return macShortcut.label;
  }

  const macroIndex = macroIndexFromKeycode(keycode);
  if (macroIndex >= 0) {
    const index = macroIndex;
    const macro = state.config?.macros?.[index];
    return macro?.name && macro.name !== keycode ? `${keycode} ${macro.name}` : keycode;
  }

  return KEY_LABELS[keycode] || keycode;
}

function keyGlyph(keycode) {
  return MACOS_SHORTCUTS[keycode]?.glyph || KEY_GLYPHS[keycode] || keycode;
}

function keycapLabel(keycode) {
  const macroIndex = macroIndexFromKeycode(keycode);
  if (macroIndex >= 0) {
    return state.config?.macros?.[macroIndex]?.name || keycode;
  }

  return keyLabel(keycode);
}

function macroIndexFromKeycode(keycode) {
  const match = String(keycode || "").match(/^M(\d+)$/);
  if (!match) {
    return -1;
  }

  const index = Number(match[1]);
  return Number.isInteger(index) && index >= 0 && index < MAX_MACRO_SLOT_COUNT ? index : -1;
}

function normalizeMacroStep(value) {
  const token = String(value || "").trim().toUpperCase();
  if (token.startsWith("DLY(") && token.endsWith(")")) {
    const delay = Number.parseInt(token.slice(4, -1), 10);
    if (Number.isInteger(delay) && delay >= 10 && delay <= 5000) {
      return `DLY(${delay})`;
    }
  }

  const parts = token
    .split("+")
    .map(normalizeMacroPart)
    .filter(Boolean);
  return parts.length > 0 ? [...new Set(parts)].join("+") : "";
}

function normalizeMacroPart(value) {
  const aliases = {
    ALT: "LALT",
    BACKSPACE: "BSPC",
    CMD: "LGUI",
    COMMAND: "LGUI",
    CONTROL: "LCTRL",
    CTRL: "LCTRL",
    DELETE: "DEL",
    ESCAPE: "ESC",
    GUI: "LGUI",
    OPTION: "LALT",
    SHIFT: "LSHFT",
    WIN: "LGUI",
    WINDOWS: "LGUI",
  };
  const token = String(value || "").trim().toUpperCase();
  const normalized = aliases[token] || token;
  return MACRO_STEP_KEYCODES.includes(normalized) ? normalized : "";
}

function highestMacroSlotInLayers(layers) {
  let highest = -1;
  layers.forEach((layer) => {
    layer.forEach((keycode) => {
      highest = Math.max(highest, macroIndexFromKeycode(keycode));
    });
  });
  return highest + 1;
}

function macroSourceHasContent(source, index) {
  if (!source) {
    return false;
  }
  const name = String(source.name || `M${index}`).trim();
  const sequence = Array.isArray(source.sequence)
    ? source.sequence.map(normalizeMacroStep).filter(Boolean)
    : typeof source.sequence === "string"
      ? macroSequenceFromText(source.sequence)
      : [];
  return sequence.length > 0 || (name !== "" && name !== `M${index}`);
}

function normalizedMacroCount(macros, requiredCount = DEFAULT_MACRO_SLOT_COUNT) {
  const minimum = Math.min(MAX_MACRO_SLOT_COUNT, Math.max(DEFAULT_MACRO_SLOT_COUNT, requiredCount));
  if (!Array.isArray(macros)) {
    return minimum;
  }

  let count = Math.min(MAX_MACRO_SLOT_COUNT, Math.max(minimum, macros.length));
  while (count > minimum && !macroSourceHasContent(macros[count - 1], count - 1)) {
    count -= 1;
  }
  return count;
}

function normalizeMacros(macros, requiredCount = DEFAULT_MACRO_SLOT_COUNT) {
  const macroCount = normalizedMacroCount(macros, requiredCount);

  return Array.from({ length: macroCount }, (_, index) => {
    const fallback = DEFAULT_CONFIG.macros[index] || { name: `M${index}`, sequence: [] };
    const source = macros?.[index] || fallback;
    const sequence = Array.isArray(source.sequence)
      ? source.sequence.map(normalizeMacroStep).filter(Boolean).slice(0, 96)
      : typeof source.sequence === "string"
        ? macroSequenceFromText(source.sequence)
      : [];

    return {
      name: String(source.name || fallback.name).trim().slice(0, 23) || fallback.name,
      sequence,
    };
  });
}

function persistConfig() {
  localStorage.setItem(STORAGE_KEY, JSON.stringify(state.config));
}

function currentKeycode(position = state.selectedPosition) {
  return state.config.layers[state.activeLayer][position];
}

function isDeviceLayer(layer = state.activeLayer) {
  return layer === DEVICE_LAYER_INDEX;
}

function layerTabLabel(index) {
  return state.config.display?.layers?.[index] || LAYER_LABELS[index] || `Layer ${index + 1}`;
}

function displayLabelForPosition(position, layer = state.activeLayer) {
  if (position < 2) {
    return state.config.display?.encoders?.[layer]?.[position] || DEFAULT_DISPLAY.encoders[layer][position];
  }

  return state.config.display?.keys?.[layer]?.[position - 2] || DEFAULT_DISPLAY.keys[layer][position - 2];
}

function displayTextForKeycode(keycode, macros = null) {
  const macroIndex = macroIndexFromKeycode(keycode);
  if (macroIndex >= 0) {
    const macroName = macros?.[macroIndex]?.name || `M${macroIndex}`;
    return normalizeDisplayText(macroName, `M${macroIndex}`);
  }

  if (/^N\d$/.test(keycode)) {
    return keycode.slice(1);
  }

  if (/^KP_N\d$/.test(keycode)) {
    return `KP${keycode.slice(-1)}`;
  }

  const shortLabel = DISPLAY_SHORT_LABELS[keycode];
  const compact = String(keycode || "NONE")
    .replace(/^C_/, "")
    .replace(/^KP_/, "KP")
    .replace(/_/g, "");
  return normalizeDisplayText(shortLabel || compact, "NONE");
}

function eventToKeycode(event) {
  const key = event.key;
  const code = event.code;

  if (/^Key[A-Z]$/.test(code)) {
    return code.slice(3);
  }

  if (/^Digit\d$/.test(code)) {
    return `N${code.slice(5)}`;
  }

  if (/^Numpad\d$/.test(code)) {
    return `KP_N${code.slice(6)}`;
  }

  const codeMap = {
    ArrowDown: "DOWN",
    ArrowLeft: "LEFT",
    ArrowRight: "RIGHT",
    ArrowUp: "UP",
    Backspace: "BSPC",
    Delete: "DEL",
    End: "END",
    Enter: "ENTER",
    Equal: "EQUAL",
    Escape: "ESC",
    Home: "HOME",
    Insert: "INS",
    Minus: "MINUS",
    NumpadAdd: "KP_PLUS",
    NumpadDecimal: "KP_DOT",
    NumpadDivide: "KP_DIVIDE",
    NumpadEnter: "KP_ENTER",
    NumpadEqual: "KP_EQUAL",
    NumpadMultiply: "KP_MULTIPLY",
    NumpadSubtract: "KP_MINUS",
    PageDown: "PG_DN",
    PageUp: "PG_UP",
    Space: "SPACE",
    Tab: "TAB",
  };

  if (codeMap[code]) {
    return codeMap[code];
  }

  const keyMap = {
    AudioVolumeDown: "C_VOL_DN",
    AudioVolumeMute: "C_MUTE",
    AudioVolumeUp: "C_VOL_UP",
    BrowserBack: "C_AC_BACK",
    BrowserForward: "C_AC_FORWARD",
    MediaPlayPause: "C_PLAY_PAUSE",
    MediaTrackNext: "C_NEXT",
    MediaTrackPrevious: "C_PREV",
  };

  if (keyMap[key]) {
    return keyMap[key];
  }

  if (/^F([1-9]|1[0-2])$/.test(key)) {
    return key;
  }

  if (key === "!") return "EXCL";
  if (key === "@") return "AT";
  if (key === "#") return "HASH";
  if (key === "$") return "DLLR";
  if (key === "%") return "PRCNT";
  if (key === "^") return "CARET";
  if (key === "&") return "AMPS";
  if (key === "*") return "ASTRK";
  if (key === "(") return "LPAR";
  if (key === ")") return "RPAR";
  if (key === "+") return "PLUS";

  return "";
}

function autoDisplayLayerLabel(layer) {
  return DEFAULT_DISPLAY.layers[layer] || `LAYER ${layer + 1}`;
}

function autoDisplayEncoderLabelFromConfig(config, index) {
  const encoder = config.encoders?.[index] || DEFAULT_CONFIG.encoders[index];
  const preset = ENCODER_PRESETS.find((item) => item.ccw === encoder.ccw && item.cw === encoder.cw);
  if (preset?.label === "Volume") {
    return "VOL";
  }
  if (preset?.label === "Page") {
    return "PGUP";
  }
  if (preset?.label === "Zoom") {
    return "ZOOM";
  }
  if (preset?.label === "Scroll") {
    return "SCRL";
  }
  return displayTextForKeycode(encoder.cw, config.macros);
}

function autoDisplayKeyLabelFromConfig(config, layer, index) {
  const keycode = config.layers?.[layer]?.[index + 2] || DEFAULT_CONFIG.layers[layer][index + 2];
  return displayTextForKeycode(keycode, config.macros);
}

function displayCustomValue(config, target, layer, index = 0) {
  if (target === "layer") {
    return Boolean(config.displayCustom?.layers?.[layer]);
  }
  if (target === "encoder") {
    return Boolean(config.displayCustom?.encoders?.[layer]?.[index]);
  }
  return Boolean(config.displayCustom?.keys?.[layer]?.[index]);
}

function setDisplayCustomValue(config, target, layer, index, value) {
  if (target === "layer") {
    config.displayCustom.layers[layer] = value;
  } else if (target === "encoder") {
    config.displayCustom.encoders[layer][index] = value;
  } else {
    config.displayCustom.keys[layer][index] = value;
  }
}

function setDisplayLabelValue(config, target, layer, index, text) {
  if (target === "layer") {
    config.display.layers[layer] = text;
  } else if (target === "encoder") {
    config.display.encoders[layer][index] = text;
  } else {
    config.display.keys[layer][index] = text;
  }
}

function displayAutoLabel(config, target, layer, index = 0) {
  if (target === "layer") {
    return autoDisplayLayerLabel(layer);
  }
  if (target === "encoder") {
    return autoDisplayEncoderLabelFromConfig(config, index);
  }
  return autoDisplayKeyLabelFromConfig(config, layer, index);
}

function refreshAutoDisplayLabels(config = state.config) {
  const changes = [];
  for (let layer = 0; layer < DEVICE_LAYER_INDEX; layer += 1) {
    ["layer"].forEach((target) => {
      if (!displayCustomValue(config, target, layer)) {
        const text = displayAutoLabel(config, target, layer);
        if (config.display.layers[layer] !== text) {
          config.display.layers[layer] = text;
          changes.push({ target, layer, index: 0 });
        }
      }
    });

    for (let encoder = 0; encoder < 2; encoder += 1) {
      if (!displayCustomValue(config, "encoder", layer, encoder)) {
        const text = displayAutoLabel(config, "encoder", layer, encoder);
        if (config.display.encoders[layer][encoder] !== text) {
          config.display.encoders[layer][encoder] = text;
          changes.push({ target: "encoder", layer, index: encoder });
        }
      }
    }

    for (let key = 0; key < 12; key += 1) {
      if (!displayCustomValue(config, "key", layer, key)) {
        const text = displayAutoLabel(config, "key", layer, key);
        if (config.display.keys[layer][key] !== text) {
          config.display.keys[layer][key] = text;
          changes.push({ target: "key", layer, index: key });
        }
      }
    }
  }
  return changes;
}

function stamp() {
  return new Date().toLocaleTimeString([], { hour12: false });
}

function addLog(prefix, text) {
  const line = `[${stamp()}] ${prefix} ${text}`;
  state.logLines.push(line);
  state.logLines = state.logLines.slice(-240);
  elements.protocolLog.textContent = state.logLines.join("\n");
  elements.protocolLog.scrollTop = elements.protocolLog.scrollHeight;
}

function showToast(message) {
  window.clearTimeout(toastTimer);
  elements.toast.textContent = message;
  elements.toast.classList.add("show");
  toastTimer = window.setTimeout(() => elements.toast.classList.remove("show"), 2800);
}

function setSaveButtonState(mode = "clean") {
  const button = elements.toolbarSaveButton;
  if (!button) {
    return;
  }

  button.classList.toggle("dirty", mode === "dirty");
  button.classList.toggle("saving", mode === "saving");
  button.classList.toggle("saved", mode === "saved");
  button.textContent = mode === "saving" ? "Saving" : mode === "saved" ? "Saved" : "Save";

  if (mode === "saved") {
    window.setTimeout(() => {
      if (button.classList.contains("saved")) {
        setSaveButtonState("clean");
      }
    }, 1600);
  }
}

function scheduleBoardSave(reason = "change", delay = 950) {
  if (!state.transport || !state.authorized) {
    setSaveButtonState("dirty");
    return;
  }

  setSaveButtonState("dirty");
  window.clearTimeout(autoSaveTimer);
  autoSaveTimer = window.setTimeout(() => {
    flushBoardSave(reason);
  }, delay);
}

async function flushBoardSave(reason = "change") {
  if (!state.transport || !state.authorized) {
    return;
  }

  if (autoSaveInFlight) {
    autoSaveQueued = true;
    return;
  }

  autoSaveInFlight = true;
  setSaveButtonState("saving");

  try {
    const reply = await sendCommand("save_config");
    if (reply.ok) {
      setSaveButtonState("saved");
      logWire("SAVE", `Auto-saved ${reason}`);
    }
  } catch (error) {
    setSaveButtonState("dirty");
    showToast(error.message);
  } finally {
    autoSaveInFlight = false;
    if (autoSaveQueued) {
      autoSaveQueued = false;
      scheduleBoardSave(reason, 450);
    }
  }
}

function setBadge(label, mode = "idle") {
  elements.connectionBadge.textContent = label;
  elements.connectionBadge.className = `connection-badge ${mode}`;
}

function setButtonsForTransport() {
  const connected = Boolean(state.transport);
  elements.disconnectButton.disabled = !connected;
  elements.usbConnectButton.disabled = connected;
  elements.bleConnectButton.disabled = connected;
  elements.authorizeUsbButton.disabled = connected;
  elements.authorizeBleButton.disabled = connected;
  renderAccessState();
}

function renderAccessState() {
  elements.authorizationShell.hidden = state.authorized;
  elements.studioShell.hidden = !state.authorized;
}

function renderLayerTabs() {
  elements.layerTabs.innerHTML = "";
  Array.from({ length: LAYER_COUNT }, (_, index) => layerTabLabel(index)).forEach((label, index) => {
    const button = document.createElement("button");
    button.type = "button";
    button.className = `layer-tab ${index === state.activeLayer ? "active" : ""} ${isDeviceLayer(index) ? "locked" : ""}`;
    button.setAttribute("role", "tab");
    button.setAttribute("aria-selected", String(index === state.activeLayer));
    button.textContent = label;
    button.addEventListener("click", () => {
      state.activeLayer = index;
      renderLayerTabs();
      renderKeyboard();
      renderSelection();
      renderKeyLibrary();
      renderEncoderGrid();
      renderDisplayControls();
      renderTester();
    });
    elements.layerTabs.append(button);
  });
}

function keyButton(position) {
  const meta = POSITION_META[position];
  const isLayerKey = position === LAYER_KEY_POSITION;
  const keycode = currentKeycode(position);
  const displayCode = isDeviceLayer() ? displayLabelForPosition(position) : keycapLabel(keycode);
  const button = document.createElement("button");
  button.type = "button";
  button.className = `key-slot ${isLayerKey ? "reserved" : ""} ${isDeviceLayer() ? "locked-layer" : ""} ${position === state.selectedPosition ? "selected" : ""}`;
  button.dataset.position = String(position);
  button.title = `${meta.id} ${displayCode}`;
  if (isLayerKey) {
    button.setAttribute("aria-label", "1.3 fixed layer cycle key");
  }
  button.innerHTML = `
    <span class="key-switch-well" aria-hidden="true"></span>
    <span class="keycap-face">
      <span class="key-meta">${meta.id}</span>
      <span class="key-code">${escapeHtml(displayCode)}</span>
      <span class="key-role">${meta.role}</span>
    </span>
  `;
  button.addEventListener("click", () => selectPosition(position));
  return button;
}

function renderKeyboard() {
  renderEncoderSwitches();

  elements.keyGrid.innerHTML = "";
  for (let position = 2; position < POSITION_META.length; position += 1) {
    elements.keyGrid.append(keyButton(position));
  }
}

function renderEncoderSwitches() {
  $$(".encoder-switch").forEach((button) => {
    const position = Number(button.dataset.position);
    const meta = POSITION_META[position];
    const displayCode = isDeviceLayer() ? displayLabelForPosition(position) : keycapLabel(currentKeycode(position));
    button.classList.toggle("selected", position === state.selectedPosition);
    button.classList.toggle("locked-layer", isDeviceLayer());
    button.style.setProperty("--encoder-angle", `${state.encoderAngles[position] || 0}deg`);
    button.title = `${meta.id} ${displayCode}`;
    button.setAttribute("aria-label", `${meta.id} ${meta.name} ${displayCode}`);
    button.innerHTML = `
      <span>ENC ${position}</span>
      <strong>${escapeHtml(displayCode)}</strong>
      <small>Press</small>
    `;
    button.onclick = () => {
      animateKeyboardEncoder(position, "CW");
      selectPosition(position);
    };
  });
}

function selectPosition(position) {
  state.selectedPosition = position;
  renderKeyboard();
  renderSelection();
  renderKeyLibrary();
}

function renderSelection() {
  const meta = POSITION_META[state.selectedPosition];
  elements.selectedPosition.textContent = meta.id;
  elements.selectedName.textContent = meta.name;
  elements.selectedKeycode.textContent = isDeviceLayer()
    ? `${displayLabelForPosition(state.selectedPosition)} locked`
    : keyLabel(currentKeycode());
}

function renderTester() {
  if (!elements.testerKeyGrid || !elements.testerEncoderGrid) {
    return;
  }

  elements.testerEncoderGrid.innerHTML = "";
  state.config.encoders.forEach((encoder, index) => {
    const card = document.createElement("div");
    const active = state.testerActiveEncoders.has(index) || state.testerActivePositions.has(index);
    card.className = `tester-encoder-card ${active ? "active" : ""}`;
    card.innerHTML = `
      <span>ENC ${index}</span>
      <strong>${escapeHtml(keycapLabel(state.config.layers[state.activeLayer][index]))}</strong>
      <small>${escapeHtml(keycapLabel(encoder.ccw))} / ${escapeHtml(keycapLabel(encoder.cw))} · ${encoder.sensitivity}x</small>
    `;
    elements.testerEncoderGrid.append(card);
  });

  elements.testerKeyGrid.innerHTML = "";
  for (let position = 2; position < POSITION_META.length; position += 1) {
    const keycode = currentKeycode(position);
    const cell = document.createElement("div");
    cell.className = `tester-key-cell ${position === LAYER_KEY_POSITION ? "reserved" : ""} ${state.testerActivePositions.has(position) ? "active" : ""}`;
    cell.innerHTML = `
      <span>${POSITION_META[position].id}</span>
      <strong>${escapeHtml(keycapLabel(keycode))}</strong>
    `;
    elements.testerKeyGrid.append(cell);
  }

  elements.testerHeldKeys.textContent = state.testerHeldKeys.size
    ? [...state.testerHeldKeys].join(" + ")
    : "Press a physical key to see raw matrix scan";

  elements.testerEventLog.innerHTML = "";
  state.testerEvents.slice(0, 12).forEach((entry) => {
    const row = document.createElement("div");
    row.className = "tester-event";
    row.innerHTML = `
      <span>${escapeHtml(entry.kind)}</span>
      <strong>${escapeHtml(entry.label)}</strong>
      <small>${escapeHtml(entry.match)}</small>
    `;
    elements.testerEventLog.append(row);
  });
}

function clearTesterEncoderPulse(index) {
  window.setTimeout(() => {
    state.testerActiveEncoders.delete(index);
    renderTester();
  }, 380);
}

function animateKeyboardEncoder(index, direction, value = 1) {
  const button = document.querySelector(`.encoder-switch[data-position="${index}"]`);
  if (!button) {
    return;
  }

  const steps = Math.max(1, Math.min(4, Math.abs(Number(value) || 1)));
  const delta = (direction === "CCW" ? -1 : 1) * 28 * steps;
  state.encoderAngles[index] = (state.encoderAngles[index] + delta) % 360;
  button.style.setProperty("--encoder-angle", `${state.encoderAngles[index]}deg`);
  button.classList.add("encoder-turning");
  button.classList.toggle("encoder-turning-cw", direction === "CW");
  button.classList.toggle("encoder-turning-ccw", direction === "CCW");

  window.clearTimeout(encoderMotionTimers.get(index));
  encoderMotionTimers.set(index, window.setTimeout(() => {
    button.classList.remove("encoder-turning", "encoder-turning-cw", "encoder-turning-ccw");
    encoderMotionTimers.delete(index);
  }, 320));
}

function addTesterEvent(kind, label, match) {
  state.testerEvents.unshift({ kind, label, match });
  state.testerEvents = state.testerEvents.slice(0, 24);
  elements.testerLastInput.textContent = `${kind} ${label}`;
  renderTester();
}

function recordTesterMatrixEvent(position, pressed, layer = state.activeLayer) {
  if (!Number.isInteger(position) || position < 0 || position >= POSITION_META.length) {
    return;
  }

  const meta = POSITION_META[position];
  const mapped = keycapLabel(state.config.layers[layer]?.[position] || currentKeycode(position));
  const label = position < 2 ? `ENC ${position} press` : meta.id;

  if (pressed) {
    state.testerActivePositions.add(position);
    state.testerHeldKeys.add(label);
  } else {
    state.testerActivePositions.delete(position);
    state.testerHeldKeys.delete(label);
  }

  addTesterEvent(pressed ? "DOWN" : "UP", label, `${meta.id} -> ${mapped}`);
}

function recordTesterEncoderEvent(index, direction, value = 0, layer = state.activeLayer) {
  if (!Number.isInteger(index) || index < 0 || index >= state.config.encoders.length) {
    return;
  }

  const encoder = state.config.encoders[index];
  const mapped = keycapLabel(direction === "CW" ? encoder.cw : encoder.ccw);
  state.testerActiveEncoders.add(index);
  animateKeyboardEncoder(index, direction, value);
  addTesterEvent(direction, `ENC ${index}`, `${mapped} (${value})`);
  clearTesterEncoderPulse(index);
}

function saveEncoderSensitivity(index, value) {
  encoderSensitivityWriteChains[index] = encoderSensitivityWriteChains[index]
    .catch(() => {})
    .then(async () => {
      if (!state.transport || !state.authorized) {
        setSaveButtonState("dirty");
        return;
      }

      setSaveButtonState("saving");
      const setReply = await sendCommand("set_encoder_sensitivity", {
        encoder: index,
        sensitivity: value,
      });
      if (!setReply.ok) {
        throw new Error(setReply.error || "Encoder sensitivity update failed");
      }

      const saveReply = await sendCommand("save_config");
      if (!saveReply.ok) {
        throw new Error(saveReply.error || "Encoder sensitivity save failed");
      }

      setSaveButtonState("saved");
      logWire("SAVE", `Encoder ${index + 1} sensitivity ${value}x`);
      showToast(`Encoder ${index + 1} sensitivity saved at ${value}x`);
    })
    .catch((error) => {
      setSaveButtonState("dirty");
      showToast(error.message);
    });
}

async function sendDisplayTest(mode) {
  try {
    const reply = await sendCommand("display_test", { mode });
    if (reply.ok) {
      showToast(`Display ${mode}`);
    }
  } catch (error) {
    showToast(error.message);
  }
}

async function sendRgbTest(mode) {
  const colors = {
    red: { hue: 0, saturation: 100 },
    green: { hue: 120, saturation: 100 },
    blue: { hue: 240, saturation: 100 },
  };

  try {
    if (mode === "reset") {
      await syncRgb(true);
      showToast("RGB reset");
      return;
    }

    const color = colors[mode];
    if (!color) {
      return;
    }

    await sendCommand("set_rgb", {
      on: true,
      reactive: false,
      layerMode: false,
      effect: 0,
      hue: color.hue,
      saturation: color.saturation,
      brightness: Math.max(state.config.rgb.brightness, 35),
    });
    showToast(`RGB ${mode}`);
  } catch (error) {
    showToast(error.message);
  }
}

function displayDirtyKey(target, layer, index = 0) {
  return `${target}:${layer}:${index}`;
}

function markDisplayChanges(changes) {
  changes.forEach(({ target, layer, index = 0 }) => {
    state.displayDirty.add(displayDirtyKey(target, layer, index));
  });
}

async function sendDisplayLabel(target, layer, index = 0) {
  const display = state.config.display;
  const text = target === "layer"
    ? display.layers[layer]
    : target === "encoder"
      ? display.encoders[layer][index]
      : display.keys[layer][index];

  return sendCommand("set_display_label", { target, layer, index, text });
}

async function updateDisplayLabel(target, layer, index, value, options = {}) {
  const { custom = true, renderControls = true, save = true } = options;
  if (isDeviceLayer(layer)) {
    showToast("Device layer labels are fixed");
    return;
  }

  const fallback = displayAutoLabel(state.config, target, layer, index);
  const text = normalizeDisplayText(value, fallback);
  setDisplayLabelValue(state.config, target, layer, index, text);
  setDisplayCustomValue(state.config, target, layer, index, custom);

  state.displayDirty.add(displayDirtyKey(target, layer, index));
  persistConfig();
  renderLayerTabs();
  renderKeyboard();
  renderSelection();
  renderRgbControls();
  if (renderControls) {
    renderDisplayControls();
  }

  if (state.transport && state.authorized) {
    try {
      await sendDisplayLabel(target, layer, index);
      state.displayDirty.delete(displayDirtyKey(target, layer, index));
      if (save) {
        scheduleBoardSave("display", 1200);
      }
    } catch (error) {
      showToast(error.message);
    }
  }
}

function queueDisplayLabelUpdate(target, layer, index, value) {
  const key = displayDirtyKey(target, layer, index);
  window.clearTimeout(displayInputTimers.get(key));
  displayInputTimers.set(key, window.setTimeout(() => {
    displayInputTimers.delete(key);
    updateDisplayLabel(target, layer, index, value, { custom: true, renderControls: false, save: true });
  }, 260));
}

function commitDisplayLabelUpdate(target, layer, index, value) {
  const key = displayDirtyKey(target, layer, index);
  window.clearTimeout(displayInputTimers.get(key));
  displayInputTimers.delete(key);
  updateDisplayLabel(target, layer, index, value, { custom: true, renderControls: true });
}

async function resetDisplayLabelToAuto(target, layer, index) {
  await updateDisplayLabel(target, layer, index, displayAutoLabel(state.config, target, layer, index), {
    custom: false,
    renderControls: true,
  });
}

async function syncDisplayChanges(changes) {
  if (!state.transport || !state.authorized) {
    return;
  }

  for (const { target, layer, index = 0 } of changes) {
    try {
      await sendDisplayLabel(target, layer, index);
      state.displayDirty.delete(displayDirtyKey(target, layer, index));
    } catch (error) {
      showToast(error.message);
      break;
    }
  }
}

function renderDisplayControls() {
  if (!elements.displayLayerName || !elements.displayEncoderLabels || !elements.displayKeyLabels) {
    return;
  }

  const power = normalizePower(state.config.power);
  state.config.power = power;
  if (elements.displayPower) {
    elements.displayPower.checked = power.displayOn;
  }
  if (elements.displayTimeout) {
    elements.displayTimeout.value = String(power.displayTimeout);
  }
  if (elements.displayTimeoutValue) {
    elements.displayTimeoutValue.textContent = `${power.displayTimeout} min`;
  }

  const layer = state.activeLayer;
  const locked = isDeviceLayer(layer);
  elements.displayEditorNote.textContent = locked
    ? "Layer 6 is fixed for BLE profile pairing and RGB controls."
    : "Names are written to the TFT display and capped at 7 characters.";
  elements.displayLayerName.value = state.config.display.layers[layer];
  elements.displayLayerName.disabled = locked;
  elements.displayLayerName.dataset.layer = String(layer);

  elements.displayEncoderLabels.innerHTML = "";
  state.config.display.encoders[layer].forEach((label, index) => {
    const encoder = state.config.encoders[index];
    const row = document.createElement("div");
    row.className = "display-label-row";
    const isCustom = displayCustomValue(state.config, "encoder", layer, index);
    row.innerHTML = `
      <span>Encoder ${index + 1}</span>
      <code class="display-map-current" title="Current encoder map">CCW ${escapeHtml(displayTextForKeycode(encoder.ccw, state.config.macros))} / CW ${escapeHtml(displayTextForKeycode(encoder.cw, state.config.macros))}</code>
      <input data-target="encoder" data-index="${index}" type="text" maxlength="${DISPLAY_TEXT_MAX}" value="${escapeHtml(label)}" aria-label="Encoder ${index + 1} display text" ${locked ? "disabled" : ""} />
      <button class="display-auto-button ${isCustom ? "" : "active"}" data-target="encoder" data-index="${index}" type="button" ${locked || !isCustom ? "disabled" : ""}>Auto</button>
    `;
    elements.displayEncoderLabels.append(row);
  });

  elements.displayKeyLabels.innerHTML = "";
  elements.displayKeyLabels.classList.add("display-key-matrix");
  state.config.display.keys[layer].forEach((label, index) => {
    const position = index + 2;
    const keycode = state.config.layers[layer][position];
    const row = document.createElement("div");
    row.className = `display-key-cell ${position === LAYER_KEY_POSITION ? "reserved" : ""}`;
    const isCustom = displayCustomValue(state.config, "key", layer, index);
    row.innerHTML = `
      <div class="display-key-head">
        <span>${POSITION_META[position].id}</span>
        <small>${escapeHtml(POSITION_META[position].role)}</small>
      </div>
      <code class="display-map-current" title="Current key map">${escapeHtml(keyLabel(keycode))}</code>
      <input data-target="key" data-index="${index}" type="text" maxlength="${DISPLAY_TEXT_MAX}" value="${escapeHtml(label)}" aria-label="${POSITION_META[position].id} display text" ${locked ? "disabled" : ""} />
      <button class="display-auto-button ${isCustom ? "" : "active"}" data-target="key" data-index="${index}" type="button" ${locked || !isCustom ? "disabled" : ""}>Auto</button>
    `;
    elements.displayKeyLabels.append(row);
  });

  elements.displayEncoderLabels.querySelectorAll("input").forEach((input) => {
    input.addEventListener("input", () => {
      input.value = sanitizeDisplayInput(input.value);
      queueDisplayLabelUpdate("encoder", layer, Number(input.dataset.index), input.value);
    });
    input.addEventListener("change", () => commitDisplayLabelUpdate("encoder", layer, Number(input.dataset.index), input.value));
  });

  elements.displayKeyLabels.querySelectorAll("input").forEach((input) => {
    input.addEventListener("input", () => {
      input.value = sanitizeDisplayInput(input.value);
      queueDisplayLabelUpdate("key", layer, Number(input.dataset.index), input.value);
    });
    input.addEventListener("change", () => commitDisplayLabelUpdate("key", layer, Number(input.dataset.index), input.value));
  });

  [...elements.displayEncoderLabels.querySelectorAll(".display-auto-button"), ...elements.displayKeyLabels.querySelectorAll(".display-auto-button")].forEach((button) => {
    button.addEventListener("click", () => (
      resetDisplayLabelToAuto(button.dataset.target, layer, Number(button.dataset.index))
    ));
  });
}

function renderCategoryTabs() {
  elements.categoryTabs.innerHTML = "";
  Object.keys(KEYCODE_GROUPS).forEach((group) => {
    const button = document.createElement("button");
    button.type = "button";
    button.className = `category-tab ${state.activeCategory === group ? "active" : ""}`;
    button.setAttribute("role", "tab");
    button.setAttribute("aria-selected", String(state.activeCategory === group));
    button.innerHTML = `<span class="category-icon">${escapeHtml(CATEGORY_ICONS[group] || group.slice(0, 3))}</span>${escapeHtml(group)}`;
    button.addEventListener("click", () => {
      state.activeCategory = group;
      renderCategoryTabs();
      renderKeyLibrary();
    });
    elements.categoryTabs.append(button);
  });
}

function setLibraryExpanded(expanded) {
  state.libraryExpanded = Boolean(expanded);
  elements.libraryPanel.classList.toggle("expanded", state.libraryExpanded);
  elements.libraryBackdrop.hidden = !state.libraryExpanded;
  elements.libraryExpandButton.hidden = state.libraryExpanded;
  elements.libraryCloseButton.hidden = !state.libraryExpanded;
  elements.libraryExpandButton.setAttribute("aria-expanded", String(state.libraryExpanded));
  document.body.classList.toggle("library-overlay-open", state.libraryExpanded);

  if (state.libraryExpanded) {
    window.setTimeout(() => elements.keySearch.focus(), 0);
  }
}

function macroKeycodes() {
  return Array.from({ length: state.config.macros.length }, (_, index) => `M${index}`);
}

function visibleKeycodes() {
  const query = elements.keySearch.value.trim().toUpperCase();
  const seed = query
    ? ALL_KEYCODES
    : state.activeCategory === "Macro"
      ? macroKeycodes()
      : (KEYCODE_GROUPS[state.activeCategory] || KEYCODE_GROUPS.Letters);
  return seed.filter((keycode) => `${keycode} ${keyLabel(keycode)}`.toUpperCase().includes(query));
}

function renderKeyLibrary() {
  elements.keyLibrary.innerHTML = "";
  elements.keyLibrary.classList.toggle("quick-macro-library", state.activeCategory === "Quick Macros");
  const layerKeySelected = state.selectedPosition === LAYER_KEY_POSITION;
  const layerLocked = isDeviceLayer();
  elements.keySearch.disabled = layerKeySelected || layerLocked;
  elements.categoryTabs.querySelectorAll("button").forEach((button) => {
    button.disabled = layerKeySelected || layerLocked;
  });

  if (layerLocked) {
    const note = document.createElement("div");
    note.className = "reserved-note";
    note.innerHTML = `<strong>Device layer locked</strong><span>Layer 6 keeps BLE profile and RGB controls safe.</span>`;
    elements.keyLibrary.append(note);
    return;
  }

  if (layerKeySelected) {
    const note = document.createElement("div");
    note.className = "reserved-note";
    note.innerHTML = `<strong>Fixed layer key</strong><span>1.3 always cycles layers on ${DEVICE_NAME}.</span>`;
    elements.keyLibrary.append(note);
    return;
  }

  if (state.activeCategory === "Quick Macros") {
    renderQuickMacroLibrary();
    return;
  }

  visibleKeycodes().forEach((keycode) => {
    const button = document.createElement("button");
    button.type = "button";
    const glyph = keyGlyph(keycode);
    const hasGlyph = Boolean(KEY_GLYPHS[keycode] || MACOS_SHORTCUTS[keycode]);
    const wideGlyph = hasGlyph && glyph.length > 4;
    const profileOnly = isProfileOnlyKeycode(keycode) && !MACOS_SHORTCUTS[keycode];
    button.className = `library-key ${keycode === currentKeycode() ? "current" : ""} ${profileOnly ? "profile-only" : ""} ${hasGlyph ? "has-glyph" : ""} ${wideGlyph ? "wide-glyph" : ""}`;
    const code = document.createElement("strong");
    code.textContent = glyph;
    const label = document.createElement("span");
    label.textContent = keyLabel(keycode);
    button.append(code, label);
    button.addEventListener("click", () => assignKeycode(keycode));
    elements.keyLibrary.append(button);
  });
}

function macroMatchesShortcut(macro, shortcut) {
  if (!macro || !shortcut) {
    return false;
  }

  const sequence = shortcut.sequence.map(normalizeMacroStep);
  return macro.sequence.length === sequence.length &&
    macro.sequence.every((token, index) => token === sequence[index]);
}

function currentShortcutMatches(shortcut) {
  const macroIndex = macroIndexFromKeycode(currentKeycode());
  return macroIndex >= 0 && macroMatchesShortcut(state.config.macros[macroIndex], shortcut);
}

function renderQuickMacroLibrary() {
  const query = elements.keySearch.value.trim().toUpperCase();
  const presets = QUICK_MACRO_PRESETS.filter((preset) => (
    `${preset.label} ${preset.shortcut} ${preset.glyph}`.toUpperCase().includes(query)
  ));

  presets.forEach((preset) => {
    const button = document.createElement("button");
    button.type = "button";
    button.className = `quick-macro-card ${currentShortcutMatches(preset) ? "current" : ""}`;
    button.innerHTML = `
      <span class="quick-macro-glyph">${escapeHtml(preset.glyph)}</span>
      <span class="quick-macro-copy">
        <strong>${escapeHtml(preset.label)}</strong>
        <small>${escapeHtml(preset.shortcut)}</small>
      </span>
      <span class="quick-macro-add" aria-hidden="true">+</span>
    `;
    button.addEventListener("click", () => assignShortcutPreset(preset));
    elements.keyLibrary.append(button);
  });

  if (presets.length === 0) {
    const note = document.createElement("div");
    note.className = "reserved-note";
    note.innerHTML = "<strong>No matching quick macro</strong><span>Try searching for a shortcut, app action, or command.</span>";
    elements.keyLibrary.append(note);
  }
}

async function assignKeycode(keycode) {
  if (MACOS_SHORTCUTS[keycode]) {
    await assignMacOSShortcut(keycode);
    return;
  }

  const layer = state.activeLayer;
  const position = state.selectedPosition;
  if (isDeviceLayer(layer)) {
    showToast("Layer 6 is reserved for device controls");
    return;
  }

  if (position === LAYER_KEY_POSITION) {
    showToast("1.3 is reserved for layer cycling");
    return;
  }

  state.config.layers[layer][position] = keycode;
  const displayChanges = refreshAutoDisplayLabels(state.config);
  markDisplayChanges(displayChanges);
  persistConfig();
  renderKeyboard();
  renderSelection();
  renderKeyLibrary();
  renderDisplayControls();
  renderTester();

  if (isProfileOnlyKeycode(keycode)) {
    await syncDisplayChanges(displayChanges);
    scheduleBoardSave("display", 1200);
    showToast(`${keyLabel(keycode)} saved to profile`);
    addLog("MAP ", `${POSITION_META[position].id} -> ${keycode} (profile only)`);
    return;
  }

  try {
    const reply = await sendCommand("set_key", { layer, position, keycode });
    if (reply.ok) {
      await syncDisplayChanges(displayChanges);
      scheduleBoardSave("keymap");
      showToast(`${POSITION_META[position].id} -> ${keycode}`);
    }
  } catch (error) {
    showToast(error.message);
  }
}

function findMacroSlotForShortcut(shortcut) {
  const normalizedSequence = shortcut.sequence.map(normalizeMacroStep);
  const existing = state.config.macros.findIndex((macro) => (
    macro.sequence.length === normalizedSequence.length &&
    macro.sequence.every((token, index) => token === normalizedSequence[index])
  ));
  if (existing >= 0) {
    return existing;
  }

  const empty = state.config.macros.findIndex((macro, index) => (
    macroIsDefaultEmpty(index) && !macroIsAssigned(index)
  ));
  if (empty >= 0) {
    return empty;
  }

  if (state.config.macros.length >= MAX_MACRO_SLOT_COUNT) {
    return -1;
  }

  const index = state.config.macros.length;
  state.config.macros.push({ name: `M${index}`, sequence: [] });
  return index;
}

async function assignShortcutPreset(shortcut) {
  const layer = state.activeLayer;
  const position = state.selectedPosition;
  if (!shortcut || isDeviceLayer(layer) || position === LAYER_KEY_POSITION) {
    showToast(isDeviceLayer(layer) ? "Layer 6 is reserved for device controls" : "Select a remappable key");
    return;
  }

  ensureMacroSlots();
  const macroIndex = findMacroSlotForShortcut(shortcut);
  if (macroIndex < 0) {
    showToast("No free macro slot is available");
    return;
  }

  state.config.macros[macroIndex] = {
    name: shortcut.label.slice(0, 23),
    sequence: shortcut.sequence.map(normalizeMacroStep),
  };
  state.config.layers[layer][position] = `M${macroIndex}`;
  const displayChanges = refreshAutoDisplayLabels(state.config);
  markDisplayChanges(displayChanges);
  persistConfig();
  renderKeyboard();
  renderSelection();
  renderKeyLibrary();
  renderMacros();
  renderDisplayControls();
  renderTester();

  if (state.transport && state.authorized) {
    try {
      await sendMacro(macroIndex);
      await sendCommand("set_key", { layer, position, keycode: `M${macroIndex}` });
      await syncDisplayChanges(displayChanges);
      await sendCommand("save_config");
      setSaveButtonState("saved");
      showToast(`${shortcut.label} assigned`);
      return;
    } catch (error) {
      setSaveButtonState("dirty");
      showToast(error.message);
      return;
    }
  }

  setSaveButtonState("dirty");
  showToast(`${shortcut.label} added locally`);
}

async function assignMacOSShortcut(keycode) {
  await assignShortcutPreset(MACOS_SHORTCUTS[keycode]);
}

function encoderDirectionMatchesPreset(keycode, preset, direction) {
  if (preset[direction]) {
    return keycode === preset[direction];
  }

  const sequence = preset[`${direction}Sequence`];
  const macroIndex = macroIndexFromKeycode(keycode);
  return macroIndex >= 0 && macroMatchesShortcut(state.config.macros[macroIndex], {
    sequence: sequence || [],
  });
}

function encoderMatchesPreset(encoder, preset) {
  return encoderDirectionMatchesPreset(encoder.ccw, preset, "ccw") &&
    encoderDirectionMatchesPreset(encoder.cw, preset, "cw");
}

function renderEncoderGrid() {
  elements.encoderGrid.innerHTML = "";
  state.config.encoders.forEach((encoder, index) => {
    const pressKeycode = state.config.layers[state.activeLayer][index];
    const activePreset = ENCODER_PRESETS.find((preset) => encoderMatchesPreset(encoder, preset));
    const layerLocked = isDeviceLayer();
    const card = document.createElement("section");
    card.className = "encoder-card";
    card.innerHTML = `
      <div class="encoder-card-head">
        <div>
          <span class="section-label">Encoder ${index + 1}</span>
          <h3>${escapeHtml(activePreset?.label || "Custom")}</h3>
        </div>
        <span class="status-pill">${escapeHtml(layerTabLabel(state.activeLayer))}</span>
      </div>
      <label class="mapping-row encoder-press-row">
        <span>Press</span>
        ${pressSelectMarkup(pressKeycode, layerLocked)}
      </label>
      <label class="mapping-row">
        <span>CCW</span>
        ${encoderSelectMarkup("ccw", encoder.ccw)}
      </label>
      <label class="mapping-row">
        <span>CW</span>
        ${encoderSelectMarkup("cw", encoder.cw)}
      </label>
      <label class="encoder-sensitivity-row">
        <span>
          <strong>Sensitivity</strong>
          <small>Output per detent</small>
        </span>
        <input
          data-role="sensitivity"
          type="range"
          min="1"
          max="4"
          step="1"
          value="${encoder.sensitivity}"
          aria-label="Encoder ${index + 1} sensitivity"
        />
        <output>${encoder.sensitivity}x</output>
      </label>
      <div class="encoder-presets encoder-preset-gallery" aria-label="Encoder ${index} presets">
        ${ENCODER_PRESETS.map((preset) => `
          <button
            class="preset-button ${activePreset?.label === preset.label ? "active" : ""}"
            data-preset="${preset.label}"
            type="button"
            title="${escapeHtml(preset.group)} preset"
          >
            <span>${escapeHtml(preset.icon)}</span>
            <strong>${escapeHtml(preset.label)}</strong>
          </button>
        `).join("")}
      </div>
    `;

    const pressSelect = card.querySelector("select[data-role='press']");
    pressSelect?.addEventListener("change", () => updateEncoderPress(index, pressSelect.value));

    card.querySelectorAll("select[data-direction]").forEach((select) => {
      select.addEventListener("change", async () => {
        const direction = select.dataset.direction;
        const keycode = normalizeKeycode(select.value);
        state.config.encoders[index][direction] = keycode;
        const displayChanges = refreshAutoDisplayLabels(state.config);
        markDisplayChanges(displayChanges);
        persistConfig();
        renderDisplayControls();
        renderTester();

        try {
          await sendCommand("set_encoder", { encoder: index, direction, keycode });
          await syncDisplayChanges(displayChanges);
          scheduleBoardSave("encoder");
        } catch (error) {
          showToast(error.message);
        }
      });
    });

    const sensitivity = card.querySelector("input[data-role='sensitivity']");
    const sensitivityOutput = card.querySelector(".encoder-sensitivity-row output");
    const updateSensitivityState = () => {
      const value = normalizeRange(sensitivity.value, 1, 4, 1);
      state.config.encoders[index].sensitivity = value;
      sensitivityOutput.textContent = `${value}x`;
      persistConfig();
      renderTester();
      return value;
    };
    sensitivity?.addEventListener("input", updateSensitivityState);
    sensitivity?.addEventListener("change", () => {
      saveEncoderSensitivity(index, updateSensitivityState());
    });

    card.querySelectorAll(".preset-button").forEach((button) => {
      button.addEventListener("click", () => applyEncoderPreset(index, button.dataset.preset));
    });

    elements.encoderGrid.append(card);
  });
}

async function updateEncoderPress(index, keycode) {
  const layer = state.activeLayer;
  if (isDeviceLayer(layer)) {
    showToast("Device layer encoder switches are fixed");
    return;
  }

  state.config.layers[layer][index] = keycode;
  const displayChanges = refreshAutoDisplayLabels(state.config);
  markDisplayChanges(displayChanges);
  persistConfig();
  renderKeyboard();
  renderSelection();
  renderKeyLibrary();
  renderDisplayControls();
  renderTester();

  try {
    const reply = await sendCommand("set_key", { layer, position: index, keycode });
    if (reply.ok) {
      await syncDisplayChanges(displayChanges);
      scheduleBoardSave("encoder press");
      showToast(`Encoder ${index + 1} press -> ${keyLabel(keycode)}`);
    }
  } catch (error) {
    showToast(error.message);
  }
}

function renderMacros() {
  ensureMacroSlots();
  state.selectedMacro = Math.min(state.selectedMacro, state.config.macros.length - 1);
  elements.macroList.innerHTML = "";
  state.config.macros.forEach((macro, index) => {
    const button = document.createElement("button");
    button.type = "button";
    button.className = `macro-slot ${index === state.selectedMacro ? "active" : ""}`;
    const slotCode = document.createElement("code");
    slotCode.textContent = `M${index}`;
    const slotName = document.createElement("span");
    slotName.textContent = macro.name || `M${index}`;
    button.append(slotCode, slotName);
    button.addEventListener("click", () => {
      saveMacroDraft();
      state.selectedMacro = index;
      renderMacros();
    });
    elements.macroList.append(button);
  });

  const macro = state.config.macros[state.selectedMacro];
  elements.macroName.value = macro.name;
  elements.macroSequence.value = macro.sequence.join(" ");
  elements.macroAddButton.disabled = state.config.macros.length >= MAX_MACRO_SLOT_COUNT;
  elements.macroAddButton.textContent = elements.macroAddButton.disabled ? "Max Macros" : "+ New Macro";
  elements.macroAddButton.title = elements.macroAddButton.disabled
    ? `TAC-W supports ${MAX_MACRO_SLOT_COUNT} firmware macro slots`
    : "Create a new firmware macro slot";
  const selectedAssigned = macroIsAssigned(state.selectedMacro);
  const selectedCoreSlot = state.selectedMacro < DEFAULT_MACRO_SLOT_COUNT;
  elements.macroDeleteButton.textContent = selectedCoreSlot || selectedAssigned ? "Clear Slot" : "Delete Macro";
  elements.macroDeleteButton.title = selectedAssigned
    ? "This macro is assigned to a key, so it will be cleared without removing the slot"
    : "Clear this macro from the profile and keyboard";

  elements.macroKeyPalette.innerHTML = "";
  MACRO_INSERT_KEYS.forEach((keycode) => {
    const button = document.createElement("button");
    button.type = "button";
    button.className = "macro-key";
    button.textContent = keycode;
    button.addEventListener("click", () => insertMacroToken(keycode));
    elements.macroKeyPalette.append(button);
  });
}

function ensureMacroSlots() {
  if (!Array.isArray(state.config.macros)) {
    state.config.macros = [];
  }
  while (state.config.macros.length < DEFAULT_MACRO_SLOT_COUNT) {
    const index = state.config.macros.length;
    state.config.macros.push({ name: `M${index}`, sequence: [] });
  }
  if (state.config.macros.length > MAX_MACRO_SLOT_COUNT) {
    state.config.macros.length = MAX_MACRO_SLOT_COUNT;
  }
  state.selectedMacro = normalizeRange(state.selectedMacro, 0, state.config.macros.length - 1, 0);
}

function macroIsAssigned(index) {
  return state.config.layers.some((layer) => layer.some((keycode) => macroIndexFromKeycode(keycode) === index));
}

function macroIsDefaultEmpty(index) {
  const macro = state.config.macros[index];
  return Boolean(macro) &&
    (!macro.name || macro.name === `M${index}`) &&
    (!Array.isArray(macro.sequence) || macro.sequence.length === 0);
}

function compactTrailingMacros() {
  while (
    state.config.macros.length > DEFAULT_MACRO_SLOT_COUNT &&
    macroIsDefaultEmpty(state.config.macros.length - 1) &&
    !macroIsAssigned(state.config.macros.length - 1)
  ) {
    state.config.macros.pop();
  }
  state.selectedMacro = normalizeRange(state.selectedMacro, 0, state.config.macros.length - 1, 0);
}

function macroSequenceFromText(text) {
  return text
    .split(/[\s,]+/)
    .map(normalizeMacroStep)
    .filter(Boolean)
    .slice(0, 96);
}

function saveMacroDraft() {
  ensureMacroSlots();
  const macro = state.config.macros[state.selectedMacro];
  macro.name = elements.macroName.value.trim().slice(0, 23) || `M${state.selectedMacro}`;
  macro.sequence = macroSequenceFromText(elements.macroSequence.value);
  persistConfig();
}

function addMacroSlot() {
  ensureMacroSlots();
  if (state.config.macros.length >= MAX_MACRO_SLOT_COUNT) {
    showToast(`TAC-W supports ${MAX_MACRO_SLOT_COUNT} firmware macro slots`);
    return;
  }

  saveMacroDraft();
  const index = state.config.macros.length;
  state.config.macros.push({ name: `M${index}`, sequence: [] });
  state.selectedMacro = index;
  persistConfig();
  renderMacros();
  renderKeyLibrary();
  showToast(`M${index} created`);
}

function insertMacroToken(token) {
  const current = elements.macroSequence.value.trim();
  elements.macroSequence.value = current ? `${current} ${token}` : token;
  saveMacroDraft();
  renderMacros();
}

function addMacroDelay() {
  const delay = normalizeRange(Number.parseInt(elements.macroDelay.value, 10), 10, 5000, 100);
  elements.macroDelay.value = String(delay);
  insertMacroToken(`DLY(${delay})`);
}

function launcherTokensForText(text) {
  const tokens = [];
  for (const character of text) {
    if (/[a-z]/.test(character)) {
      tokens.push(character.toUpperCase());
    } else if (/[A-Z]/.test(character)) {
      tokens.push(`LSHFT+${character}`);
    } else if (/[0-9]/.test(character)) {
      tokens.push(`N${character}`);
    } else if (WINDOWS_LAUNCHER_CHARACTERS[character]) {
      tokens.push(WINDOWS_LAUNCHER_CHARACTERS[character]);
    } else {
      return { tokens: [], unsupported: character };
    }
  }

  return { tokens, unsupported: "" };
}

function launcherMacroName(target) {
  const cleanTarget = target.replace(/^"|"$/g, "");
  try {
    return new URL(cleanTarget).hostname.replace(/^www\./, "").split(".")[0].slice(0, 23) || "Website";
  } catch {
    return cleanTarget
      .split(/[\\/]/)
      .pop()
      .replace(/\.(exe|com|bat|cmd|lnk|app)$/i, "")
      .slice(0, 23) || "Launcher";
  }
}

function renderLauncherPresets() {
  const type = elements.launcherType?.value || "website";
  const presets = type === "app" ? APP_LAUNCHER_PRESETS : WEBSITE_LAUNCHER_PRESETS;
  elements.launcherPreset.innerHTML = "";
  presets.forEach((preset) => {
    const option = document.createElement("option");
    option.value = preset.value;
    option.textContent = preset.label;
    elements.launcherPreset.append(option);
  });
  elements.launcherTarget.placeholder = type === "app" ? "C:\\Path\\App.exe or app command" : "https://example.com";
  elements.launcherTarget.value = presets[0]?.value || "";
}

function normalizeLauncherTarget(type, target) {
  if (type === "website" && !/^[a-z][a-z0-9+.-]*:/i.test(target)) {
    return `https://${target}`;
  }

  const uriLike = /^[a-z][a-z0-9+.-]*:/i.test(target);
  const windowsDrivePath = /^[a-z]:[\\/]/i.test(target);
  if (type === "app" && /\s/.test(target) && !/^".*"$/.test(target) && (!uriLike || windowsDrivePath)) {
    return `"${target}"`;
  }

  return target;
}

async function buildLauncherMacro() {
  const type = elements.launcherType?.value || "website";
  let target = elements.launcherTarget.value.trim();
  if (!target) {
    showToast(type === "app" ? "Enter an app path or command" : "Enter a website address");
    elements.launcherTarget.focus();
    return;
  }

  target = normalizeLauncherTarget(type, target);
  elements.launcherTarget.value = target;

  const encoded = launcherTokensForText(target);
  if (encoded.unsupported) {
    showToast(`Unsupported launcher character: ${encoded.unsupported}`);
    return;
  }

  const sequence = ["LGUI+R", "DLY(220)", ...encoded.tokens, "DLY(120)", "ENTER"];
  if (sequence.join(" ").length > MAX_FIRMWARE_MACRO_SEQUENCE_LENGTH) {
    showToast("This address or command is too long for one device macro");
    return;
  }

  ensureMacroSlots();
  const index = state.selectedMacro;
  const currentName = state.config.macros[index].name;
  state.config.macros[index] = {
    name: !currentName || currentName === `M${index}` ? launcherMacroName(target) : currentName,
    sequence,
  };
  persistConfig();
  renderMacros();
  renderKeyLibrary();
  await saveMacroProfile();
}

function clearCurrentMacro() {
  state.config.macros[state.selectedMacro] = {
    name: `M${state.selectedMacro}`,
    sequence: [],
  };
  persistConfig();
  renderMacros();
  showToast(`M${state.selectedMacro} cleared`);
}

async function deleteCurrentMacro() {
  ensureMacroSlots();
  const index = state.selectedMacro;
  const slotName = `M${index}`;
  const removable = index >= DEFAULT_MACRO_SLOT_COUNT && !macroIsAssigned(index);

  state.config.macros[index] = { name: slotName, sequence: [] };
  if (removable) {
    compactTrailingMacros();
  }

  const displayChanges = refreshAutoDisplayLabels(state.config);
  markDisplayChanges(displayChanges);
  persistConfig();
  renderMacros();
  renderKeyboard();
  renderSelection();
  renderKeyLibrary();
  renderDisplayControls();

  if (state.transport && state.authorized) {
    try {
      setSaveButtonState("saving");
      await sendMacro(index);
      await syncDisplayChanges(displayChanges);
      const reply = await sendCommand("save_config");
      if (reply.ok) {
        setSaveButtonState("saved");
      }
    } catch (error) {
      setSaveButtonState("dirty");
      showToast(error.message);
      return;
    }
  }

  showToast(removable ? `${slotName} deleted` : `${slotName} cleared`);
}

async function sendMacro(index) {
  const macro = state.config.macros[index] || { name: `M${index}`, sequence: [] };
  return sendCommand("set_macro", {
    index,
    name: macro.name || `M${index}`,
    sequence: macro.sequence.join(" "),
  });
}

async function saveMacroProfile() {
  saveMacroDraft();
  const displayChanges = refreshAutoDisplayLabels(state.config);
  markDisplayChanges(displayChanges);
  persistConfig();
  renderMacros();
  renderKeyboard();
  renderSelection();
  renderKeyLibrary();
  renderDisplayControls();
  addLog("MAC ", `Saved M${state.selectedMacro} to local profile`);

  if (state.transport && state.authorized) {
    try {
      setSaveButtonState("saving");
      await sendMacro(state.selectedMacro);
      await syncDisplayChanges(displayChanges);
      const reply = await sendCommand("save_config");
      if (reply.ok) {
        setSaveButtonState("saved");
      }
      showToast(`M${state.selectedMacro} saved to TAC-W`);
      return;
    } catch (error) {
      setSaveButtonState("dirty");
      showToast(error.message);
      return;
    }
  }

  showToast(`M${state.selectedMacro} saved`);
}

function updateMacroDraftUi() {
  saveMacroDraft();
  const displayChanges = refreshAutoDisplayLabels(state.config);
  markDisplayChanges(displayChanges);
  persistConfig();
  const activeName = elements.macroList.querySelector(".macro-slot.active span");
  if (activeName) {
    activeName.textContent = state.config.macros[state.selectedMacro].name;
  }
  renderKeyboard();
  renderSelection();
  renderKeyLibrary();
  renderDisplayControls();
}

async function startMacroRecording() {
  saveMacroDraft();
  const macro = state.config.macros[state.selectedMacro];
  state.recording = {
    heldModifiers: new Map(),
    previous: clone(macro.sequence),
    sequence: [],
    lastAt: 0,
  };
  elements.recorderTitle.textContent = `M${state.selectedMacro} ${macro.name}`;
  renderRecordedKeys();
  elements.macroRecorder.hidden = false;
  elements.macroRecorder.focus();

  try {
    await elements.macroRecorder.requestFullscreen();
  } catch {
    // Fullscreen can be blocked by the browser; the overlay still records.
  }
}

function modifierTokenFromEvent(event) {
  const modifierCodes = {
    AltLeft: "LALT",
    AltRight: "RALT",
    ControlLeft: "LCTRL",
    ControlRight: "RCTRL",
    MetaLeft: "LGUI",
    MetaRight: "RGUI",
    ShiftLeft: "LSHFT",
    ShiftRight: "RSHFT",
  };
  return modifierCodes[event.code] || "";
}

function macroTokenFromEvent(event) {
  const codeMap = {
    ArrowDown: "DOWN",
    ArrowLeft: "LEFT",
    ArrowRight: "RIGHT",
    ArrowUp: "UP",
    Backspace: "BSPC",
    CapsLock: "CAPS",
    Delete: "DEL",
    End: "END",
    Enter: "ENTER",
    Escape: "ESC",
    Home: "HOME",
    Insert: "INS",
    ContextMenu: "K_APP",
    LaunchApp1: "C_AL_MY_COMPUTER",
    LaunchApp2: "C_AL_CALC",
    LaunchMail: "C_AL_MAIL",
    MediaPlayPause: "C_PLAY_PAUSE",
    MediaSelect: "C_MEDIA_STEP",
    MediaStop: "C_STOP",
    MediaTrackNext: "C_NEXT",
    MediaTrackPrevious: "C_PREV",
    Numpad0: "KP_N0",
    Numpad1: "KP_N1",
    Numpad2: "KP_N2",
    Numpad3: "KP_N3",
    Numpad4: "KP_N4",
    Numpad5: "KP_N5",
    Numpad6: "KP_N6",
    Numpad7: "KP_N7",
    Numpad8: "KP_N8",
    Numpad9: "KP_N9",
    NumpadAdd: "KP_PLUS",
    NumpadClear: "KP_CLEAR",
    NumpadComma: "KP_COMMA",
    NumpadDecimal: "KP_DOT",
    NumpadDivide: "KP_DIVIDE",
    NumpadEnter: "KP_ENTER",
    NumpadEqual: "KP_EQUAL",
    NumpadMultiply: "KP_MULTIPLY",
    NumpadParenLeft: "KP_LPAR",
    NumpadParenRight: "KP_RPAR",
    NumpadSubtract: "KP_MINUS",
    NumLock: "KP_NUM",
    Pause: "PAUSE_BREAK",
    PageDown: "PG_DN",
    PageUp: "PG_UP",
    PrintScreen: "PSCRN",
    ScrollLock: "SLCK",
    Space: "SPACE",
    Tab: "TAB",
    VolumeDown: "C_VOL_DN",
    VolumeMute: "C_MUTE",
    VolumeUp: "C_VOL_UP",
    WakeUp: "SYSTEM_WAKE_UP",
  };

  if (codeMap[event.code]) {
    return codeMap[event.code];
  }

  if (/^Key[A-Z]$/.test(event.code)) {
    return event.code.slice(3);
  }

  if (/^Digit\d$/.test(event.code)) {
    return `N${event.code.slice(5)}`;
  }

  if (/^F([1-9]|1[0-2])$/.test(event.code)) {
    return event.code;
  }

  if (event.key.length === 1) {
    if (/^[a-z]$/i.test(event.key)) {
      return event.key.toUpperCase();
    }
    if (/^\d$/.test(event.key)) {
      return `N${event.key}`;
    }
    const symbolMap = {
      "!": "EXCL",
      "@": "AT",
      "#": "HASH",
      "$": "DLLR",
      "%": "PRCNT",
      "^": "CARET",
      "&": "AMPS",
      "*": "ASTRK",
      "(": "LPAR",
      ")": "RPAR",
      "-": "MINUS",
      "=": "EQUAL",
      "+": "PLUS",
      " ": "SPACE",
      "[": "LBKT",
      "]": "RBKT",
      "\\": "BSLH",
      ";": "SEMI",
      "'": "SQT",
      ",": "COMMA",
      ".": "DOT",
      "/": "FSLH",
      "`": "GRAVE",
    };
    return symbolMap[event.key] || "";
  }

  return "";
}

function addRecordedToken(token) {
  if (!token || state.recording.sequence.length >= 96) {
    return;
  }

  const now = performance.now();
  if (state.recording.lastAt > 0) {
    const delay = Math.min(2000, Math.round((now - state.recording.lastAt) / 10) * 10);
    if (delay >= 120) {
      state.recording.sequence.push(`DLY(${delay})`);
    }
  }
  state.recording.sequence.push(normalizeMacroStep(token));
  state.recording.lastAt = now;
  renderRecordedKeys();
}

function handleMacroRecordKey(event) {
  if (!state.recording) {
    return;
  }

  event.preventDefault();
  event.stopPropagation();

  if (event.repeat) {
    return;
  }

  const modifier = modifierTokenFromEvent(event);
  if (modifier) {
    if (!state.recording.heldModifiers.has(modifier)) {
      state.recording.heldModifiers.set(modifier, { used: false });
    }
    return;
  }

  const token = macroTokenFromEvent(event);
  if (!token) {
    return;
  }

  const modifiers = [...state.recording.heldModifiers.keys()];
  modifiers.forEach((held) => {
    const item = state.recording.heldModifiers.get(held);
    if (item) {
      item.used = true;
    }
  });
  addRecordedToken([...modifiers, token].join("+"));
}

function handleMacroRecordKeyUp(event) {
  if (!state.recording) {
    return;
  }

  event.preventDefault();
  event.stopPropagation();

  const modifier = modifierTokenFromEvent(event);
  if (!modifier || !state.recording.heldModifiers.has(modifier)) {
    return;
  }

  const item = state.recording.heldModifiers.get(modifier);
  state.recording.heldModifiers.delete(modifier);
  if (!item.used) {
    addRecordedToken(modifier);
  }
}

function renderRecordedKeys() {
  const sequence = state.recording?.sequence || [];
  elements.recordingCount.textContent = `${sequence.length} keys`;
  elements.recordedKeys.innerHTML = "";
  if (sequence.length === 0) {
    const empty = document.createElement("span");
    empty.className = "recording-empty";
    empty.textContent = "Ready";
    elements.recordedKeys.append(empty);
    return;
  }

  sequence.forEach((token) => {
    const chip = document.createElement("span");
    chip.className = "recorded-key";
    chip.textContent = token;
    elements.recordedKeys.append(chip);
  });
}

async function stopMacroRecording(save) {
  if (!state.recording) {
    return;
  }

  if (save) {
    state.config.macros[state.selectedMacro].sequence = state.recording.sequence.slice(0, 96);
  } else {
    state.config.macros[state.selectedMacro].sequence = state.recording.previous;
  }
  state.recording = null;
  elements.macroRecorder.hidden = true;

  if (document.fullscreenElement) {
    try {
      await document.exitFullscreen();
    } catch {
      // The browser may have already exited fullscreen.
    }
  }

  persistConfig();
  renderMacros();
  showToast(save ? `M${state.selectedMacro} recorded` : "Recording cancelled");
}

function renderRgbControls() {
  const rgb = state.config.rgb;
  const power = normalizePower(state.config.power);
  state.config.power = power;
  elements.rgbPower.checked = rgb.on;
  elements.rgbLayerMode.checked = rgb.layerMode;
  elements.rgbHue.value = String(rgb.hue);
  elements.rgbSaturation.value = String(rgb.saturation);
  elements.rgbBrightness.value = String(rgb.brightness);
  elements.rgbHueValue.textContent = rgb.hue;
  elements.rgbSaturationValue.textContent = rgb.saturation;
  elements.rgbBrightnessValue.textContent = rgb.brightness;
  elements.rgbEffectName.textContent = RGB_EFFECTS[rgb.effect];
  elements.rgbSwatch.style.background = hsvToCss(rgb.hue, rgb.saturation, rgb.brightness);
  elements.rgbSwatch.classList.toggle("off", !rgb.on);

  elements.rgbTimeout.value = String(power.rgbTimeout);
  elements.rgbTimeoutValue.textContent = `${power.rgbTimeout} min`;

  elements.rgbEffects.querySelectorAll("[data-effect]").forEach((button) => {
    button.classList.toggle("active", Number(button.dataset.effect) === rgb.effect);
  });

  elements.rgbLayerColors.innerHTML = "";
  rgb.layerColors.forEach((color, index) => {
    const label = document.createElement("label");
    label.className = "rgb-layer-row";
    label.innerHTML = `
      <span>${escapeHtml(layerTabLabel(index))}</span>
      <input data-layer="${index}" type="color" value="${hsbToHex(color.hue, color.saturation)}" />
    `;
    elements.rgbLayerColors.append(label);
  });

  elements.rgbLayerColors.querySelectorAll("input[data-layer]").forEach((input) => {
    input.addEventListener("input", () => updateLayerRgb(input));
    input.addEventListener("change", () => sendLayerRgb(Number(input.dataset.layer)));
  });
}

function hsvRgb(hue, saturation, brightness) {
  const s = saturation / 100;
  const v = brightness / 100;
  const chroma = v * s;
  const slot = hue / 60;
  const x = chroma * (1 - Math.abs((slot % 2) - 1));
  const [r, g, b] = slot < 1 ? [chroma, x, 0]
    : slot < 2 ? [x, chroma, 0]
    : slot < 3 ? [0, chroma, x]
    : slot < 4 ? [0, x, chroma]
    : slot < 5 ? [x, 0, chroma]
    : [chroma, 0, x];
  const lift = v - chroma;
  return [r, g, b].map((channel) => Math.round((channel + lift) * 255));
}

function hsvToCss(hue, saturation, brightness) {
  return `rgb(${hsvRgb(hue, saturation, brightness).join(" ")})`;
}

function hsbToHex(hue, saturation, brightness = 100) {
  return `#${hsvRgb(hue, saturation, brightness)
    .map((channel) => channel.toString(16).padStart(2, "0"))
    .join("")}`;
}

function hexToLayerHsb(value) {
  const channels = value.slice(1).match(/.{2}/g)?.map((channel) => Number.parseInt(channel, 16));
  if (!channels || channels.length !== 3) {
    return { hue: 0, saturation: 0 };
  }

  const [r, g, b] = channels.map((channel) => channel / 255);
  const max = Math.max(r, g, b);
  const min = Math.min(r, g, b);
  const delta = max - min;
  const saturation = max === 0 ? 0 : Math.round((delta / max) * 100);

  if (delta === 0) {
    return { hue: 0, saturation };
  }

  const rawHue = max === r ? 60 * (((g - b) / delta) % 6)
    : max === g ? 60 * (((b - r) / delta) + 2)
    : 60 * (((r - g) / delta) + 4);
  return { hue: Math.round((rawHue + 360) % 360), saturation };
}

function updateRgbValue(name, value) {
  state.config.rgb[name] = Number(value);
  persistConfig();
  renderRgbControls();
  renderDisplayControls();
}

async function sendRgbState() {
  try {
    await syncRgb();
    scheduleBoardSave("rgb", 800);
  } catch (error) {
    showToast(error.message);
  }
}

function updatePowerValue(name, value) {
  state.config.power = normalizePower({
    ...(state.config.power || DEFAULT_CONFIG.power),
    [name]: Number(value),
  });
  persistConfig();
  renderRgbControls();
  renderDisplayControls();
}

async function sendPowerState() {
  try {
    await syncPower();
    scheduleBoardSave("power", 800);
  } catch (error) {
    showToast(error.message);
  }
}

function updateLayerRgb(input) {
  state.config.rgb.layerColors[Number(input.dataset.layer)] = hexToLayerHsb(input.value);
  persistConfig();
}

async function sendLayerRgb(layer) {
  try {
    await syncLayerRgb(layer);
    scheduleBoardSave("layer rgb", 800);
  } catch (error) {
    showToast(error.message);
  }
}

async function applyEncoderPreset(index, label) {
  const preset = ENCODER_PRESETS.find((entry) => entry.label === label);
  if (!preset) {
    return;
  }

  const macrosBefore = clone(state.config.macros);
  const macroIndexes = new Set();
  const resolveDirection = (direction) => {
    if (preset[direction]) {
      return preset[direction];
    }

    const shortcut = {
      label: `${preset.label} ${direction === "ccw" ? "CCW" : "CW"}`,
      sequence: preset[`${direction}Sequence`] || [],
    };
    const macroIndex = findMacroSlotForShortcut(shortcut);
    if (macroIndex < 0) {
      return "";
    }
    state.config.macros[macroIndex] = {
      name: shortcut.label.slice(0, 23),
      sequence: shortcut.sequence.map(normalizeMacroStep),
    };
    macroIndexes.add(macroIndex);
    return `M${macroIndex}`;
  };

  const ccw = resolveDirection("ccw");
  const cw = resolveDirection("cw");
  if (!ccw || !cw) {
    state.config.macros = macrosBefore;
    showToast("No free macro slots are available for this encoder preset");
    return;
  }

  state.config.encoders[index] = {
    ccw,
    cw,
    sensitivity: state.config.encoders[index].sensitivity,
  };
  const displayChanges = refreshAutoDisplayLabels(state.config);
  markDisplayChanges(displayChanges);
  persistConfig();
  renderEncoderGrid();
  renderDisplayControls();
  renderTester();

  try {
    for (const macroIndex of macroIndexes) {
      await sendMacro(macroIndex);
    }
    await syncEncoder(index);
    await syncDisplayChanges(displayChanges);
    scheduleBoardSave("encoder preset");
    showToast(`Encoder ${index} -> ${label}`);
  } catch (error) {
    showToast(error.message);
  }
}

function groupedKeycodeOptions(value, allowedKeycodes, includeMacros = false) {
  const allowed = new Set(allowedKeycodes);
  const seen = new Set();
  const groups = [];

  ENCODER_OPTION_GROUPS.forEach(({ label, source }) => {
    const keycodes = (KEYCODE_GROUPS[source] || []).filter(
      (keycode) => allowed.has(keycode) && !seen.has(keycode)
    );
    if (keycodes.length === 0) {
      return;
    }
    keycodes.forEach((keycode) => seen.add(keycode));
    groups.push({ label, keycodes });
  });

  const remaining = [...allowed].filter((keycode) => !seen.has(keycode));
  if (remaining.length > 0) {
    groups.push({ label: "Other", keycodes: remaining });
  }
  if (includeMacros) {
    groups.push({ label: "Macros", keycodes: macroKeycodes() });
  }

  return groups.map(({ label, keycodes }) => `
    <optgroup label="${escapeHtml(label)}">
      ${keycodes.map((keycode) => (
        `<option value="${keycode}" ${keycode === value ? "selected" : ""}>${escapeHtml(keyLabel(keycode))} (${keycode})</option>`
      )).join("")}
    </optgroup>
  `).join("");
}

function encoderSelectMarkup(direction, value) {
  const keycodes = FIRMWARE_KEYCODES.filter((keycode) => keycode !== "NONE");
  return `
    <select class="mapping-select" data-direction="${direction}" aria-label="${direction.toUpperCase()} action">
      ${groupedKeycodeOptions(value, keycodes, true)}
    </select>
  `;
}

function pressSelectMarkup(value, disabled) {
  return `
    <select class="mapping-select" data-role="press" aria-label="Encoder press action" ${disabled ? "disabled" : ""}>
      ${groupedKeycodeOptions(value, FIRMWARE_KEYCODES, true)}
    </select>
  `;
}

function setPanel(panel) {
  state.activePanel = panel;
  $$(".mode-tab").forEach((button) => button.classList.toggle("active", button.dataset.panel === panel));
  $$(".editor-panel").forEach((section) => section.classList.remove("active"));
  $(`#${panel}Panel`).classList.add("active");
  if (panel === "tester") {
    renderTester();
    window.setTimeout(() => elements.testerCapturePad?.focus(), 0);
  }
}

function restoreLocalDefaults() {
  state.config = clone(DEFAULT_CONFIG);
  state.displayDirty.clear();
  state.fullSyncRequired = false;
  state.selectedMacro = 0;
  persistConfig();
  renderAll();
  setSaveButtonState("dirty");
  addLog("MAP ", "Restored local defaults");
  showToast("Local map restored");
}

function profilePayload() {
  saveMacroDraft();
  return {
    format: PROFILE_FORMAT,
    device: DEVICE_NAME,
    exportedAt: new Date().toISOString(),
    config: clone(state.config),
  };
}

function exportProfile() {
  const payload = profilePayload();
  const blob = new Blob([`${JSON.stringify(payload, null, 2)}\n`], { type: "application/json" });
  const url = URL.createObjectURL(blob);
  const anchor = document.createElement("a");
  anchor.href = url;
  anchor.download = `${DEVICE_NAME.toLowerCase()}-braxon-profile.json`;
  document.body.append(anchor);
  anchor.click();
  anchor.remove();
  URL.revokeObjectURL(url);
  addLog("FILE", "Exported local profile");
  showToast("Profile exported");
}

async function importProfile(event) {
  const file = event.target.files?.[0];
  if (!file) {
    return;
  }

  try {
    const data = JSON.parse(await file.text());
    const config = normalizeConfig(data.config || data);
    state.config = config;
    state.fullSyncRequired = true;
    state.selectedMacro = 0;
    persistConfig();
    renderAll();
    setSaveButtonState("dirty");
    addLog("FILE", `Imported ${file.name}`);
    showToast("Profile imported. Press Save to write it to TAC-W.");
  } catch (error) {
    showToast(error.message || "Import failed");
    addLog("ERR ", error.message || "Import failed");
  } finally {
    event.target.value = "";
  }
}

function payloadConfig() {
  saveMacroDraft();
  const config = clone(state.config);
  config.layers = config.layers.map((layer) => layer.map((keycode) => (
    isProfileOnlyKeycode(keycode) ? DEVICE_KEEP : keycode
  )));
  delete config.macros;
  delete config.displayCustom;
  return config;
}

function macroSyncCount() {
  let highest = -1;
  state.config.layers.forEach((layer) => {
    layer.forEach((keycode) => {
      highest = Math.max(highest, macroIndexFromKeycode(keycode));
    });
  });
  state.config.macros.forEach((macro, index) => {
    const hasSequence = Array.isArray(macro.sequence) && macro.sequence.length > 0;
    const renamed = Boolean(macro.name && macro.name !== `M${index}`);
    if (hasSequence || renamed) {
      highest = Math.max(highest, index);
    }
  });
  return Math.min(MAX_MACRO_SLOT_COUNT, Math.max(0, highest + 1));
}

async function syncMacros() {
  saveMacroDraft();
  const count = macroSyncCount();
  for (let index = 0; index < count; index += 1) {
    await sendMacro(index);
  }
}

async function syncEncoder(index) {
  const encoder = state.config.encoders[index];
  await sendCommand("set_encoder", { encoder: index, direction: "ccw", keycode: encoder.ccw });
  await sendCommand("set_encoder", { encoder: index, direction: "cw", keycode: encoder.cw });
  await sendCommand("set_encoder_sensitivity", {
    encoder: index,
    sensitivity: encoder.sensitivity,
  });
}

async function syncRgb(includeLayerColors = false) {
  const reply = await sendCommand("set_rgb", {
    on: state.config.rgb.on,
    reactive: false,
    layerMode: state.config.rgb.layerMode,
    effect: state.config.rgb.effect,
    hue: state.config.rgb.hue,
    saturation: state.config.rgb.saturation,
    brightness: state.config.rgb.brightness,
  });

  if (includeLayerColors) {
    for (let layer = 0; layer < state.config.rgb.layerColors.length; layer += 1) {
      await syncLayerRgb(layer);
    }
  }

  return reply;
}

async function syncPower() {
  const power = normalizePower(state.config.power);
  state.config.power = power;
  return sendCommand("set_power", {
    rgbTimeout: power.rgbTimeout,
    displayTimeout: power.displayTimeout,
    displayOn: power.displayOn,
  });
}

async function syncLayerRgb(layer) {
  const color = state.config.rgb.layerColors[layer];
  return sendCommand("set_layer_rgb", { layer, hue: color.hue, saturation: color.saturation });
}

async function syncDisplayLabels() {
  for (let layer = 0; layer < DEVICE_LAYER_INDEX; layer += 1) {
    await sendDisplayLabel("layer", layer);
    for (let encoder = 0; encoder < state.config.display.encoders[layer].length; encoder += 1) {
      await sendDisplayLabel("encoder", layer, encoder);
    }
    for (let key = 0; key < state.config.display.keys[layer].length; key += 1) {
      await sendDisplayLabel("key", layer, key);
    }
  }
  state.displayDirty.clear();
}

async function syncProfileToBoard() {
  await syncMacros();
  let applied = 0;
  for (let layer = 0; layer < DEVICE_LAYER_INDEX; layer += 1) {
    for (let position = 0; position < state.config.layers[layer].length; position += 1) {
      if (position === LAYER_KEY_POSITION) {
        continue;
      }
      const keycode = state.config.layers[layer][position];
      if (isProfileOnlyKeycode(keycode)) {
        continue;
      }
      await sendCommand("set_key", { layer, position, keycode });
      applied += 1;
      if (state.transport?.kind === "ble-local") {
        await pause(35);
      }
    }
  }
  const reply = { ok: true, cmd: "set_key", applied };
  const skipped = [];

  for (let index = 0; index < state.config.encoders.length; index += 1) {
    try {
      await syncEncoder(index);
    } catch (error) {
      skipped.push(`encoder ${index}`);
      logWire("NOTE", `Encoder ${index} sync skipped: ${error.message}`);
    }
  }

  try {
    await syncRgb(true);
  } catch (error) {
    skipped.push("rgb");
    logWire("NOTE", `RGB sync skipped: ${error.message}`);
  }

  try {
    await syncPower();
  } catch (error) {
    skipped.push("power");
    logWire("NOTE", `Power timeout sync skipped: ${error.message}`);
  }

  try {
    await syncDisplayLabels();
  } catch (error) {
    skipped.push("display");
    logWire("NOTE", `Display label sync skipped: ${error.message}`);
  }

  return { ...reply, skipped };
}

async function saveToBoard() {
  window.clearTimeout(autoSaveTimer);
  try {
    setSaveButtonState("saving");
    if (state.fullSyncRequired) {
      await syncProfileToBoard();
      state.fullSyncRequired = false;
    }
    const reply = await sendCommand("save_config");
    if (reply.ok) {
      setSaveButtonState("saved");
      showToast("Saved to keyboard");
    }
  } catch (error) {
    setSaveButtonState("dirty");
    showToast(error.message);
  }
}

async function resetBoardMap() {
  try {
    const reply = await sendCommand("reset_config");
    if (reply.ok) {
      restoreLocalDefaults();
      showToast("Board map reset");
    }
  } catch (error) {
    showToast(error.message);
  }
}

async function hardResetDevice() {
  const confirmed = window.confirm(
    `Factory reset ${DEVICE_NAME}? This clears Bluetooth pairings, keymap, encoders, macros, RGB, power, and display labels.`
  );
  if (!confirmed) {
    return;
  }

  try {
    const reply = await sendCommand("hard_reset");
    if (reply.ok) {
      restoreLocalDefaults();
      state.knownBleDevices = [];
      showToast("Factory reset complete");
      addLog("RST ", "Factory reset sent to keyboard");
    }
  } catch (error) {
    showToast(error.message);
  }
}

function logWire(prefix, message) {
  addLog(prefix, message);
}

function encodeCommand(cmd, payload = {}) {
  const id = ++state.messageId;
  return { id, raw: `${JSON.stringify({ id, cmd, ...payload })}\n` };
}

async function sendCommand(cmd, payload = {}, waitForReply = true) {
  const { id, raw } = encodeCommand(cmd, payload);

  if (!state.transport) {
    logWire("SIM ", raw.trim());
    return { id, ok: true, simulated: true, cmd };
  }

  if (!state.authorized && !["ping", "get_config"].includes(cmd)) {
    throw new Error(`Authorize ${DEVICE_NAME} first`);
  }

  logWire("TX  ", raw.trim());

  if (!waitForReply) {
    await state.transport.write(raw);
    return { id, ok: true };
  }

  const replyPromise = new Promise((resolve, reject) => {
    const timeoutMs = ["save_config", "reset_config", "hard_reset"].includes(cmd)
      ? 20000
      : state.transport?.kind === "ble-local"
        ? 12000
        : 8000;
    const timeout = window.setTimeout(() => {
      state.pending.delete(id);
      reject(new Error(`${cmd} timed out`));
    }, timeoutMs);

    state.pending.set(id, {
      resolve: (reply) => {
        window.clearTimeout(timeout);
        if (reply.ok) {
          resolve(reply);
          return;
        }
        reject(new Error(reply.error || `${cmd} failed`));
      },
    });
  });

  try {
    await state.transport.write(raw);
  } catch (error) {
    const pending = state.pending.get(id);
    state.pending.delete(id);
    if (pending) {
      pending.resolve({ ok: false, error: error.message || `${cmd} write failed` });
    }
  }

  return replyPromise;
}

function feedTransportText(chunk) {
  if (!state.transport) {
    return;
  }

  state.transport.rxText += chunk.replace(/\r/g, "");
  const parts = state.transport.rxText.split("\n");
  state.transport.rxText = parts.pop() || "";
  parts.filter(Boolean).forEach(handleTransportLine);
}

function handleTransportLine(line) {
  logWire("RX  ", line);

  let reply;
  try {
    reply = JSON.parse(line);
  } catch {
    return;
  }

  if (reply.event) {
    handleDeviceEvent(reply);
    return;
  }

  if (typeof reply.id !== "number") {
    return;
  }

  const pending = state.pending.get(reply.id);
  if (pending) {
    state.pending.delete(reply.id);
    pending.resolve(reply);
  }
}

function handleDeviceEvent(reply) {
  if (reply.event === "matrix") {
    const layer = Number.isInteger(reply.layer) ? reply.layer : state.activeLayer;
    recordTesterMatrixEvent(reply.position, Boolean(reply.pressed), layer);
    return;
  }

  if (reply.event === "encoder") {
    const direction = reply.direction === "CCW" ? "CCW" : "CW";
    const layer = Number.isInteger(reply.layer) ? reply.layer : state.activeLayer;
    recordTesterEncoderEvent(reply.encoder, direction, reply.value, layer);
  }
}

async function connectSerial() {
  if (!("serial" in navigator)) {
    showToast("Web Serial is not available in this browser");
    return;
  }

  try {
    const port = await navigator.serial.requestPort({ filters: USB_FILTERS });
    await disconnectTransport();
    await port.open({ baudRate: SERIAL_BAUD });
    const reader = port.readable.getReader();
    const writer = port.writable.getWriter();
    const decoder = new TextDecoder();
    const encoder = new TextEncoder();

    state.transport = {
      kind: "serial",
      label: "USB",
      reader,
      writer,
      rxText: "",
      write: (text) => writer.write(encoder.encode(text)),
      close: async () => {
        try {
          await reader.cancel();
        } catch {
          // Reader may already be gone on unplug.
        }
        try {
          reader.releaseLock();
        } catch {
          // Lock may already be released.
        }
        try {
          writer.releaseLock();
        } catch {
          // Lock may already be released.
        }
        try {
          await port.close();
        } catch {
          // Port may already be closed.
        }
      },
    };

    setBadge("Checking USB device", "preview");
    setButtonsForTransport();
    logWire("USB ", `Opened serial at ${SERIAL_BAUD}`);
    readSerialLoop(state.transport, decoder);
    await helloDevice();
  } catch (error) {
    setBadge("Not connected", "idle");
    setButtonsForTransport();
    if (error.name !== "NotFoundError") {
      showToast(error.message || "USB connect failed");
      logWire("ERR ", error.message || "USB connect failed");
    }
  }
}

async function readSerialLoop(activeTransport, decoder) {
  try {
    while (state.transport === activeTransport) {
      const { value, done } = await activeTransport.reader.read();
      if (done) {
        break;
      }
      if (value) {
        feedTransportText(decoder.decode(value, { stream: true }));
      }
    }
  } catch (error) {
    if (state.transport === activeTransport) {
      logWire("ERR ", error.message || "USB read stopped");
    }
  } finally {
    if (state.transport === activeTransport) {
      state.transport = null;
      state.authorized = false;
      setBadge("USB disconnected", "error");
      setButtonsForTransport();
    }
  }
}

async function connectBle() {
  if (IS_WINDOWS) {
    try {
      await openWindowsBleTransport();
    } catch (error) {
      setBadge("Not connected", "idle");
      setButtonsForTransport();
      showToast(error.message || "Windows BLE bridge could not connect");
      logWire("ERR ", error.message || "Windows BLE bridge could not connect");
    }
    return;
  }

  if (!("bluetooth" in navigator)) {
    showToast("BLE connection is unavailable");
    return;
  }

  try {
    const savedDevice = state.knownBleDevices.find(looksLikeTacWDevice);
    try {
      const device = savedDevice || await requestBleDevice();
      await openBleTransport(device);
    } catch (error) {
      if (!savedDevice || error.name === "NotFoundError") {
        throw error;
      }

      logWire("BLE ", "Saved BLE permission failed; click Connect BLE again to reopen the chooser");
      state.knownBleDevices = state.knownBleDevices.filter((device) => device.id !== savedDevice.id);
      throw error;
    }
  } catch (error) {
    setBadge("Not connected", "idle");
    setButtonsForTransport();
    if (error.name === "NotFoundError") {
      const message = `No BLE ${DEVICE_NAME} was selected. Keep the keyboard awake, then choose ${DEVICE_NAME} from the Bluetooth picker.`;
      showToast(message);
      logWire("BLE ", message);
      return;
    }

    showToast(error.message || "BLE connect failed");
    logWire("ERR ", error.message || "BLE connect failed");
  }
}

async function openWindowsBleTransport() {
  showToast(`Connecting ${DEVICE_NAME} through Windows Bluetooth`);
  logWire("BLE ", "Opening local Windows remap channel");
  await disconnectTransport();

  const connectResponse = await fetch("/api/ble/connect", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: "{}",
  });
  const connectResult = await connectResponse.json();
  if (!connectResponse.ok || !connectResult.ok) {
    throw new Error(connectResult.error || "Windows BLE bridge could not connect");
  }

  state.transport = {
    kind: "ble-local",
    label: connectResult.name || "BLE",
    rxText: "",
    write: async (text) => {
      const response = await fetch("/api/ble/write", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ text }),
      });
      const result = await response.json();
      if (!response.ok || !result.ok) {
        throw new Error(result.error || "BLE write failed");
      }
      if (result.text) {
        feedTransportText(result.text);
      }
    },
    close: async () => {
      try {
        await fetch("/api/ble/disconnect", {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: "{}",
        });
      } catch {
        // The local server may already be stopped.
      }
    },
  };

  setBadge(`Checking ${state.transport.label}`, "preview");
  setButtonsForTransport();
  logWire("BLE ", `Connected to ${state.transport.label} through Windows`);
  await helloDevice();
}

function looksLikeTacWDevice(device) {
  return /^(TAC-W|wire-x)/i.test(device?.name || "");
}

async function requestBleDevice() {
  showToast(`Choose ${DEVICE_NAME} in the Bluetooth picker`);
  logWire("BLE ", "Opening Bluetooth chooser");
  const device = await navigator.bluetooth.requestDevice({
    filters: [{ services: [NUS_SERVICE_UUID] }],
    optionalServices: [NUS_SERVICE_UUID],
  });
  if (device.name && !looksLikeTacWDevice(device)) {
    logWire("BLE ", `Selected ${device.name}; checking remap service`);
  }
  rememberBleDevice(device);
  return device;
}

function rememberBleDevice(device) {
  if (!device || state.knownBleDevices.some((known) => known.id === device.id)) {
    return;
  }

  state.knownBleDevices.push(device);
}

async function refreshKnownBleDevices() {
  if (!("bluetooth" in navigator) || typeof navigator.bluetooth.getDevices !== "function") {
    return;
  }

  try {
    const devices = await navigator.bluetooth.getDevices();
    devices.filter(looksLikeTacWDevice).forEach(rememberBleDevice);
    if (state.knownBleDevices.length > 0) {
      logWire("BLE ", `Saved browser permission found for ${state.knownBleDevices[0].name || DEVICE_NAME}`);
    }
  } catch (error) {
    logWire("NOTE", error.message || "Saved BLE permission check failed");
  }
}

async function openBleTransport(device) {
  if (!device) {
    throw new Error(`No BLE ${DEVICE_NAME} device is available`);
  }

  showToast(`Connecting ${device.name || DEVICE_NAME} over BLE`);
  logWire("BLE ", `Opening ${device.name || DEVICE_NAME} remap channel`);
  try {
    await disconnectTransport();
    const server = await device.gatt.connect();
    const service = await server.getPrimaryService(NUS_SERVICE_UUID);
    const rx = await service.getCharacteristic(NUS_RX_UUID);
    const tx = await service.getCharacteristic(NUS_TX_UUID);
    const encoder = new TextEncoder();
    const decoder = new TextDecoder();

    const notify = (event) => {
      const value = event.target.value;
      feedTransportText(decoder.decode(value.buffer.slice(value.byteOffset, value.byteOffset + value.byteLength)));
    };

    await tx.startNotifications();
    tx.addEventListener("characteristicvaluechanged", notify);

    state.transport = {
      kind: "ble",
      label: device.name || "BLE",
      rxText: "",
      write: async (text) => {
        const bytes = encoder.encode(text);
        for (let offset = 0; offset < bytes.length; offset += BLE_CHUNK_SIZE) {
          const chunk = bytes.slice(offset, offset + BLE_CHUNK_SIZE);
          if (rx.writeValueWithoutResponse) {
            await rx.writeValueWithoutResponse(chunk);
          } else {
            await rx.writeValue(chunk);
          }
          await pause(10);
        }
      },
      close: async () => {
        try {
          tx.removeEventListener("characteristicvaluechanged", notify);
          await tx.stopNotifications();
        } catch {
          // Notification channel may already be gone.
        }
        try {
          server.disconnect();
        } catch {
          // GATT may already be disconnected.
        }
      },
    };

    device.addEventListener("gattserverdisconnected", () => {
      if (state.transport?.kind === "ble") {
        state.transport = null;
        state.authorized = false;
        setBadge("BLE disconnected", "error");
        setButtonsForTransport();
        logWire("BLE ", "Disconnected");
      }
    });

    setBadge(`Checking ${state.transport.label}`, "preview");
    setButtonsForTransport();
    logWire("BLE ", `Connected to ${state.transport.label}`);
    await helloDevice();
  } catch (error) {
    throw error;
  }
}

async function helloDevice() {
  try {
    const reply = await sendCommand("ping");
    if (!reply.ok || reply.name !== DEVICE_NAME) {
      throw new Error(`This is not a supported ${DEVICE_NAME} device`);
    }

    state.authorized = true;
    setBadge(`${state.transport.label} authorized`, "live");
    renderAccessState();

    const configReply = await sendCommand("get_config");
    if (configReply.ok && configReply.config) {
      const localConfig = state.config;
      const boardConfig = normalizeConfig(configReply.config);
      boardConfig.layers = boardConfig.layers.map((layer, layerIndex) => layer.map((keycode, position) => (
        isProfileOnlyKeycode(localConfig.layers?.[layerIndex]?.[position])
          ? localConfig.layers[layerIndex][position]
          : keycode
      )));
      state.config = boardConfig;
      state.fullSyncRequired = false;
      persistConfig();
      renderAll();
      setSaveButtonState("clean");
      logWire("MAP ", "Loaded saved map from keyboard");
    }

    showToast(`${reply.name} authorized and synced`);
  } catch (error) {
    showToast(error.message);
    await disconnectTransport();
  }
}

async function disconnectTransport() {
  const transport = state.transport;
  window.clearTimeout(autoSaveTimer);
  autoSaveQueued = false;
  state.transport = null;
  state.authorized = false;
  state.pending.forEach((pending) => pending.resolve({ ok: false, error: "disconnected" }));
  state.pending.clear();

  if (transport) {
    await transport.close();
    logWire("OFF ", `${transport.label} disconnected`);
  }

  setBadge("Not connected", "idle");
  setButtonsForTransport();
}

function pause(milliseconds) {
  return new Promise((resolve) => window.setTimeout(resolve, milliseconds));
}

function renderAll() {
  renderLayerTabs();
  renderKeyboard();
  renderSelection();
  renderCategoryTabs();
  renderKeyLibrary();
  renderEncoderGrid();
  renderMacros();
  renderRgbControls();
  renderDisplayControls();
  renderTester();
}

function bindEvents() {
  elements.usbConnectButton.addEventListener("click", connectSerial);
  elements.authorizeUsbButton.addEventListener("click", connectSerial);
  elements.bleConnectButton.addEventListener("click", connectBle);
  elements.authorizeBleButton.addEventListener("click", connectBle);
  elements.disconnectButton.addEventListener("click", disconnectTransport);
  elements.toolbarImportButton.addEventListener("click", () => elements.profileImportFile.click());
  elements.toolbarExportButton.addEventListener("click", exportProfile);
  elements.profileImportFile.addEventListener("change", importProfile);
  elements.toolbarSaveButton.addEventListener("click", saveToBoard);
  elements.toolbarResetButton.addEventListener("click", resetBoardMap);
  elements.toolbarHardResetButton.addEventListener("click", hardResetDevice);
  elements.keySearch.addEventListener("input", renderKeyLibrary);
  elements.libraryExpandButton.addEventListener("click", () => setLibraryExpanded(true));
  elements.libraryCloseButton.addEventListener("click", () => setLibraryExpanded(false));
  elements.libraryBackdrop.addEventListener("click", () => setLibraryExpanded(false));
  document.addEventListener("keydown", (event) => {
    if (event.key === "Escape" && state.libraryExpanded && !state.recording) {
      setLibraryExpanded(false);
    }
  });
  elements.clearLogButton.addEventListener("click", () => {
    state.logLines = [];
    elements.protocolLog.textContent = "";
  });
  elements.macroName.addEventListener("input", updateMacroDraftUi);
  elements.macroSequence.addEventListener("input", saveMacroDraft);
  elements.macroAddButton.addEventListener("click", addMacroSlot);
  elements.macroRecordButton.addEventListener("click", startMacroRecording);
  elements.macroAddDelayButton.addEventListener("click", addMacroDelay);
  elements.macroClearButton.addEventListener("click", clearCurrentMacro);
  elements.macroDeleteButton.addEventListener("click", deleteCurrentMacro);
  elements.macroSaveButton.addEventListener("click", saveMacroProfile);
  elements.launcherType.addEventListener("change", renderLauncherPresets);
  elements.launcherPreset.addEventListener("change", () => {
    elements.launcherTarget.value = elements.launcherPreset.value;
  });
  elements.launcherBuildButton.addEventListener("click", buildLauncherMacro);
  elements.displayLayerName.addEventListener("input", () => {
    elements.displayLayerName.value = sanitizeDisplayInput(elements.displayLayerName.value);
    queueDisplayLabelUpdate("layer", Number(elements.displayLayerName.dataset.layer || state.activeLayer), 0, elements.displayLayerName.value);
  });
  elements.displayLayerName.addEventListener("change", () => (
    commitDisplayLabelUpdate("layer", Number(elements.displayLayerName.dataset.layer || state.activeLayer), 0, elements.displayLayerName.value)
  ));
  elements.recordStopButton.addEventListener("click", () => stopMacroRecording(true));
  elements.recordCancelButton.addEventListener("click", () => stopMacroRecording(false));
  document.addEventListener("keydown", handleMacroRecordKey, true);
  document.addEventListener("keyup", handleMacroRecordKeyUp, true);
  document.addEventListener("fullscreenchange", () => {
    if (!document.fullscreenElement && state.recording && !elements.macroRecorder.hidden) {
      stopMacroRecording(true);
    }
  });
  elements.testerFocusButton.addEventListener("click", () => elements.testerCapturePad.focus());
  elements.testerClearButton.addEventListener("click", () => {
    state.testerEvents = [];
    state.testerHeldKeys.clear();
    state.testerActivePositions.clear();
    state.testerActiveEncoders.clear();
    elements.testerLastInput.textContent = "Waiting";
    renderTester();
  });
  $$("[data-display-test]").forEach((button) => {
    button.addEventListener("click", () => sendDisplayTest(button.dataset.displayTest));
  });
  $$("[data-rgb-test]").forEach((button) => {
    button.addEventListener("click", () => sendRgbTest(button.dataset.rgbTest));
  });

  elements.rgbPower.addEventListener("change", () => {
    state.config.rgb.on = elements.rgbPower.checked;
    persistConfig();
    renderRgbControls();
    sendRgbState();
  });
  elements.rgbLayerMode.addEventListener("change", () => {
    state.config.rgb.layerMode = elements.rgbLayerMode.checked;
    if (state.config.rgb.layerMode) {
      state.config.rgb.effect = 0;
    }
    persistConfig();
    renderRgbControls();
    sendRgbState();
  });
  [
    [elements.rgbHue, "hue"],
    [elements.rgbSaturation, "saturation"],
    [elements.rgbBrightness, "brightness"],
  ].forEach(([input, name]) => {
    input.addEventListener("input", () => updateRgbValue(name, input.value));
    input.addEventListener("change", sendRgbState);
  });
  if (elements.displayPower) {
    elements.displayPower.addEventListener("change", () => {
      state.config.power = normalizePower({
        ...(state.config.power || DEFAULT_CONFIG.power),
        displayOn: elements.displayPower.checked,
      });
      persistConfig();
      renderDisplayControls();
      sendPowerState();
    });
  }
  [[elements.rgbTimeout, "rgbTimeout"], [elements.displayTimeout, "displayTimeout"]].forEach(([input, name]) => {
    if (!input) {
      return;
    }
    input.addEventListener("input", () => updatePowerValue(name, input.value));
    input.addEventListener("change", sendPowerState);
  });
  elements.rgbEffects.querySelectorAll("[data-effect]").forEach((button) => {
    button.addEventListener("click", () => {
      state.config.rgb.effect = Number(button.dataset.effect);
      persistConfig();
      renderRgbControls();
      sendRgbState();
    });
  });

  $$(".mode-tab").forEach((button) => {
    button.addEventListener("click", () => setPanel(button.dataset.panel));
  });

  window.addEventListener("beforeunload", () => {
    if (state.transport) {
      state.transport.close();
    }
  });
}

renderLauncherPresets();
bindEvents();
renderAll();
setButtonsForTransport();
addLog("BOOT", "Braxon Studio ready");
refreshKnownBleDevices();
if (!("serial" in navigator)) {
  addLog("NOTE", "Web Serial is unavailable in this browser");
}
if (!("bluetooth" in navigator)) {
  addLog("NOTE", "Web Bluetooth is unavailable in this browser");
}
