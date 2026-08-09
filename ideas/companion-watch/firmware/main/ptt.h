#pragma once
#include <stdbool.h>
#include <stdint.h>

// Push-to-Talk ueber den Touchscreen: Finger auflegen und halten nimmt auf,
// loslassen sendet. Bewusst gegen die urspruengliche Planung (Side-Push-
// Knopf) entschieden — spart Bauteil, GPIO und Gehaeuse-Durchbruch.

typedef enum {
    PTT_EVENT_NONE = 0,
    PTT_EVENT_TAP,     // kurz beruehrt — bewusst kein PTT, fuer UI-Bedienung
    PTT_EVENT_START,   // Aufnahme beginnen
    PTT_EVENT_STOP,    // Aufnahme beenden und absenden
    PTT_EVENT_ABORT,   // Aufnahme verwerfen (Maximaldauer ueberschritten)
} ptt_event_t;

// Wichtig: Tap und PTT teilen sich dieselbe Flaeche. Deshalb liefert dieses
// Modul beide Events — wer stattdessen touch_tapped() auswertet, schaltet
// beim Aufsetzen des Fingers ungewollt mit, bevor klar ist, ob daraus ein
// PTT-Halten wird.

void ptt_init(void);

// Einmal pro Loop-Durchlauf aufrufen. Liefert hoechstens ein Event.
ptt_event_t ptt_update(void);

bool ptt_is_recording(void);

// Laufende Aufnahmedauer in ms (0 wenn nicht aufgenommen wird).
uint32_t ptt_recording_ms(void);
