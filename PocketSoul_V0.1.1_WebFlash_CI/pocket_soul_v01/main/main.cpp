#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lvgl.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "pet_core.hpp"

static const char *TAG = "PocketSoul";
static pocket_soul::PetState pet;
static lv_obj_t *pet_group = nullptr;
static lv_obj_t *message_label = nullptr;
static lv_obj_t *status_label = nullptr;
static lv_obj_t *mood_label = nullptr;
static lv_obj_t *food_button = nullptr;
static lv_obj_t *play_button = nullptr;
static lv_timer_t *animation_timer = nullptr;
static uint8_t animation_frame = 0;
static bool happy_face = false;


// 16x16 pixel sprite. '.' is transparent; colors are intentionally few for a crisp pixel-art look.
static const char *sprite[16] = {
    "....1111........",
    "...122221.......",
    "..12222221......",
    ".1222222221.....",
    "122222222221....",
    "122223222321....",
    "122223222321....",
    "122222222221....",
    "122222222221....",
    ".12222222221.....",
    "..122222221......",
    "...122221.......",
    "....1111........",
    ".....33.........",
    "....3333........",
    "...3..3........."
};

static lv_color_t pixel_color(char c) {
    switch (c) {
        case '1': return lv_color_hex(0x6CCFF6); // outer glow/body edge
        case '2': return lv_color_hex(0xD9F5FF); // body
        case '3': return lv_color_hex(0xF6C66B); // star tail
        default: return lv_color_black();
    }
}


static void clear_sprite(void) {
    if (pet_group) lv_obj_clean(pet_group);
}

static void draw_sprite(bool blink) {
    if (!pet_group) return;
    clear_sprite();
    const int cell = 12;
    const int start_x = 0;
    const int start_y = 0;
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            char px = sprite[y][x];
            if (px == '.') continue;
            lv_obj_t *pixel = lv_obj_create(pet_group);
            lv_obj_remove_style_all(pixel);
            lv_obj_set_size(pixel, cell, cell);
            lv_obj_set_pos(pixel, start_x + x * cell, start_y + y * cell);
            lv_obj_set_style_bg_color(pixel, pixel_color(px), 0);
            lv_obj_set_style_bg_opa(pixel, LV_OPA_COVER, 0);
            lv_obj_set_style_radius(pixel, 0, 0);
            lv_obj_set_style_border_width(pixel, 0, 0);
        }
    }
    // Eyes and expression are separate pixels so the sprite remains easy to animate.
    lv_obj_t *eye1 = lv_obj_create(pet_group);
    lv_obj_remove_style_all(eye1);
    lv_obj_set_size(eye1, blink ? 12 : 8, blink ? 3 : 8);
    lv_obj_set_pos(eye1, 60, 65);
    lv_obj_set_style_bg_color(eye1, lv_color_hex(0x163448), 0);
    lv_obj_set_style_bg_opa(eye1, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(eye1, 1, 0);
    lv_obj_t *eye2 = lv_obj_create(pet_group);
    lv_obj_remove_style_all(eye2);
    lv_obj_set_size(eye2, blink ? 12 : 8, blink ? 3 : 8);
    lv_obj_set_pos(eye2, 120, 65);
    lv_obj_set_style_bg_color(eye2, lv_color_hex(0x163448), 0);
    lv_obj_set_style_bg_opa(eye2, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(eye2, 1, 0);
    lv_obj_t *mouth = lv_obj_create(pet_group);
    lv_obj_remove_style_all(mouth);
    lv_obj_set_size(mouth, happy_face ? 14 : 8, 4);
    lv_obj_set_pos(mouth, 94, 92);
    lv_obj_set_style_bg_color(mouth, lv_color_hex(0x5B8FA8), 0);
    lv_obj_set_style_bg_opa(mouth, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(mouth, 2, 0);
}


static void update_status(void) {
    if (!status_label || !mood_label) return;
    char line[80];
    snprintf(line, sizeof(line), "Hunger %u   Energy %u   Bond %u", pet.hunger, pet.energy, pet.affection);
    lv_label_set_text(status_label, line);
    const char *mood = "Calm";
    switch (pocket_soul::current_mood(pet)) {
        case pocket_soul::Mood::Happy: mood = "Happy"; break;
        case pocket_soul::Mood::Sleepy: mood = "Sleepy"; break;
        case pocket_soul::Mood::Hungry: mood = "Hungry"; break;
        case pocket_soul::Mood::Curious: mood = "Curious"; break;
        case pocket_soul::Mood::Shy: mood = "Shy"; break;
        default: mood = "Calm"; break;
    }
    lv_label_set_text_fmt(mood_label, "Mood: %s", mood);
}

static void pet_event_cb(lv_event_t *e) {
    (void)e;
    auto event = pocket_soul::interact(pet, pocket_soul::Pet);
    happy_face = true;
    draw_sprite(false);
    lv_label_set_text(message_label, event.text);
    update_status();
}
static void feed_event_cb(lv_event_t *e) {
    (void)e;
    auto event = pocket_soul::interact(pet, pocket_soul::Feed);
    happy_face = true;
    draw_sprite(false);
    lv_label_set_text(message_label, event.text);
    update_status();
}
static void play_event_cb(lv_event_t *e) {
    (void)e;
    auto event = pocket_soul::interact(pet, pocket_soul::Play);
    happy_face = event.mood == pocket_soul::Mood::Happy;
    draw_sprite(false);
    lv_label_set_text(message_label, event.text);
    update_status();
}
static void anim_cb(lv_timer_t *t) {
    (void)t;
    animation_frame++;
    const bool blink = (animation_frame % 18 == 0);
    if (animation_frame % 45 == 0) happy_face = false;
    draw_sprite(blink);
}

static lv_obj_t *make_button(lv_obj_t *parent, const char *text, lv_event_cb_t cb, int x) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 110, 52);
    lv_obj_set_pos(btn, x, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x25445A), 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x4D8DAA), 0);
    lv_obj_set_style_border_width(btn, 2, 0);
    lv_obj_set_style_radius(btn, 12, 0);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    return btn;
}


extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Pocket Soul V0.1.1 - pixel pet display milestone");
    bsp_display_start();
    bsp_display_lock(-1);

    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x071522), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "POCKET SOUL");
    lv_obj_set_style_text_color(title, lv_color_hex(0xB6E8FF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

    lv_obj_t *subtitle = lv_label_create(screen);
    lv_label_set_text(subtitle, "A tiny life, a little universe");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0x668CA1), 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 55);

    pet_group = lv_obj_create(screen);
    lv_obj_remove_style_all(pet_group);
    lv_obj_set_size(pet_group, 192, 192);
    lv_obj_align(pet_group, LV_ALIGN_CENTER, 0, -20);
    lv_obj_add_flag(pet_group, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(pet_group, pet_event_cb, LV_EVENT_CLICKED, nullptr);
    draw_sprite(false);

    lv_obj_t *name = lv_label_create(screen);
    lv_label_set_text(name, "XINGZAI  /  星仔");
    lv_obj_set_style_text_color(name, lv_color_hex(0xE5F8FF), 0);
    lv_obj_align(name, LV_ALIGN_CENTER, 0, 115);

    mood_label = lv_label_create(screen);
    lv_obj_set_style_text_color(mood_label, lv_color_hex(0x94CFEA), 0);
    lv_obj_align(mood_label, LV_ALIGN_CENTER, 0, 145);

    status_label = lv_label_create(screen);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x7798AA), 0);
    lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 175);
    update_status();

    message_label = lv_label_create(screen);
    lv_label_set_text(message_label, "Tap Xingzai to say hello!");
    lv_obj_set_style_text_color(message_label, lv_color_hex(0xD0EAF5), 0);
    lv_obj_align(message_label, LV_ALIGN_CENTER, 0, 215);

    lv_obj_t *buttons = lv_obj_create(screen);
    lv_obj_remove_style_all(buttons);
    lv_obj_set_size(buttons, 240, 56);
    lv_obj_align(buttons, LV_ALIGN_BOTTOM_MID, 0, -30);
    food_button = make_button(buttons, "Feed", feed_event_cb, 0);
    play_button = make_button(buttons, "Play", play_event_cb, 130);

    animation_timer = lv_timer_create(anim_cb, 500, nullptr);
    (void)animation_timer;
    bsp_display_unlock();
}
