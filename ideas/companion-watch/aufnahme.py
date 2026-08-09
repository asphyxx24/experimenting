import serial
import wave
import struct
import sys
import time
from datetime import datetime

PORT = 'COM5'
SAMPLE_RATE = 16000

print(f"Verbinde mit {PORT}...")
try:
    ser = serial.Serial(PORT, 115200, timeout=60)
except Exception as e:
    print(f"Fehler: {e}")
    print("Stelle sicher dass der idf monitor geschlossen ist!")
    sys.exit(1)

# Fix 1: Buffer leeren — alte Daten von vorherigen Boots verwerfen
time.sleep(0.5)
ser.reset_input_buffer()

print("Bereit. Druecke jetzt RESET auf dem Board.")
samples = []
recording = False

while True:
    try:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
    except Exception:
        break
    if not line:
        continue
    if line.startswith('COUNTDOWN:'):
        sek = line.split(':')[1]
        print(f"  Startet in {sek}...")
        continue
    if 'AUFNAHME' in line:
        print(">>> JETZT SPRECHEN! (10 Sekunden) <<<")
        continue
    if line == 'AUDIO_START':
        print("Empfange Daten...")
        recording = True
        # Fix 2: Bei jedem AUDIO_START die Samples zuruecksetzen
        # — nur die LETZTE Aufnahme wird gespeichert
        samples = []
        continue
    if line == 'AUDIO_END':
        break
    if recording:
        try:
            val = int(line)
            val = max(-32768, min(32767, val))
            samples.append(struct.pack('<h', val))
            if len(samples) % 8000 == 0:
                print(f"  {len(samples) / SAMPLE_RATE:.1f}s empfangen...")
        except ValueError:
            pass

ser.close()

if not samples:
    print("Keine Daten empfangen!")
    sys.exit(1)

# Fix 3: Timestamp im Dateinamen — kein PermissionError wenn alte WAV noch offen ist
ts = datetime.now().strftime("%Y%m%d_%H%M%S")
filename = f'aufnahme_{ts}.wav'

with wave.open(filename, 'wb') as w:
    w.setnchannels(1)
    w.setsampwidth(2)
    w.setframerate(SAMPLE_RATE)
    w.writeframes(b''.join(samples))

print(f"\nGespeichert: {filename}")
print(f"Dauer: {len(samples) / SAMPLE_RATE:.1f} Sekunden")
print(f"Oeffne {filename} mit einem Mediaplayer!")
