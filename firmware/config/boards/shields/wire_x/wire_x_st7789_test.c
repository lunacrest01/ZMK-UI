#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <lvgl.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/util.h>

#include <zmk/display.h>
#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/keymap.h>
#include <zmk/usb.h>
#include <dt-bindings/zmk/hid_indicators.h>

#if IS_ENABLED(CONFIG_ZMK_HID_INDICATORS)
#include <zmk/events/hid_indicators_changed.h>
#include <zmk/hid_indicators.h>
#endif

#if IS_ENABLED(CONFIG_ZMK_BATTERY)
#include <zmk/battery.h>
#endif

#if IS_ENABLED(CONFIG_ZMK_BLE)
#include <zmk/ble.h>
#endif

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define TFT_WIDTH 240
#define TFT_HEIGHT 135
#define WIRE_X_LAYERS 6
#define WIRE_X_DEVICE_LAYER_INDEX 5
#define WIRE_X_KEYS 12
#define WIRE_X_ENCODERS 2
#define GRID_ROWS 3
#define GRID_COLS 4
#define HEADER_H 44
#define CELL_W 60
#define CELL_H 30
#define GRID_POSITION_OFFSET 2
#define SPLASH_DURATION_MS 2500
#define KEY_RELEASE_LINGER_MS 90
#define LAYER_ACCENT_MS 210
#define ENCODER_PULSE_MS 180
#define MAX_LABEL_CHARS 7
#define DISPLAY_LABEL_LEN (MAX_LABEL_CHARS + 1)
#define DISPLAY_LABEL_SETTINGS_KEY "wire_x/display_labels"

#define WIRE_X_DISPLAY_LABEL_LAYER 0
#define WIRE_X_DISPLAY_LABEL_KEY 1
#define WIRE_X_DISPLAY_LABEL_ENCODER 2

#define COLOR_BG 0x090a0f
#define COLOR_HEADER 0x0f0c19
#define COLOR_GRID 0x2a1f4f
#define COLOR_GRID_DIM 0x171525
#define COLOR_ACCENT 0x7b3fe4
#define COLOR_ACCENT_2 0x3ecfcf
#define COLOR_TEXT 0xd9c9ff
#define COLOR_TEXT_DIM 0x9da0b8
#define COLOR_KEY_TEXT 0xbfc2d6
#define COLOR_KEY_PRESS 0xffffff
#define COLOR_ENC 0xffba35
#define COLOR_PANEL 0x12111d
#define COLOR_STATUS_CARD 0x111820
#define COLOR_STATUS_GREEN 0x36c88a
#define COLOR_STATUS_WARN 0xffba35
#define COLOR_STATUS_BLUE 0x2d9cff
#define COLOR_STATUS_RED 0xff4d5d

#include "wire_x_logo_240x135.inc"
#include "wire_x_dance_cat_rle_240x135.inc"

static const lv_image_dsc_t splash_logo_image = {
	.header = {
		.magic = LV_IMAGE_HEADER_MAGIC,
		.cf = LV_COLOR_FORMAT_RGB565,
		.flags = 0,
		.w = TFT_WIDTH,
		.h = TFT_HEIGHT,
		.stride = TFT_WIDTH * 2,
	},
	.data_size = sizeof(wire_x_logo_240x135),
	.data = wire_x_logo_240x135,
	.reserved = NULL,
};

static uint8_t idle_dance_buf[TFT_WIDTH * TFT_HEIGHT * 2];
static lv_image_dsc_t idle_dance_image = {
	.header = {
		.magic = LV_IMAGE_HEADER_MAGIC,
		.cf = LV_COLOR_FORMAT_RGB565,
		.flags = 0,
		.w = TFT_WIDTH,
		.h = TFT_HEIGHT,
		.stride = TFT_WIDTH * 2,
	},
	.data_size = sizeof(idle_dance_buf),
	.data = idle_dance_buf,
	.reserved = NULL,
};
static uint8_t idle_dance_loaded_frame = UINT8_MAX;

static lv_obj_t *screen_obj;
static lv_obj_t *main_obj;
static lv_obj_t *splash_obj;
static lv_obj_t *status_obj;
static lv_obj_t *idle_obj;
static lv_obj_t *idle_cat_img;
static lv_obj_t *layer_title;
static lv_obj_t *accent_bar;
static lv_obj_t *layer_marks[WIRE_X_LAYERS];
static lv_obj_t *encoder_zone[WIRE_X_ENCODERS];
static lv_obj_t *encoder_value[WIRE_X_ENCODERS];
static lv_obj_t *key_cells[WIRE_X_KEYS];
static lv_obj_t *key_labels[WIRE_X_KEYS];
static lv_obj_t *display_test_obj;
static lv_obj_t *display_test_label;
static lv_obj_t *status_uptime_value;
static lv_obj_t *status_layer_value;
static lv_obj_t *status_ble_badges[3];
static lv_obj_t *status_ble_badge_labels[3];
static lv_obj_t *status_ble_badge_numbers[3];
static lv_obj_t *status_usb_badge;
static lv_obj_t *status_usb_badge_label;
static lv_obj_t *status_route_ble_badge;
static lv_obj_t *status_route_ble_badge_label;
static lv_obj_t *status_battery_icon;
static lv_obj_t *status_battery_value;
static lv_obj_t *status_lock_badges[3];
static lv_obj_t *status_lock_labels[3];
static lv_obj_t *status_host_card;
static lv_obj_t *status_host_title;
static lv_obj_t *status_host_value;
static uint8_t status_charge_frame;
static const struct gpio_dt_spec backlight_gpio =
	GPIO_DT_SPEC_GET_OR(DT_ALIAS(wire_x_backlight), gpios, {0});
static const struct device *const display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static bool backlight_ready;
static bool backlight_on = true;

static lv_style_t style_screen;
static lv_style_t style_header_zone;
static lv_style_t style_key_base;
static lv_style_t style_key_pressed;
static lv_style_transition_dsc_t key_press_tr;
static lv_style_transition_dsc_t key_release_tr;
static bool styles_ready;
static bool release_work_ready;
static uint8_t current_layer;
static bool status_active;
static bool idle_active;
static uint8_t idle_frame;

struct wire_x_display_labels {
	char layer[WIRE_X_LAYERS][DISPLAY_LABEL_LEN];
	char encoder[WIRE_X_LAYERS][WIRE_X_ENCODERS][DISPLAY_LABEL_LEN];
	char key[WIRE_X_LAYERS][WIRE_X_KEYS][DISPLAY_LABEL_LEN];
};

static const lv_style_prop_t key_press_props[] = {
	LV_STYLE_BG_OPA,
	LV_STYLE_SHADOW_OPA,
	LV_STYLE_BORDER_COLOR,
	0,
};

static const struct wire_x_display_labels display_label_defaults = {
	.layer = {"LAYER 1", "LAYER 2", "LAYER 3", "LAYER 4", "LAYER 5", "DEVICE"},
	.encoder = {
		{"PGUP", "VOL"},
		{"SCRL", "ZOOM"},
		{"UNDO", "REDO"},
		{"BRT", "TRK"},
		{"NAV", "APP"},
		{"USB", "BLE"},
	},
	.key = {
		{"A", "B", "C", "LAYER", "E", "F", "G", "H", "I", "J", "K", "L"},
		{"1", "2", "3", "LAYER", "5", "6", "7", "8", "9", "0", "-", "="},
		{"!", "@", "#", "LAYER", "%", "^", "&", "*", "(", ")", "UP", "DN"},
		{"PLAY", "MUTE", "VDN", "LAYER", "NEXT", "PREV", "HOME", "END", "DEL", "PGUP",
		 "PGDN", "TAB"},
		{"ESC", "SPACE", "F1", "LAYER", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
		 "F10"},
		{"BLE1", "BLE2", "BLE3", "LAYER", "USB", "BLE", "RESET", "CLEAR", "RGB",
		 "FX", "HUE", "BRT"},
	},
};

static struct wire_x_display_labels display_labels = {
	.layer = {"LAYER 1", "LAYER 2", "LAYER 3", "LAYER 4", "LAYER 5", "DEVICE"},
	.encoder = {
		{"PGUP", "VOL"},
		{"SCRL", "ZOOM"},
		{"UNDO", "REDO"},
		{"BRT", "TRK"},
		{"NAV", "APP"},
		{"USB", "BLE"},
	},
	.key = {
		{"A", "B", "C", "LAYER", "E", "F", "G", "H", "I", "J", "K", "L"},
		{"1", "2", "3", "LAYER", "5", "6", "7", "8", "9", "0", "-", "="},
		{"!", "@", "#", "LAYER", "%", "^", "&", "*", "(", ")", "UP", "DN"},
		{"PLAY", "MUTE", "VDN", "LAYER", "NEXT", "PREV", "HOME", "END", "DEL", "PGUP",
		 "PGDN", "TAB"},
		{"ESC", "SPACE", "F1", "LAYER", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
		 "F10"},
		{"BLE1", "BLE2", "BLE3", "LAYER", "USB", "BLE", "RESET", "CLEAR", "RGB",
		 "FX", "HUE", "BRT"},
	},
};
static bool display_labels_dirty;

enum wire_x_press_target {
	PRESS_TARGET_KEY = 0,
	PRESS_TARGET_ENCODER = 1,
};

struct wire_x_key_event {
	uint8_t target;
	uint8_t index;
	bool pressed;
};

struct wire_x_display_state {
	uint8_t layer;
};

K_MSGQ_DEFINE(key_event_msgq, sizeof(struct wire_x_key_event), 8, 4);
static struct k_work key_event_work;
static struct k_work display_label_update_work;
static struct k_work display_test_work;
static struct k_work status_toggle_work;
static struct k_work_delayable status_refresh_work;
static struct k_work_delayable idle_anim_work;
static struct k_work_delayable key_release_work[WIRE_X_KEYS];
static struct k_work_delayable encoder_release_work[WIRE_X_ENCODERS];
static uint8_t display_test_mode;

static void update_display_for_layer(uint8_t layer, bool animate);
static void update_status_ui(void);

static int init_backlight(void)
{
	if (!backlight_gpio.port) {
		return -ENODEV;
	}

	if (!device_is_ready(backlight_gpio.port)) {
		return -ENODEV;
	}

	if (!backlight_ready) {
		int ret = gpio_pin_configure_dt(&backlight_gpio, GPIO_OUTPUT_ACTIVE);
		if (ret < 0) {
			return ret;
		}
		backlight_ready = true;
	}

	return 0;
}

void wire_x_display_backlight_set(bool on)
{
	if (init_backlight() < 0) {
		return;
	}

	if (backlight_on == on) {
		return;
	}

	if (!on && device_is_ready(display_dev)) {
		(void)display_blanking_on(display_dev);
	}

	backlight_on = on;
	(void)gpio_pin_set_dt(&backlight_gpio, on ? 1 : 0);

	if (on) {
		k_sleep(K_MSEC(35));
		if (device_is_ready(display_dev)) {
			(void)display_blanking_off(display_dev);
		}
		if (screen_obj != NULL) {
			lv_obj_invalidate(screen_obj);
		}
	}
}

static void init_styles(void)
{
	if (styles_ready) {
		return;
	}

	lv_style_transition_dsc_init(&key_press_tr, key_press_props, lv_anim_path_ease_out, 55, 0,
				     NULL);
	lv_style_transition_dsc_init(&key_release_tr, key_press_props, lv_anim_path_ease_in_out, 145,
				     0, NULL);

	lv_style_init(&style_screen);
	lv_style_set_bg_color(&style_screen, lv_color_hex(COLOR_BG));
	lv_style_set_bg_opa(&style_screen, LV_OPA_COVER);
	lv_style_set_pad_all(&style_screen, 0);
	lv_style_set_border_width(&style_screen, 0);
	lv_style_set_radius(&style_screen, 0);

	lv_style_init(&style_header_zone);
	lv_style_set_bg_color(&style_header_zone, lv_color_hex(COLOR_PANEL));
	lv_style_set_bg_opa(&style_header_zone, LV_OPA_50);
	lv_style_set_border_color(&style_header_zone, lv_color_hex(COLOR_ACCENT));
	lv_style_set_border_width(&style_header_zone, 1);
	lv_style_set_radius(&style_header_zone, 2);
	lv_style_set_pad_all(&style_header_zone, 0);
	lv_style_set_transition(&style_header_zone, &key_release_tr);

	lv_style_init(&style_key_base);
	lv_style_set_bg_color(&style_key_base, lv_color_hex(COLOR_ACCENT));
	lv_style_set_bg_opa(&style_key_base, LV_OPA_TRANSP);
	lv_style_set_border_color(&style_key_base, lv_color_hex(COLOR_GRID));
	lv_style_set_border_width(&style_key_base, 1);
	lv_style_set_radius(&style_key_base, 2);
	lv_style_set_pad_all(&style_key_base, 0);
	lv_style_set_shadow_color(&style_key_base, lv_color_hex(COLOR_ACCENT));
	lv_style_set_shadow_opa(&style_key_base, LV_OPA_TRANSP);
	lv_style_set_shadow_width(&style_key_base, 7);
	lv_style_set_shadow_spread(&style_key_base, 1);
	lv_style_set_transition(&style_key_base, &key_release_tr);

	lv_style_init(&style_key_pressed);
	lv_style_set_bg_color(&style_key_pressed, lv_color_hex(COLOR_ACCENT));
	lv_style_set_bg_opa(&style_key_pressed, LV_OPA_70);
	lv_style_set_border_color(&style_key_pressed, lv_color_hex(COLOR_ACCENT_2));
	lv_style_set_shadow_color(&style_key_pressed, lv_color_hex(COLOR_ACCENT));
	lv_style_set_shadow_opa(&style_key_pressed, LV_OPA_50);
	lv_style_set_shadow_width(&style_key_pressed, 10);
	lv_style_set_shadow_spread(&style_key_pressed, 2);
	lv_style_set_transition(&style_key_pressed, &key_press_tr);

	styles_ready = true;
}

static void prep_obj(lv_obj_t *obj)
{
	lv_obj_remove_style_all(obj);
	lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static lv_obj_t *make_rect(lv_obj_t *parent, int x, int y, int w, int h, uint32_t color,
			   lv_opa_t opa, uint8_t radius)
{
	lv_obj_t *obj = lv_obj_create(parent);
	prep_obj(obj);
	lv_obj_set_pos(obj, x, y);
	lv_obj_set_size(obj, w, h);
	lv_obj_set_style_bg_color(obj, lv_color_hex(color), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(obj, opa, LV_PART_MAIN);
	lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
	lv_obj_set_style_radius(obj, radius, LV_PART_MAIN);
	lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
	return obj;
}

static lv_obj_t *make_label(lv_obj_t *parent, const char *text, const lv_font_t *font,
			    uint32_t color, int x, int y, int w, lv_text_align_t align)
{
	lv_obj_t *label = lv_label_create(parent);
	lv_label_set_text(label, text);
	lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
	lv_obj_set_width(label, w);
	lv_obj_set_pos(label, x, y);
	lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
	lv_obj_set_style_text_color(label, lv_color_hex(color), LV_PART_MAIN);
	lv_obj_set_style_text_align(label, align, LV_PART_MAIN);
	lv_obj_set_style_text_letter_space(label, 0, LV_PART_MAIN);
	return label;
}

static void set_compact_label(lv_obj_t *label, const char *text)
{
	char clipped[MAX_LABEL_CHARS + 1] = {0};

	if (text == NULL) {
		lv_label_set_text(label, "");
		return;
	}

	strncpy(clipped, text, MAX_LABEL_CHARS);
	lv_label_set_text(label, clipped);
}

static void copy_display_label(char *dst, const char *src, const char *fallback)
{
	const char *source = (src != NULL && src[0] != '\0') ? src : fallback;

	memset(dst, 0, DISPLAY_LABEL_LEN);
	if (source == NULL) {
		return;
	}

	for (size_t i = 0; source[i] != '\0' && i + 1 < DISPLAY_LABEL_LEN; i++) {
		unsigned char c = (unsigned char)source[i];
		dst[i] = (c >= 0x20 && c < 0x7f) ? (char)c : ' ';
	}
}

static void sanitize_display_labels(void)
{
	for (uint8_t layer = 0; layer < WIRE_X_LAYERS; layer++) {
		display_labels.layer[layer][DISPLAY_LABEL_LEN - 1] = '\0';
		if (display_labels.layer[layer][0] == '\0') {
			copy_display_label(display_labels.layer[layer], display_label_defaults.layer[layer],
					   display_label_defaults.layer[layer]);
		}

		for (uint8_t encoder = 0; encoder < WIRE_X_ENCODERS; encoder++) {
			display_labels.encoder[layer][encoder][DISPLAY_LABEL_LEN - 1] = '\0';
			if (display_labels.encoder[layer][encoder][0] == '\0') {
				copy_display_label(display_labels.encoder[layer][encoder],
						   display_label_defaults.encoder[layer][encoder],
						   display_label_defaults.encoder[layer][encoder]);
			}
		}

		for (uint8_t key = 0; key < WIRE_X_KEYS; key++) {
			display_labels.key[layer][key][DISPLAY_LABEL_LEN - 1] = '\0';
			if (display_labels.key[layer][key][0] == '\0') {
				copy_display_label(display_labels.key[layer][key],
						   display_label_defaults.key[layer][key],
						   display_label_defaults.key[layer][key]);
			}
		}
	}

	memcpy(display_labels.layer[WIRE_X_DEVICE_LAYER_INDEX],
	       display_label_defaults.layer[WIRE_X_DEVICE_LAYER_INDEX],
	       sizeof(display_labels.layer[WIRE_X_DEVICE_LAYER_INDEX]));
	memcpy(display_labels.encoder[WIRE_X_DEVICE_LAYER_INDEX],
	       display_label_defaults.encoder[WIRE_X_DEVICE_LAYER_INDEX],
	       sizeof(display_labels.encoder[WIRE_X_DEVICE_LAYER_INDEX]));
	memcpy(display_labels.key[WIRE_X_DEVICE_LAYER_INDEX],
	       display_label_defaults.key[WIRE_X_DEVICE_LAYER_INDEX],
	       sizeof(display_labels.key[WIRE_X_DEVICE_LAYER_INDEX]));
}

static void request_display_label_update(void)
{
	if (screen_obj != NULL && zmk_display_is_initialized()) {
		k_work_submit_to_queue(zmk_display_work_q(), &display_label_update_work);
	}
}

const char *wire_x_display_layer_label(uint8_t layer)
{
	return layer < WIRE_X_LAYERS ? display_labels.layer[layer] : "";
}

const char *wire_x_display_encoder_label(uint8_t layer, uint8_t encoder)
{
	return layer < WIRE_X_LAYERS && encoder < WIRE_X_ENCODERS
		       ? display_labels.encoder[layer][encoder]
		       : "";
}

const char *wire_x_display_key_label(uint8_t layer, uint8_t key)
{
	return layer < WIRE_X_LAYERS && key < WIRE_X_KEYS ? display_labels.key[layer][key] : "";
}

int wire_x_display_set_label(uint8_t kind, uint8_t layer, uint8_t index, const char *text)
{
	if (layer >= WIRE_X_LAYERS) {
		return -EINVAL;
	}

	if (layer == WIRE_X_DEVICE_LAYER_INDEX) {
		return -EACCES;
	}

	switch (kind) {
	case WIRE_X_DISPLAY_LABEL_LAYER:
		copy_display_label(display_labels.layer[layer], text, display_label_defaults.layer[layer]);
		break;
	case WIRE_X_DISPLAY_LABEL_KEY:
		if (index >= WIRE_X_KEYS) {
			return -EINVAL;
		}
		copy_display_label(display_labels.key[layer][index], text,
				   display_label_defaults.key[layer][index]);
		break;
	case WIRE_X_DISPLAY_LABEL_ENCODER:
		if (index >= WIRE_X_ENCODERS) {
			return -EINVAL;
		}
		copy_display_label(display_labels.encoder[layer][index], text,
				   display_label_defaults.encoder[layer][index]);
		break;
	default:
		return -EINVAL;
	}

	display_labels_dirty = true;
	if (layer == current_layer) {
		request_display_label_update();
	}
	return 0;
}

bool wire_x_display_labels_dirty(void)
{
	return display_labels_dirty;
}

int wire_x_display_save_labels(void)
{
	int ret = settings_save_one(DISPLAY_LABEL_SETTINGS_KEY, &display_labels, sizeof(display_labels));
	if (ret >= 0) {
		display_labels_dirty = false;
	}

	return ret;
}

int wire_x_display_reset_labels(void)
{
	int ret = settings_delete(DISPLAY_LABEL_SETTINGS_KEY);
	if (ret < 0 && ret != -ENOENT) {
		return ret;
	}

	display_labels = display_label_defaults;
	display_labels_dirty = false;
	request_display_label_update();
	return 0;
}

static uint32_t display_test_color(uint8_t mode)
{
	switch (mode) {
	case 1:
		return 0xff0000;
	case 2:
		return 0x00d060;
	case 3:
		return 0x0066ff;
	default:
		return COLOR_BG;
	}
}

static const char *display_test_name(uint8_t mode)
{
	switch (mode) {
	case 1:
		return "RED";
	case 2:
		return "GREEN";
	case 3:
		return "BLUE";
	default:
		return "TAC-W";
	}
}

static void display_test_work_cb(struct k_work *work)
{
	ARG_UNUSED(work);

	if (screen_obj == NULL || !zmk_display_is_initialized()) {
		return;
	}

	if (display_test_mode == 0) {
		if (display_test_obj != NULL) {
			lv_obj_add_flag(display_test_obj, LV_OBJ_FLAG_HIDDEN);
		}
		if (status_active && status_obj != NULL) {
			if (main_obj != NULL) {
				lv_obj_add_flag(main_obj, LV_OBJ_FLAG_HIDDEN);
			}
			lv_obj_clear_flag(status_obj, LV_OBJ_FLAG_HIDDEN);
			update_status_ui();
		} else if (idle_active && idle_obj != NULL) {
			if (main_obj != NULL) {
				lv_obj_add_flag(main_obj, LV_OBJ_FLAG_HIDDEN);
			}
			lv_obj_clear_flag(idle_obj, LV_OBJ_FLAG_HIDDEN);
		} else if (main_obj != NULL) {
			lv_obj_clear_flag(main_obj, LV_OBJ_FLAG_HIDDEN);
			update_display_for_layer(current_layer, false);
		}
		return;
	}

	if (display_test_obj == NULL) {
		display_test_obj = lv_obj_create(screen_obj);
		prep_obj(display_test_obj);
		lv_obj_set_size(display_test_obj, TFT_WIDTH, TFT_HEIGHT);
		lv_obj_set_style_border_width(display_test_obj, 0, LV_PART_MAIN);
		lv_obj_set_style_radius(display_test_obj, 0, LV_PART_MAIN);
		lv_obj_set_style_pad_all(display_test_obj, 0, LV_PART_MAIN);

		display_test_label = make_label(display_test_obj, "", &lv_font_montserrat_18, 0xffffff, 0,
						0, TFT_WIDTH, LV_TEXT_ALIGN_CENTER);
		lv_obj_center(display_test_label);
	}

	if (splash_obj != NULL) {
		lv_obj_add_flag(splash_obj, LV_OBJ_FLAG_HIDDEN);
	}
	if (main_obj != NULL) {
		lv_obj_add_flag(main_obj, LV_OBJ_FLAG_HIDDEN);
	}
	if (status_obj != NULL) {
		lv_obj_add_flag(status_obj, LV_OBJ_FLAG_HIDDEN);
	}
	if (idle_obj != NULL) {
		lv_obj_add_flag(idle_obj, LV_OBJ_FLAG_HIDDEN);
	}

	lv_obj_set_style_bg_color(display_test_obj, lv_color_hex(display_test_color(display_test_mode)),
				  LV_PART_MAIN);
	lv_obj_set_style_bg_opa(display_test_obj, LV_OPA_COVER, LV_PART_MAIN);
	lv_label_set_text(display_test_label, display_test_name(display_test_mode));
	lv_obj_set_style_text_color(display_test_label, lv_color_hex(0xffffff), LV_PART_MAIN);
	lv_obj_clear_flag(display_test_obj, LV_OBJ_FLAG_HIDDEN);
	lv_obj_move_foreground(display_test_obj);
}

int wire_x_display_test(uint8_t mode)
{
	if (mode > 3) {
		return -EINVAL;
	}

	display_test_mode = mode;
	if (screen_obj != NULL && zmk_display_is_initialized()) {
		k_work_submit_to_queue(zmk_display_work_q(), &display_test_work);
	}
	return 0;
}

static lv_obj_t *make_status_value(lv_obj_t *parent, int x, int y, int w, int h,
				   const char *title)
{
	lv_obj_t *card = lv_obj_create(parent);
	prep_obj(card);
	lv_obj_set_pos(card, x, y);
	lv_obj_set_size(card, w, h);
	lv_obj_set_style_bg_color(card, lv_color_hex(COLOR_STATUS_CARD), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(card, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_border_color(card, lv_color_hex(COLOR_GRID), LV_PART_MAIN);
	lv_obj_set_style_border_width(card, 1, LV_PART_MAIN);
	lv_obj_set_style_radius(card, 3, LV_PART_MAIN);
	lv_obj_set_style_pad_all(card, 0, LV_PART_MAIN);

	int title_y = 3;
	int value_y = h <= 28 ? 10 : (h < 34 ? 12 : 16);
	const lv_font_t *value_font = h <= 28 ? &lv_font_montserrat_12 : &lv_font_montserrat_14;

	make_label(card, title, &lv_font_montserrat_8, COLOR_TEXT_DIM, 4, title_y, w - 8,
		   LV_TEXT_ALIGN_CENTER);
	return make_label(card, "--", value_font, COLOR_TEXT, 4, value_y, w - 8,
			  LV_TEXT_ALIGN_CENTER);
}

static lv_obj_t *make_status_icon_badge(lv_obj_t *parent, int x, int y, int w, int h,
					const char *symbol, lv_obj_t **label_ref)
{
	lv_obj_t *badge = lv_obj_create(parent);
	prep_obj(badge);
	lv_obj_set_pos(badge, x, y);
	lv_obj_set_size(badge, w, h);
	lv_obj_set_style_bg_color(badge, lv_color_hex(COLOR_STATUS_CARD), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_border_color(badge, lv_color_hex(COLOR_GRID), LV_PART_MAIN);
	lv_obj_set_style_border_width(badge, 1, LV_PART_MAIN);
	lv_obj_set_style_radius(badge, 4, LV_PART_MAIN);
	lv_obj_set_style_pad_all(badge, 0, LV_PART_MAIN);
	lv_obj_set_style_shadow_width(badge, 8, LV_PART_MAIN);
	lv_obj_set_style_shadow_opa(badge, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_shadow_color(badge, lv_color_hex(COLOR_STATUS_BLUE), LV_PART_MAIN);

	*label_ref = make_label(badge, symbol, &lv_font_montserrat_20, COLOR_TEXT_DIM, 0,
				(h - 20) / 2, w, LV_TEXT_ALIGN_CENTER);
	return badge;
}

static lv_obj_t *make_status_profile_badge(lv_obj_t *parent, int x, int y, uint8_t profile,
					   lv_obj_t **label_ref, lv_obj_t **number_ref)
{
	lv_obj_t *badge = lv_obj_create(parent);
	prep_obj(badge);
	lv_obj_set_pos(badge, x, y);
	lv_obj_set_size(badge, 34, 38);
	lv_obj_set_style_bg_color(badge, lv_color_hex(COLOR_STATUS_CARD), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_border_color(badge, lv_color_hex(COLOR_GRID), LV_PART_MAIN);
	lv_obj_set_style_border_width(badge, 1, LV_PART_MAIN);
	lv_obj_set_style_radius(badge, 4, LV_PART_MAIN);
	lv_obj_set_style_pad_all(badge, 0, LV_PART_MAIN);
	lv_obj_set_style_shadow_width(badge, 8, LV_PART_MAIN);
	lv_obj_set_style_shadow_opa(badge, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_shadow_color(badge, lv_color_hex(COLOR_STATUS_BLUE), LV_PART_MAIN);

	*label_ref = make_label(badge, LV_SYMBOL_BLUETOOTH, &lv_font_montserrat_20,
				COLOR_TEXT_DIM, 0, 3, 34, LV_TEXT_ALIGN_CENTER);

	char number[2];
	snprintk(number, sizeof(number), "%u", profile + 1);
	*number_ref = make_label(badge, number, &lv_font_montserrat_10, COLOR_TEXT_DIM, 0, 25,
				 34, LV_TEXT_ALIGN_CENTER);

	return badge;
}

static void set_status_badge_state(lv_obj_t *badge, lv_obj_t *label, bool active, bool linked,
				   bool open, uint32_t active_color)
{
	if (badge == NULL || label == NULL) {
		return;
	}

	ARG_UNUSED(open);

	uint32_t border = active ? active_color : (linked ? COLOR_STATUS_GREEN : COLOR_STATUS_RED);
	uint32_t bg = active ? 0x102238 : COLOR_STATUS_CARD;
	uint32_t text = active ? 0xffffff : (linked ? COLOR_STATUS_GREEN : COLOR_STATUS_RED);

	lv_obj_set_style_bg_color(badge, lv_color_hex(bg), LV_PART_MAIN);
	lv_obj_set_style_border_color(badge, lv_color_hex(border), LV_PART_MAIN);
	lv_obj_set_style_shadow_color(badge, lv_color_hex(active_color), LV_PART_MAIN);
	lv_obj_set_style_shadow_opa(badge, active ? LV_OPA_50 : LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_text_color(label, lv_color_hex(text), LV_PART_MAIN);
}

static void set_status_label_state(lv_obj_t *label, bool active, bool linked)
{
	if (label == NULL) {
		return;
	}

	lv_obj_set_style_text_color(label,
				    lv_color_hex(active ? 0xffffff :
						 (linked ? COLOR_STATUS_GREEN : COLOR_STATUS_RED)),
				    LV_PART_MAIN);
}

static lv_obj_t *make_status_lock_badge(lv_obj_t *parent, int x, int y, const char *text,
					lv_obj_t **label_ref)
{
	lv_obj_t *badge = make_rect(parent, x, y, 50, 23, COLOR_STATUS_CARD, LV_OPA_COVER, 4);
	lv_obj_set_style_border_color(badge, lv_color_hex(COLOR_GRID), LV_PART_MAIN);
	lv_obj_set_style_border_width(badge, 1, LV_PART_MAIN);
	*label_ref = make_label(badge, text, &lv_font_montserrat_12, COLOR_TEXT_DIM, 0, 4, 50,
				LV_TEXT_ALIGN_CENTER);
	return badge;
}

static void set_status_lock_state(lv_obj_t *badge, lv_obj_t *label, bool enabled)
{
	if (badge == NULL || label == NULL) {
		return;
	}

	uint32_t color = enabled ? COLOR_STATUS_GREEN : COLOR_GRID;
	lv_obj_set_style_border_color(badge, lv_color_hex(color), LV_PART_MAIN);
	lv_obj_set_style_bg_color(badge, lv_color_hex(enabled ? 0x10251d : COLOR_STATUS_CARD),
				  LV_PART_MAIN);
	lv_obj_set_style_text_color(label,
				    lv_color_hex(enabled ? COLOR_STATUS_GREEN : COLOR_TEXT_DIM),
				    LV_PART_MAIN);
}

static uint32_t charging_color_for_frame(uint8_t frame)
{
	return (frame & 0x01) ? 0x77eef4 : COLOR_STATUS_BLUE;
}

static uint32_t battery_color_for_percent(uint8_t percent)
{
	if (percent <= 15) {
		return COLOR_STATUS_RED;
	}

	return COLOR_STATUS_GREEN;
}

static const char *battery_symbol_for_percent(uint8_t percent)
{
	if (percent >= 92) {
		return LV_SYMBOL_BATTERY_FULL;
	}
	if (percent >= 70) {
		return LV_SYMBOL_BATTERY_3;
	}
	if (percent >= 40) {
		return LV_SYMBOL_BATTERY_2;
	}
	if (percent >= 15) {
		return LV_SYMBOL_BATTERY_1;
	}
	return LV_SYMBOL_BATTERY_EMPTY;
}

static const char *battery_charge_symbol(uint8_t frame)
{
	static const char *const frames[] = {
		LV_SYMBOL_BATTERY_EMPTY,
		LV_SYMBOL_BATTERY_1,
		LV_SYMBOL_BATTERY_2,
		LV_SYMBOL_BATTERY_3,
		LV_SYMBOL_BATTERY_FULL,
	};

	return frames[frame % ARRAY_SIZE(frames)];
}

static void make_status_battery_widget(lv_obj_t *parent, int x, int y, int w, int h)
{
	lv_obj_t *card = make_rect(parent, x, y, w, h, COLOR_STATUS_CARD, LV_OPA_COVER, 4);
	lv_obj_set_style_border_color(card, lv_color_hex(COLOR_GRID), LV_PART_MAIN);
	lv_obj_set_style_border_width(card, 1, LV_PART_MAIN);

	status_battery_icon = make_label(card, LV_SYMBOL_BATTERY_EMPTY, &lv_font_montserrat_20,
					 COLOR_TEXT_DIM, 0, 3, w, LV_TEXT_ALIGN_CENTER);
	status_battery_value = make_label(card, "--", &lv_font_montserrat_10, COLOR_TEXT, 0, 25,
					  w, LV_TEXT_ALIGN_CENTER);
}

static void make_status_host_card(lv_obj_t *parent, int x, int y)
{
	status_host_card = make_rect(parent, x, y, 53, 23, COLOR_STATUS_CARD, LV_OPA_COVER, 4);
	lv_obj_set_style_border_color(status_host_card, lv_color_hex(COLOR_GRID), LV_PART_MAIN);
	lv_obj_set_style_border_width(status_host_card, 1, LV_PART_MAIN);

	status_host_title = make_label(status_host_card, "HOST", &lv_font_montserrat_8,
				       COLOR_TEXT_DIM, 0, 1, 53, LV_TEXT_ALIGN_CENTER);
	status_host_value = make_label(status_host_card, "--", &lv_font_montserrat_12,
				       COLOR_TEXT, 0, 8, 53, LV_TEXT_ALIGN_CENTER);
}

static void set_status_host_card(const char *text, uint32_t color, bool active)
{
	if (status_host_card == NULL || status_host_title == NULL || status_host_value == NULL) {
		return;
	}

	lv_label_set_text(status_host_value, text);
	lv_obj_set_style_border_color(status_host_card, lv_color_hex(color), LV_PART_MAIN);
	lv_obj_set_style_bg_color(status_host_card,
				  lv_color_hex(active ? 0x102238 : COLOR_STATUS_CARD),
				  LV_PART_MAIN);
	lv_obj_set_style_text_color(status_host_title, lv_color_hex(color), LV_PART_MAIN);
	lv_obj_set_style_text_color(status_host_value, lv_color_hex(active ? 0xffffff : color),
				    LV_PART_MAIN);
}

static void set_status_battery_widget(uint8_t percent, bool charging, uint8_t frame)
{
	if (status_battery_icon == NULL || status_battery_value == NULL) {
		return;
	}

	uint32_t color = charging ? charging_color_for_frame(frame) : battery_color_for_percent(percent);
	char text[8];

	lv_label_set_text(status_battery_icon,
			  charging ? battery_charge_symbol(frame) : battery_symbol_for_percent(percent));
	snprintk(text, sizeof(text), "%u%%", percent);
	lv_label_set_text(status_battery_value, text);
	lv_obj_set_style_text_color(status_battery_icon, lv_color_hex(color), LV_PART_MAIN);
	lv_obj_set_style_text_color(status_battery_value, lv_color_hex(color), LV_PART_MAIN);
}

static void format_uptime(char *out, size_t len)
{
	int64_t seconds = k_uptime_get() / 1000;
	int64_t hours = seconds / 3600;
	int64_t minutes = (seconds / 60) % 60;
	int64_t secs = seconds % 60;

	if (hours > 99) {
		snprintk(out, len, "%lldH", hours);
	} else {
		snprintk(out, len, "%02lld:%02lld:%02lld", hours, minutes, secs);
	}
}

static void update_status_ui(void)
{
	if (status_obj == NULL || !status_active) {
		return;
	}

	char text[24];
	struct zmk_endpoint_instance endpoint = zmk_endpoint_get_selected();
	enum zmk_transport preferred = zmk_endpoint_get_preferred_transport();
	bool usb_powered = zmk_usb_is_powered();
	bool selected_usb = endpoint.transport == ZMK_TRANSPORT_USB;
	bool selected_ble = endpoint.transport == ZMK_TRANSPORT_BLE;
	bool route_usb = selected_usb || (preferred == ZMK_TRANSPORT_USB && usb_powered);
	bool route_ble = selected_ble || preferred == ZMK_TRANSPORT_BLE;
	bool any_ble_linked = false;
	int active_ble_profile = -1;

#if IS_ENABLED(CONFIG_ZMK_BLE)
	int profile = zmk_ble_active_profile_index();
	active_ble_profile = profile;
	for (uint8_t i = 0; i < 3; i++) {
		bool linked = zmk_ble_profile_is_connected(i);
		bool active = route_ble && profile == i;
		any_ble_linked = any_ble_linked || linked;
		set_status_badge_state(status_ble_badges[i], status_ble_badge_labels[i],
				       active, linked, false, COLOR_STATUS_BLUE);
		set_status_label_state(status_ble_badge_numbers[i], active, linked);
	}
#else
	for (uint8_t i = 0; i < 3; i++) {
		set_status_badge_state(status_ble_badges[i], status_ble_badge_labels[i], false, false,
				       false, COLOR_STATUS_GREEN);
		set_status_label_state(status_ble_badge_numbers[i], false, false);
	}
#endif
	set_status_badge_state(status_usb_badge, status_usb_badge_label, route_usb,
			       usb_powered, false, COLOR_STATUS_BLUE);
	set_status_badge_state(status_route_ble_badge, status_route_ble_badge_label, route_ble,
			       any_ble_linked, false, COLOR_STATUS_BLUE);

	char host_text[8];
	uint32_t host_color = COLOR_STATUS_WARN;
	bool host_active = false;
	if (route_usb) {
		snprintk(host_text, sizeof(host_text), "USB");
		host_color = COLOR_STATUS_BLUE;
		host_active = usb_powered;
	} else if (route_ble && active_ble_profile >= 0) {
		snprintk(host_text, sizeof(host_text), "B%d", active_ble_profile + 1);
		host_color = any_ble_linked ? COLOR_STATUS_GREEN : COLOR_STATUS_RED;
		host_active = any_ble_linked;
	} else {
		snprintk(host_text, sizeof(host_text), "--");
	}
	set_status_host_card(host_text, host_color, host_active);

	uint8_t charge_frame = status_charge_frame;
	if (usb_powered) {
		status_charge_frame++;
	}

#if IS_ENABLED(CONFIG_ZMK_BATTERY)
	uint8_t battery_percent = zmk_battery_state_of_charge();
	set_status_battery_widget(battery_percent, usb_powered, charge_frame);
#else
	set_status_battery_widget(0, false, charge_frame);
#endif

	format_uptime(text, sizeof(text));
	lv_label_set_text(status_uptime_value, text);
	snprintk(text, sizeof(text), "L%d", current_layer + 1);
	lv_label_set_text(status_layer_value, text);

	uint8_t indicators = 0;
#if IS_ENABLED(CONFIG_ZMK_HID_INDICATORS)
	indicators = zmk_hid_indicators_get_current_profile();
#endif
	set_status_lock_state(status_lock_badges[0], status_lock_labels[0],
			      (indicators & HID_INDICATOR_CAPS_LOCK) != 0);
	set_status_lock_state(status_lock_badges[1], status_lock_labels[1],
			      (indicators & HID_INDICATOR_NUM_LOCK) != 0);
	set_status_lock_state(status_lock_badges[2], status_lock_labels[2],
			      (indicators & HID_INDICATOR_SCROLL_LOCK) != 0);
}

static void status_refresh_work_cb(struct k_work *work)
{
	ARG_UNUSED(work);

	update_status_ui();
	if (status_active) {
		k_work_schedule_for_queue(zmk_display_work_q(), &status_refresh_work, K_SECONDS(1));
	}
}

static void load_idle_dance_frame(uint8_t frame)
{
	frame %= WIRE_X_DANCE_CAT_FRAME_COUNT;
	if (idle_dance_loaded_frame == frame) {
		return;
	}

	const uint16_t *rle = wire_x_dance_cat_rle_frames[frame];
	uint16_t pair_count = wire_x_dance_cat_rle_pair_counts[frame];
	uint8_t *dst = idle_dance_buf;
	size_t written = 0;

	for (uint16_t i = 0; i < pair_count && written < WIRE_X_DANCE_CAT_PIXELS; i++) {
		uint16_t count = rle[i * 2];
		uint16_t color = rle[(i * 2) + 1];
		uint8_t lo = color & 0xff;
		uint8_t hi = color >> 8;

		while (count-- > 0 && written < WIRE_X_DANCE_CAT_PIXELS) {
			*dst++ = lo;
			*dst++ = hi;
			written++;
		}
	}

	idle_dance_loaded_frame = frame;
}

static void idle_anim_work_cb(struct k_work *work)
{
	ARG_UNUSED(work);

	if (!idle_active || idle_obj == NULL || idle_cat_img == NULL) {
		return;
	}

	uint8_t frame = idle_frame++;
	load_idle_dance_frame(frame);
	lv_obj_invalidate(idle_cat_img);

	k_work_schedule_for_queue(zmk_display_work_q(), &idle_anim_work, K_MSEC(120));
}

static void show_idle_ui(bool show)
{
	idle_active = show;

	if (show) {
		status_active = false;
		k_work_cancel_delayable(&status_refresh_work);
		if (splash_obj != NULL) {
			lv_obj_add_flag(splash_obj, LV_OBJ_FLAG_HIDDEN);
		}
		if (main_obj != NULL) {
			lv_obj_add_flag(main_obj, LV_OBJ_FLAG_HIDDEN);
		}
		if (status_obj != NULL) {
			lv_obj_add_flag(status_obj, LV_OBJ_FLAG_HIDDEN);
		}
		if (idle_obj != NULL) {
			lv_obj_clear_flag(idle_obj, LV_OBJ_FLAG_HIDDEN);
			lv_obj_move_foreground(idle_obj);
		}
		k_work_schedule_for_queue(zmk_display_work_q(), &idle_anim_work, K_NO_WAIT);
		return;
	}

	k_work_cancel_delayable(&idle_anim_work);
	if (idle_obj != NULL) {
		lv_obj_add_flag(idle_obj, LV_OBJ_FLAG_HIDDEN);
	}
}

static void show_status_ui(bool show)
{
	status_active = show;
	if (show) {
		show_idle_ui(false);
	}

	if (display_test_obj != NULL) {
		lv_obj_add_flag(display_test_obj, LV_OBJ_FLAG_HIDDEN);
	}
	if (splash_obj != NULL) {
		lv_obj_add_flag(splash_obj, LV_OBJ_FLAG_HIDDEN);
	}

	if (show) {
		if (main_obj != NULL) {
			lv_obj_add_flag(main_obj, LV_OBJ_FLAG_HIDDEN);
		}
		if (status_obj != NULL) {
			lv_obj_clear_flag(status_obj, LV_OBJ_FLAG_HIDDEN);
			lv_obj_move_foreground(status_obj);
		}
		update_status_ui();
		k_work_schedule_for_queue(zmk_display_work_q(), &status_refresh_work, K_SECONDS(1));
		return;
	}

	k_work_cancel_delayable(&status_refresh_work);
	if (status_obj != NULL) {
		lv_obj_add_flag(status_obj, LV_OBJ_FLAG_HIDDEN);
	}
	if (main_obj != NULL) {
		lv_obj_clear_flag(main_obj, LV_OBJ_FLAG_HIDDEN);
		update_display_for_layer(current_layer, true);
	}
}

static void status_toggle_work_cb(struct k_work *work)
{
	ARG_UNUSED(work);

	if (screen_obj == NULL || !zmk_display_is_initialized()) {
		return;
	}

	if (!status_active && !idle_active) {
		show_status_ui(true);
	} else if (status_active) {
		show_status_ui(false);
		show_idle_ui(true);
	} else {
		show_idle_ui(false);
		show_status_ui(false);
	}
}

void wire_x_display_toggle_status(void)
{
	if (screen_obj != NULL && zmk_display_is_initialized()) {
		k_work_submit_to_queue(zmk_display_work_q(), &status_toggle_work);
	}
}

int wire_x_display_settings_set(const char *name, size_t len, settings_read_cb read_cb,
				void *cb_arg)
{
	const char *next;
	if (!settings_name_steq(name, "display_labels", &next) || next) {
		return -ENOENT;
	}

	if (len != sizeof(display_labels)) {
		return -EINVAL;
	}

	int ret = read_cb(cb_arg, &display_labels, sizeof(display_labels));
	if (ret <= 0) {
		return ret;
	}

	sanitize_display_labels();
	display_labels_dirty = false;
	request_display_label_update();
	return 0;
}

static void make_hline(lv_obj_t *parent, int y, uint32_t color, lv_opa_t opa)
{
	make_rect(parent, 0, y, TFT_WIDTH, 1, color, opa, 0);
}

static void make_vline(lv_obj_t *parent, int x, int y, int h, uint32_t color, lv_opa_t opa)
{
	make_rect(parent, x, y, 1, h, color, opa, 0);
}

static void accent_width_anim_cb(void *obj, int32_t value)
{
	lv_obj_set_width((lv_obj_t *)obj, (lv_coord_t)value);
}

static void encoder_color_anim_cb(void *obj, int32_t value)
{
	lv_color_t color = lv_color_mix(lv_color_hex(COLOR_ENC), lv_color_white(), value);
	lv_obj_set_style_text_color((lv_obj_t *)obj, color, LV_PART_MAIN);
}

static void pulse_encoder_label(uint8_t index)
{
	if (index >= WIRE_X_ENCODERS || encoder_value[index] == NULL) {
		return;
	}

	lv_anim_t anim;
	lv_anim_init(&anim);
	lv_anim_set_var(&anim, encoder_value[index]);
	lv_anim_set_exec_cb(&anim, encoder_color_anim_cb);
	lv_anim_set_values(&anim, 0, 255);
	lv_anim_set_time(&anim, ENCODER_PULSE_MS);
	lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
	lv_anim_start(&anim);
}

static void animate_layer_change(void)
{
	if (accent_bar != NULL) {
		lv_obj_set_width(accent_bar, 4);

		lv_anim_t anim;
		lv_anim_init(&anim);
		lv_anim_set_var(&anim, accent_bar);
		lv_anim_set_exec_cb(&anim, accent_width_anim_cb);
		lv_anim_set_values(&anim, 4, 47);
		lv_anim_set_time(&anim, LAYER_ACCENT_MS);
		lv_anim_set_path_cb(&anim, lv_anim_path_overshoot);
		lv_anim_start(&anim);
	}

	for (uint8_t i = 0; i < WIRE_X_ENCODERS; i++) {
		pulse_encoder_label(i);
	}
}

static void set_key_visual(uint8_t index, bool pressed)
{
	if (index >= WIRE_X_KEYS || key_cells[index] == NULL || key_labels[index] == NULL) {
		return;
	}

	if (pressed) {
		lv_obj_add_state(key_cells[index], LV_STATE_PRESSED);
		lv_obj_set_style_text_color(key_labels[index], lv_color_hex(COLOR_KEY_PRESS),
					    LV_PART_MAIN);
	} else {
		lv_obj_clear_state(key_cells[index], LV_STATE_PRESSED);
		lv_obj_set_style_text_color(key_labels[index], lv_color_hex(COLOR_KEY_TEXT),
					    LV_PART_MAIN);
	}
}

static void set_encoder_visual(uint8_t index, bool pressed)
{
	if (index >= WIRE_X_ENCODERS || encoder_zone[index] == NULL || encoder_value[index] == NULL) {
		return;
	}

	if (pressed) {
		lv_obj_add_state(encoder_zone[index], LV_STATE_PRESSED);
		lv_obj_set_style_text_color(encoder_value[index], lv_color_white(), LV_PART_MAIN);
	} else {
		lv_obj_clear_state(encoder_zone[index], LV_STATE_PRESSED);
		lv_obj_set_style_text_color(encoder_value[index], lv_color_hex(COLOR_ENC), LV_PART_MAIN);
	}
}

static void key_release_work_cb(struct k_work *work)
{
	struct k_work_delayable *dwork = k_work_delayable_from_work(work);

	for (uint8_t i = 0; i < WIRE_X_KEYS; i++) {
		if (dwork == &key_release_work[i]) {
			set_key_visual(i, false);
			return;
		}
	}

	for (uint8_t i = 0; i < WIRE_X_ENCODERS; i++) {
		if (dwork == &encoder_release_work[i]) {
			set_encoder_visual(i, false);
			return;
		}
	}
}

static void init_release_work(void)
{
	if (release_work_ready) {
		return;
	}

	for (uint8_t i = 0; i < WIRE_X_KEYS; i++) {
		k_work_init_delayable(&key_release_work[i], key_release_work_cb);
	}
	for (uint8_t i = 0; i < WIRE_X_ENCODERS; i++) {
		k_work_init_delayable(&encoder_release_work[i], key_release_work_cb);
	}

	release_work_ready = true;
}

static void process_key_event(struct wire_x_key_event event)
{
	if (event.target == PRESS_TARGET_KEY) {
		k_work_cancel_delayable(&key_release_work[event.index]);
		set_key_visual(event.index, true);
		k_work_schedule_for_queue(zmk_display_work_q(), &key_release_work[event.index],
					  K_MSEC(KEY_RELEASE_LINGER_MS));
	} else if (event.target == PRESS_TARGET_ENCODER) {
		k_work_cancel_delayable(&encoder_release_work[event.index]);
		set_encoder_visual(event.index, true);
		k_work_schedule_for_queue(zmk_display_work_q(), &encoder_release_work[event.index],
					  K_MSEC(KEY_RELEASE_LINGER_MS));
	}
}

static void key_event_work_cb(struct k_work *work)
{
	ARG_UNUSED(work);

	struct wire_x_key_event event;
	while (k_msgq_get(&key_event_msgq, &event, K_NO_WAIT) == 0) {
		process_key_event(event);
	}
}

static int key_position_listener_cb(const zmk_event_t *eh)
{
	const struct zmk_position_state_changed *event = as_zmk_position_state_changed(eh);

	if (event == NULL || !event->state || status_active || idle_active || !zmk_display_is_initialized() ||
	    screen_obj == NULL) {
		return ZMK_EV_EVENT_BUBBLE;
	}

	struct wire_x_key_event key_event = {0};

	if (event->position < WIRE_X_ENCODERS) {
		key_event.target = PRESS_TARGET_ENCODER;
		key_event.index = (uint8_t)event->position;
	} else if (event->position >= GRID_POSITION_OFFSET &&
		   event->position < GRID_POSITION_OFFSET + WIRE_X_KEYS) {
		key_event.target = PRESS_TARGET_KEY;
		key_event.index = (uint8_t)(event->position - GRID_POSITION_OFFSET);
	} else {
		return ZMK_EV_EVENT_BUBBLE;
	}

	key_event.pressed = true;

	if (k_msgq_put(&key_event_msgq, &key_event, K_NO_WAIT) != 0) {
		struct wire_x_key_event discard;
		(void)k_msgq_get(&key_event_msgq, &discard, K_NO_WAIT);
		(void)k_msgq_put(&key_event_msgq, &key_event, K_NO_WAIT);
	}

	k_work_submit_to_queue(zmk_display_work_q(), &key_event_work);
	return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(wire_x_lvgl_keypress, key_position_listener_cb);
ZMK_SUBSCRIPTION(wire_x_lvgl_keypress, zmk_position_state_changed);

static void update_display_for_layer(uint8_t layer, bool animate)
{
	if (main_obj == NULL) {
		return;
	}

	layer = MIN(layer, WIRE_X_LAYERS - 1);
	current_layer = layer;

	set_compact_label(layer_title, display_labels.layer[layer]);

	for (uint8_t i = 0; i < WIRE_X_LAYERS; i++) {
		uint32_t color = (i == layer) ? COLOR_ACCENT_2 : COLOR_GRID;
		lv_opa_t opa = (i == layer) ? LV_OPA_COVER : LV_OPA_50;
		lv_obj_set_style_bg_color(layer_marks[i], lv_color_hex(color), LV_PART_MAIN);
		lv_obj_set_style_bg_opa(layer_marks[i], opa, LV_PART_MAIN);
	}

	for (uint8_t i = 0; i < WIRE_X_ENCODERS; i++) {
		set_compact_label(encoder_value[i], display_labels.encoder[layer][i]);
		lv_obj_set_style_text_color(encoder_value[i], lv_color_hex(COLOR_ENC), LV_PART_MAIN);
		lv_obj_align(encoder_value[i], LV_ALIGN_CENTER, 0, 5);
	}

	for (uint8_t i = 0; i < WIRE_X_KEYS; i++) {
		set_compact_label(key_labels[i], display_labels.key[layer][i]);
		lv_obj_center(key_labels[i]);
	}

	if (animate) {
		animate_layer_change();
	}
}

static struct wire_x_display_state get_display_state(const zmk_event_t *eh)
{
	ARG_UNUSED(eh);
	return (struct wire_x_display_state){
		.layer = MIN(zmk_keymap_highest_layer_active(), WIRE_X_LAYERS - 1),
	};
}

static void display_state_changed(struct wire_x_display_state state)
{
	current_layer = MIN(state.layer, WIRE_X_LAYERS - 1);
	if (status_active) {
		update_status_ui();
		return;
	}
	if (idle_active) {
		return;
	}

	update_display_for_layer(current_layer, true);
}

static void display_label_update_work_cb(struct k_work *work)
{
	ARG_UNUSED(work);
	update_display_for_layer(current_layer, false);
}

ZMK_DISPLAY_WIDGET_LISTENER(wire_x_lvgl_tft, struct wire_x_display_state,
			    display_state_changed, get_display_state)
ZMK_SUBSCRIPTION(wire_x_lvgl_tft, zmk_layer_state_changed);
#if IS_ENABLED(CONFIG_ZMK_HID_INDICATORS)
ZMK_SUBSCRIPTION(wire_x_lvgl_tft, zmk_hid_indicators_changed);
#endif

static void splash_done_cb(struct k_work *work)
{
	ARG_UNUSED(work);

	if (splash_obj != NULL) {
		lv_obj_add_flag(splash_obj, LV_OBJ_FLAG_HIDDEN);
	}
	if (main_obj != NULL) {
		lv_obj_clear_flag(main_obj, LV_OBJ_FLAG_HIDDEN);
		animate_layer_change();
	}
}

K_WORK_DELAYABLE_DEFINE(splash_done_work, splash_done_cb);

static void build_splash(void)
{
	splash_obj = lv_obj_create(screen_obj);
	prep_obj(splash_obj);
	lv_obj_set_size(splash_obj, TFT_WIDTH, TFT_HEIGHT);
	lv_obj_add_style(splash_obj, &style_screen, LV_PART_MAIN);

	lv_obj_t *logo = lv_image_create(splash_obj);
	lv_image_set_src(logo, &splash_logo_image);
	lv_obj_center(logo);
}

static void build_status_ui(void)
{
	status_obj = lv_obj_create(screen_obj);
	prep_obj(status_obj);
	lv_obj_set_size(status_obj, TFT_WIDTH, TFT_HEIGHT);
	lv_obj_add_style(status_obj, &style_screen, LV_PART_MAIN);
	lv_obj_add_flag(status_obj, LV_OBJ_FLAG_HIDDEN);

	for (int y = 18; y < TFT_HEIGHT; y += 9) {
		make_rect(status_obj, 0, y, TFT_WIDTH, 1, COLOR_GRID_DIM, LV_OPA_20, 0);
	}

	make_rect(status_obj, 0, 0, TFT_WIDTH, 27, COLOR_HEADER, LV_OPA_COVER, 0);
	make_label(status_obj, "TAC-W", &lv_font_montserrat_18, COLOR_TEXT, 8, 4, 72,
		   LV_TEXT_ALIGN_LEFT);
	make_label(status_obj, "STATUS", &lv_font_montserrat_12, COLOR_ACCENT_2, 79, 8, 82,
		   LV_TEXT_ALIGN_CENTER);
	lv_obj_t *zmk_badge = make_rect(status_obj, 188, 4, 44, 18, COLOR_STATUS_CARD, LV_OPA_COVER,
					3);
	lv_obj_set_style_border_color(zmk_badge, lv_color_hex(COLOR_ACCENT_2), LV_PART_MAIN);
	lv_obj_set_style_border_width(zmk_badge, 1, LV_PART_MAIN);
	make_label(zmk_badge, "ZMK", &lv_font_montserrat_12, COLOR_STATUS_BLUE, 0, 0, 44,
		   LV_TEXT_ALIGN_CENTER);

	status_ble_badges[0] = make_status_profile_badge(status_obj, 7, 32, 0,
							 &status_ble_badge_labels[0],
							 &status_ble_badge_numbers[0]);
	status_ble_badges[1] = make_status_profile_badge(status_obj, 45, 32, 1,
							 &status_ble_badge_labels[1],
							 &status_ble_badge_numbers[1]);
	status_ble_badges[2] = make_status_profile_badge(status_obj, 83, 32, 2,
							 &status_ble_badge_labels[2],
							 &status_ble_badge_numbers[2]);
	status_usb_badge = make_status_icon_badge(status_obj, 124, 32, 30, 38, LV_SYMBOL_USB,
						  &status_usb_badge_label);
	status_route_ble_badge =
		make_status_icon_badge(status_obj, 159, 32, 30, 38, LV_SYMBOL_BLUETOOTH,
					&status_route_ble_badge_label);

	make_status_battery_widget(status_obj, 194, 32, 39, 38);

	status_lock_badges[0] =
		make_status_lock_badge(status_obj, 8, 74, "CAPS", &status_lock_labels[0]);
	status_lock_badges[1] =
		make_status_lock_badge(status_obj, 65, 74, "NUM", &status_lock_labels[1]);
	status_lock_badges[2] =
		make_status_lock_badge(status_obj, 122, 74, "SCRL", &status_lock_labels[2]);
	make_status_host_card(status_obj, 180, 74);

	status_uptime_value = make_status_value(status_obj, 7, 100, 126, 28, "UPTIME");
	status_layer_value = make_status_value(status_obj, 141, 100, 92, 28, "LAYER");
}

static void build_idle_ui(void)
{
	idle_obj = lv_obj_create(screen_obj);
	prep_obj(idle_obj);
	lv_obj_set_size(idle_obj, TFT_WIDTH, TFT_HEIGHT);
	lv_obj_add_style(idle_obj, &style_screen, LV_PART_MAIN);
	lv_obj_add_flag(idle_obj, LV_OBJ_FLAG_HIDDEN);

	load_idle_dance_frame(0);
	idle_cat_img = lv_image_create(idle_obj);
	lv_image_set_src(idle_cat_img, &idle_dance_image);
	lv_obj_set_pos(idle_cat_img, 0, 0);
	lv_obj_clear_flag(idle_cat_img, LV_OBJ_FLAG_SCROLLABLE);
}

static void build_main_ui(void)
{
	main_obj = lv_obj_create(screen_obj);
	prep_obj(main_obj);
	lv_obj_set_size(main_obj, TFT_WIDTH, TFT_HEIGHT);
	lv_obj_add_style(main_obj, &style_screen, LV_PART_MAIN);
	lv_obj_add_flag(main_obj, LV_OBJ_FLAG_HIDDEN);

	for (int y = HEADER_H + 4; y < TFT_HEIGHT; y += 6) {
		make_rect(main_obj, 0, y, TFT_WIDTH, 1, COLOR_GRID_DIM, LV_OPA_20, 0);
	}

	make_rect(main_obj, 0, 0, TFT_WIDTH, HEADER_H - 1, COLOR_HEADER, LV_OPA_COVER, 0);

	layer_title = make_label(main_obj, "LAYER 1", &lv_font_montserrat_18, COLOR_TEXT, 8, 12, 102,
				 LV_TEXT_ALIGN_LEFT);

	accent_bar = make_rect(main_obj, 8, 35, 47, 3, COLOR_ACCENT, LV_OPA_COVER, 1);

	for (uint8_t i = 0; i < WIRE_X_LAYERS; i++) {
		layer_marks[i] = make_rect(main_obj, 59 + (i * 10), 36, 6, 3,
					   i == 0 ? COLOR_ACCENT_2 : COLOR_GRID,
					   i == 0 ? LV_OPA_COVER : LV_OPA_50, 1);
	}

	make_vline(main_obj, 115, 0, HEADER_H, COLOR_ACCENT, LV_OPA_70);
	make_vline(main_obj, 178, 0, HEADER_H, COLOR_ACCENT, LV_OPA_70);
	make_hline(main_obj, HEADER_H - 1, COLOR_ACCENT, LV_OPA_COVER);
	make_hline(main_obj, HEADER_H, COLOR_BG, LV_OPA_60);

	for (uint8_t i = 0; i < WIRE_X_ENCODERS; i++) {
		int x = (i == 0) ? 118 : 181;
		int w = (i == 0) ? 57 : 56;

		encoder_zone[i] = lv_obj_create(main_obj);
		prep_obj(encoder_zone[i]);
		lv_obj_set_pos(encoder_zone[i], x, 4);
		lv_obj_set_size(encoder_zone[i], w, 35);
		lv_obj_add_style(encoder_zone[i], &style_header_zone, LV_PART_MAIN);
		lv_obj_add_style(encoder_zone[i], &style_key_pressed, LV_STATE_PRESSED);

		make_rect(encoder_zone[i], w - 8, 5, 3, 3, COLOR_ACCENT_2, LV_OPA_80, 1);
		make_label(encoder_zone[i], i == 0 ? "EN1" : "EN2", &lv_font_montserrat_8,
			   COLOR_TEXT_DIM, 5, 4, 30, LV_TEXT_ALIGN_LEFT);
		encoder_value[i] = make_label(encoder_zone[i], "", &lv_font_montserrat_12,
					      COLOR_ENC, 0, 0, w, LV_TEXT_ALIGN_CENTER);
		lv_obj_align(encoder_value[i], LV_ALIGN_CENTER, 0, 5);
	}

	for (uint8_t r = 0; r < GRID_ROWS; r++) {
		make_hline(main_obj, HEADER_H + CELL_H * r, COLOR_GRID, LV_OPA_COVER);
	}
	make_hline(main_obj, TFT_HEIGHT - 1, COLOR_GRID, LV_OPA_COVER);

	for (uint8_t c = 0; c <= GRID_COLS; c++) {
		make_vline(main_obj, CELL_W * c, HEADER_H, TFT_HEIGHT - HEADER_H, COLOR_GRID,
			   LV_OPA_COVER);
	}

	for (uint8_t row = 0; row < GRID_ROWS; row++) {
		for (uint8_t col = 0; col < GRID_COLS; col++) {
			uint8_t index = row * GRID_COLS + col;
			int x = (col * CELL_W) + 2;
			int y = HEADER_H + (row * CELL_H) + 2;

			key_cells[index] = lv_obj_create(main_obj);
			prep_obj(key_cells[index]);
			lv_obj_set_pos(key_cells[index], x, y);
			lv_obj_set_size(key_cells[index], CELL_W - 4, CELL_H - 4);
			lv_obj_add_style(key_cells[index], &style_key_base, LV_PART_MAIN);
			lv_obj_add_style(key_cells[index], &style_key_pressed, LV_STATE_PRESSED);

			key_labels[index] = make_label(key_cells[index], "", &lv_font_montserrat_12,
						       COLOR_KEY_TEXT, 0, 7, CELL_W - 4,
						       LV_TEXT_ALIGN_CENTER);
			lv_obj_center(key_labels[index]);
		}
	}
}

lv_obj_t *zmk_display_status_screen(void)
{
	init_styles();
	init_release_work();
	wire_x_display_backlight_set(true);
	sanitize_display_labels();
	k_work_init(&key_event_work, key_event_work_cb);
	k_work_init(&display_label_update_work, display_label_update_work_cb);
	k_work_init(&display_test_work, display_test_work_cb);
	k_work_init(&status_toggle_work, status_toggle_work_cb);
	k_work_init_delayable(&status_refresh_work, status_refresh_work_cb);
	k_work_init_delayable(&idle_anim_work, idle_anim_work_cb);

	current_layer = MIN(zmk_keymap_highest_layer_active(), WIRE_X_LAYERS - 1);

	screen_obj = lv_obj_create(NULL);
	prep_obj(screen_obj);
	lv_obj_set_size(screen_obj, TFT_WIDTH, TFT_HEIGHT);
	lv_obj_add_style(screen_obj, &style_screen, LV_PART_MAIN);

	build_main_ui();
	build_status_ui();
	build_idle_ui();
	update_display_for_layer(current_layer, false);
	build_splash();

	wire_x_lvgl_tft_init();
	k_work_schedule_for_queue(zmk_display_work_q(), &splash_done_work,
				  K_MSEC(SPLASH_DURATION_MS));

	return screen_obj;
}
