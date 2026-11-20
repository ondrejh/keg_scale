#include "board.h"
#include "ui_input.h"

#define CENTER_R        110     /* px, centre tap zone radius */
#define DTAP_MS         300     /* double tap window (centre only) */
#define READ_PERIOD_MS  15      /* touch polling, fast enough for flicks */

static ui_input_cb_t s_cb;
static lv_timer_t *s_dtap_timer;
static bool s_swiped;           /* current press already produced a swipe */

const char *ui_input_name(ui_input_t ev)
{
    static const char *const names[] = {
        "tap left", "tap right", "tap top", "tap bottom", "tap center",
        "dtap center", "swipe left", "swipe right", "swipe up", "swipe down",
    };
    return ev < sizeof(names) / sizeof(names[0]) ? names[ev] : "?";
}

static void emit(ui_input_t ev)
{
    if (s_cb)
        s_cb(ev);
}

static void dtap_timeout(lv_timer_t *t)
{
    s_dtap_timer = NULL;        /* one-shot timer deletes itself */
    emit(UI_TAP_CENTER);
}

static void on_pressed(lv_event_t *e)
{
    s_swiped = false;
}

static void on_short_click(lv_event_t *e)
{
    if (s_swiped)
        return;

    lv_point_t p;
    lv_indev_get_point(lv_indev_active(), &p);
    int32_t dx = p.x - BOARD_LCD_H_RES / 2;
    int32_t dy = p.y - BOARD_LCD_V_RES / 2;

    if (dx * dx + dy * dy < CENTER_R * CENTER_R) {
        if (s_dtap_timer) {
            lv_timer_delete(s_dtap_timer);
            s_dtap_timer = NULL;
            emit(UI_DTAP_CENTER);
        } else {
            s_dtap_timer = lv_timer_create(dtap_timeout, DTAP_MS, NULL);
            lv_timer_set_repeat_count(s_dtap_timer, 1);
        }
        return;
    }

    if (LV_ABS(dx) > LV_ABS(dy))
        emit(dx < 0 ? UI_TAP_LEFT : UI_TAP_RIGHT);
    else
        emit(dy < 0 ? UI_TAP_TOP : UI_TAP_BOTTOM);
}

/* LVGL sends GESTURE to the object under the finger; children bubble it to the screen. */
static void on_gesture(lv_event_t *e)
{
    lv_indev_t *indev = lv_indev_active();
    if (!indev)
        return;
    s_swiped = true;
    lv_indev_wait_release(indev);
    switch (lv_indev_get_gesture_dir(indev)) {
    case LV_DIR_LEFT:   emit(UI_SWIPE_LEFT);  break;
    case LV_DIR_RIGHT:  emit(UI_SWIPE_RIGHT); break;
    case LV_DIR_TOP:    emit(UI_SWIPE_UP);    break;
    case LV_DIR_BOTTOM: emit(UI_SWIPE_DOWN);  break;
    default: break;
    }
}

void ui_input_init(lv_indev_t *indev, ui_input_cb_t cb)
{
    s_cb = cb;
    if (!indev)
        return;
    lv_timer_set_period(lv_indev_get_read_timer(indev), READ_PERIOD_MS);
    lv_indev_add_event_cb(indev, on_pressed, LV_EVENT_PRESSED, NULL);
    lv_indev_add_event_cb(indev, on_short_click, LV_EVENT_SHORT_CLICKED, NULL);
    /* screens are rebuilt by cleaning children; the screen object itself persists */
    lv_obj_add_event_cb(lv_screen_active(), on_gesture, LV_EVENT_GESTURE, NULL);
}
