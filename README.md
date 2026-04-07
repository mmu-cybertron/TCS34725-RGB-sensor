# Color Sensor & Layout Identification System

This module handles real-time color detection using the **TCS34725** sensor, manages a sequence of 8 resource scans, and identifies the corresponding layout from a pre-defined table of 576 possibilities.
(Refer to the TCS34725_changes folder for the updated code)

---

## Main Functionalities

### 1. Calibration Mode
To normalize readings and reduce the impact of ambient lighting, the system includes a manual calibration routine.
* **Action:** Press **both buttons (PC8 + PC9)** simultaneously for approximately **2 seconds**.
* **Process:** The system captures raw data to compute RGB multipliers ($r_{mult}$, $g_{mult}$, $b_{mult}$).
* **Feedback:** All LEDs turn on once calibration is complete and multipliers are applied.

### 2. Color Detection & Storage (PC8)
The system tracks a sequence of 8 colors (4 branches, 2 slots each) starting at `index_ptr = 0`.
* **Action:** Press **PC8** to scan a color.
* **Logic:** * Captures RGBC data and applies normalization multipliers.
    * Uses **Euclidean distance** ($2\Delta R^2 + 4\Delta G^2 + 3\Delta B^2$) against predefined reference values.
    * **Empty Scan Protection:** If the "Clear" channel is below threshold (no object present), the system ignores the press and flashes an error.
    * Stores the result in `colorArray` and increments the index.

### 3. Undo Controls (PC9)
Manual overrides to ensure the scanned sequence is 100% accurate.
* **Action:** Press **PC9** once to decrement the `index_ptr` by 1, allowing you to overwrite the last scanned color.

---

## Mode Selection (The Toggle Switch)

The system state is governed by a physical **Toggle Switch** (connected to **PC2**). This acts as a master gate between the collection phase and final execution.

* **Collection Mode (Switch OFF):** System is in "Listening Mode." Store (PC8) and Undo (PC9) buttons are active to populate the `colorArray[8]`.
* **Game Mode (Switch ON):** System "locks" the data and initiates the matching sequence against `g_layouts[576]`.
    * **Success:** A **Layout ID** is identified, and **all LEDs** light up.
    * **Failure:** If the array is incomplete or no match is found, the **Red LED** lights up as an error signal.

---

## LED Feedback Logic

| Status / Color | LED Behavior |
| :--- | :--- |
| **Red Detected** | Blinks **Success**, then lights Red LED (PB12) |
| **Green Detected** | Blinks **Success**, then lights Green LED (PC5) |
| **Yellow Detected** | Blinks **Success**, then lights Yellow LED (PA12) |
| **Blue Detected** | Blinks **Success**, then lights all 3 LEDs |
| **Success** | Short simultaneous blink (All LEDs) |
| **Error / Full** | Rapid simultaneous toggle (3 times) |

---

## Hardware Configuration

* **Sensor:** TCS34725 (I2C)
* **Toggle Switch:** PC2
* **Main Trigger:** PC8 (Store)
* **Alt Trigger:** PC9 (Undo)
* **Indicators:** PB12 (Red LED), PC5 (Green LED), PA12 (Yellow LED)

> **Note:** Ensure internal Pull-Down resistors are enabled for all input pins (PC8, PC9, and Toggle) to prevent ghost triggers from EMI.
