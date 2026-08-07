#include "ptt.h"
#include "touch.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "ptt";

// Ein Aermel, der ueber das Display streift, ist fuer einen kapazitiven
// Sensor auch ein Finger. Erst nach dieser Haltedauer startet die Aufnahme.
#define MIN_HOLD_MS 200

// Kapazitive Sensoren verlieren kurz den Kontakt, wenn der Finger rollt.
// Ohne diese Karenz wuerde die Aufnahme mitten im Satz abbrechen.
#define RELEASE_DEBOUNCE_MS 80

// Notbremse, falls etwas dauerhaft auf dem Display aufliegt.
#define MAX_RECORD_MS 30000

typedef enum {
    ST_IDLE,          // nichts liegt auf
    ST_PENDING,       // Finger liegt auf, Mindesthaltedauer laeuft
    ST_RECORDING,     // Aufnahme laeuft
    ST_RELEASING,     // Finger scheint weg — Karenzzeit laeuft
    ST_BLOCKED,       // nach Abbruch: warten bis wirklich losgelassen wurde
} state_t;

static state_t state = ST_IDLE;
static int64_t state_since_ms = 0;
static int64_t record_start_ms = 0;

static int64_t now_ms(void)
{
    return esp_timer_get_time() / 1000;
}

static void go(state_t next)
{
    state = next;
    state_since_ms = now_ms();
}

void ptt_init(void)
{
    go(ST_IDLE);
    ESP_LOGI(TAG, "PTT ready — hold the screen to talk");
}

ptt_event_t ptt_update(void)
{
    bool down = touch_is_down();
    int64_t now = now_ms();

    switch (state) {
    case ST_IDLE:
        if (down) go(ST_PENDING);
        return PTT_EVENT_NONE;

    case ST_PENDING:
        if (!down) {
            // Zu kurz fuer PTT — als normalen Tap melden. Der Aufrufer
            // erfaehrt davon erst hier, nicht schon beim Aufsetzen.
            go(ST_IDLE);
            return PTT_EVENT_TAP;
        } else if (now - state_since_ms >= MIN_HOLD_MS) {
            record_start_ms = now;
            go(ST_RECORDING);
            ESP_LOGI(TAG, "recording started");
            return PTT_EVENT_START;
        }
        return PTT_EVENT_NONE;

    case ST_RECORDING:
        if (now - record_start_ms >= MAX_RECORD_MS) {
            go(ST_BLOCKED);
            ESP_LOGW(TAG, "max duration reached — aborting");
            return PTT_EVENT_ABORT;
        }
        if (!down) go(ST_RELEASING);
        return PTT_EVENT_NONE;

    case ST_RELEASING:
        if (down) {
            // War nur ein Aussetzer — weiter aufnehmen.
            go(ST_RECORDING);
        } else if (now - state_since_ms >= RELEASE_DEBOUNCE_MS) {
            go(ST_IDLE);
            ESP_LOGI(TAG, "recording stopped (%lld ms)", now - record_start_ms);
            return PTT_EVENT_STOP;
        }
        return PTT_EVENT_NONE;

    case ST_BLOCKED:
        if (!down) go(ST_IDLE);
        return PTT_EVENT_NONE;
    }

    return PTT_EVENT_NONE;
}

bool ptt_is_recording(void)
{
    return state == ST_RECORDING || state == ST_RELEASING;
}

uint32_t ptt_recording_ms(void)
{
    if (!ptt_is_recording()) return 0;
    return (uint32_t)(now_ms() - record_start_ms);
}
