# CAN Bus Gauge & VFD Configuration

Settings for each channel are stored in EEPROM and sent over the USB serial port (115200 baud). Switch the dashboard to **CAN mode** by pressing the mode button until the VFD shows `CAN`.

---

## Channel Assignments

| Channel | Output |
|---------|--------|
| 0 | Gauge 0 (e.g. RPM) |
| 1 | Gauge 1 (e.g. MPH) |
| 2 | Gauge 2 (e.g. Fuel) |
| 3 | Gauge 3 (e.g. Temp) |
| 4 | VFD numeric display |

---

## Config Fields

Each channel has six fields:

| Field | Size | Description |
|-------|------|-------------|
| `PGN` | 3 bytes | Upper 3 bytes of the CAN ID (big-endian). Set a byte to `0xFF` to wildcard it. |
| `Source` | 1 byte | Lowest byte of the CAN ID (J1939 source address). Set to `0xFF` to match any. |
| `DataStart` | 1 byte | Index of the first data byte to read (0–7). |
| `DataLen` | 1 byte | Number of bytes to read (1–4). Data bytes are little-endian — `DataStart` is the LSB. |
| `Gain` | float | Multiplied against the raw extracted value. |
| `Offset` | float | Added after the gain multiplication. |

The final value sent to the gauge or VFD is:

```
result = raw * gain + offset
```

### Little-Endian Extraction

For a 2-byte signal at `DataStart=0` with frame bytes `[0xA0, 0x02, ...]`:

```
raw = 0xA0 + (0x02 << 8) = 0x02A0 = 672
```

---

## CAN ID Encoding

The four bytes `PGN[0]`, `PGN[1]`, `PGN[2]`, `Source` form a 32-bit CAN identifier (big-endian):

```
targetId = (PGN[0] << 24) | (PGN[1] << 16) | (PGN[2] << 8) | Source
```

**Standard 11-bit ID** (e.g. `0x1B0`):

```
PGN = 0001B0   src = FF
→ targetId = 0x000001B0 = 0x1B0  (source wildcarded)
```

**J1939 29-bit ID** (e.g. `0x18FEF100`):

```
PGN = 18FEF1   src = 00
→ targetId = 0x18FEF100
```

### Wildcards

Set any byte of PGN or src to `FF` to ignore it during matching.

```
PGN = 18FEF1   src = FF
→ matches 0x18FEF1xx (any source address)
```

---

## Serial Config Protocol

Send a comma-separated ASCII line terminated with `\r`. Works in any display mode.

```
C<ch>,<PGN>,<src>,<dataStart>,<dataLen>,<gain>,<offset>\r
```

- `PGN` — 6 hex digits (no prefix), upper-case or lower-case, e.g. `0000CF`
- `src` — 2 hex digits, e.g. `FF` for wildcard
- `dataStart`, `dataLen` — decimal integers
- `gain`, `offset` — floating point

```
C0,0000CF,FF,0,2,0.25,0.0\r
```

The device responds with a confirmation line, e.g.:

```
CFG ch0: PGN=0000CF src=FF dStart=0 dLen=2 gain=0.2500 off=0.0000
```

### Query a Channel

Send `Q` followed by the channel digit and `\r` to read back saved settings:

```
Q0\r   → prints channel 0 config
Q4\r   → prints channel 4 (VFD) config
```

---

## Examples

### RPM on ID 0x0CF, bytes 0–1 (little-endian)
Raw convention: `(byte1 << 8 | byte0) / 4 = RPM`

```
C0,0000CF,FF,0,2,0.25,0.0\r
```

### Vehicle Speed on ID 0x1B0, bytes 0–1 (little-endian)
Raw convention: `(byte1 << 8 | byte0) / 128 = MPH`

```
C1,0001B0,FF,0,2,0.0078125,0.0\r
```

### Coolant Temp on ID 0x1E4, byte 0 (single byte)
Raw convention: `byte0 - 40 = °C`, mapped to 0–100 gauge range (40–120 °C → 0–100%)

```
C3,0001E4,FF,0,1,1.25,-50.0\r
```

### VFD showing fuel level on ID 0x1E4, byte 1 (single byte)
Raw convention: 0–255 → 0–100%

```
C4,0001E4,FF,1,1,0.3922,0.0\r
```

---

## Discovering IDs

Set `CAN_SNIFF_MODE 1` in `CANDash.h` and monitor the serial output. Every received frame is printed:

```
CAN 0x000001B0 [8]: A0 02 00 00 00 00 00 00
```

This shows the full 32-bit identifier, data length, and all 8 data bytes in frame order (byte 0 first). Remember that multi-byte signals are little-endian — byte 0 is the LSB. In the example above, the 2-byte value at bytes 0–1 is `0x02A0 = 672`.

Set `CAN_SNIFF_MODE 0` when done to reduce serial traffic.
