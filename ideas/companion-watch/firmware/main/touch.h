#pragma once
#include <stdbool.h>

void touch_init(void);

// Steigende Flanke: true genau einmal beim Aufsetzen des Fingers.
bool touch_tapped(void);

// Zustand: true solange ein Finger aufliegt. Basis fuer Push-to-Talk.
bool touch_is_down(void);
