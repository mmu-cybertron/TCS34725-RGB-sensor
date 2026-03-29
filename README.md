## Color Sensor & Layout Identification System

This module handles real-time color detection using the **TCS34725** sensor, manages a sequence of 8 resource scans, and identifies the corresponding layout from a pre-defined table of 576 possibilities.

---

## Main Functionalities

### 1. Calibration Mode
To normalize readings and reduce the impact of ambient lighting, the system includes a manual calibration routine.
* **Action:** Press **both buttons (PB0 + PB1)** simultaneously for approximately **2 seconds**.
* **Process:** The system captures raw data to compute RGB multipliers ($rmult$, $gmult$, $bmult$).
* **Feedback:** Both LEDs will turn on once the calibration is complete and multipliers are applied.

### 2. Color Detection & Storage (PB0)
The system tracks a sequence of 8 colors (4 branches, 2 slots each) starting at `index_ptr = 0`.
* **Action:** Press **PB0** to scan a color.
* **Logic:** * Captures RGBC data and applies the normalization multipliers.
    * Uses **Euclidean distance** ($2\Delta R^2 + 4\Delta G^2 + 3\Delta B^2$) against predefined reference values.
    * Stores the result in `colorArray` and increments the index.
* **Note:** Reference values (`refRed`, `refGreen`, etc.) should be updated with physical resource colors.

### 3. Undo Controls (PB1)
Manual overrides to ensure the scanned sequence is 100% accurate.
* **Undo:** Press **PB1** once to decrement the `index_ptr` by 1, allowing you to overwrite the last scanned color.

---

## LED Feedback Logic

The system uses a Red and Green LED pair to communicate detection results and system status:

| Status / Color | LED Behavior |
| :--- | :--- |
| **Red Detected** | Red LED (PB8) flashes |
| **Green Detected** | Green LED (PB9) flashes |
| **Blue Detected** | LEDs **alternate** flashing, then turn OFF |
| **Yellow Detected** | Both LEDs flash, then stay **ON** |
| **Success** | Short simultaneous blink |
| **Error / Full** | Rapid simultaneous toggle (3 times) |

---

## Interfacing & Table Matching

The logic is designed to interface with an external `generated_layouts.h` file.
* **The Table:** `g_layouts[576]` contains every valid permutation of the 8 resources (24 possibilities per slot).
* **The Matcher:** Once 8 colors are stored, the system runs a search algorithm to find the exact index in the table. This **Layout ID** is the final output used by the robot for pathing.

---

## Hardware Configuration
* **Sensor:** TCS34725 (I2C)
* **Main Trigger:** PB0 (Store)
* **Alt Trigger:** PB1 (Undo/Reset)
* **Indicators:** PB8 (Red LED), PB9 (Green LED)
