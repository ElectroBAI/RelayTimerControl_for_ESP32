# Relay Timer Control (RTC) for ESP32

A robust, non-blocking Serial command interface for controlling relays with human-readable time durations.

---

## Features
*   **Human-Readable Commands**: Control devices using simple strings like `/bulb-ON-30s` or `/fan-ON-15min`.
*   **Multi-Command Support**: Chain actions together using the `&&` separator (e.g., `/fan-OFF && /bulb-ON`).
*   **Non-Blocking Timers**: Uses `millis()` logic to handle multiple timers simultaneously without freezing the processor.
*   **Flexible Units**: Supports `ms`, `sec`, `min`, `hr`, and `day`.
*   **Active-Low Optimized**: Specifically designed for common relay modules where a `LOW` signal triggers the relay.

---

## Hardware Requirements
*   **Microcontroller**: ESP32
*   **Relays**: 2-Channel Relay Module (Active-Low).
*   **Pins Used**:
    *   **GPIO 26**: Bulb / Light
    *   **GPIO 27**: Fan / Ceiling Fan

---

## Command Syntax
All commands must start with a `/` and follow the `device-state-duration` pattern.

| Component | Options |
| :--- | :--- |
| **Device** | `bulb`, `light`, `fan`, `ceilingfan` |
| **State** | `ON`, `OFF` |
| **Duration** | (Optional) `10s`, `500ms`, `2min`, `1hr`, `1day` |

### **Examples**
*   `/bulb-ON-10s` — Turns the light on for 10 seconds.
*   `/fan-ON` — Turns the fan on manually (no timer).
*   `/bulb-OFF && /fan-OFF` — Turns both devices off immediately.

---

## Technical Overview
This project demonstrates key embedded programming concepts:
1.  **State Machines**: Uses a `struct` to track the state of each relay independently.
2.  **String Parsing**: Efficiently splits serial input to extract device names and time multipliers.
3.  **Overflow Handling**: Includes logic to manage the `millis()` rollover that occurs every 49 days, ensuring long-term reliability for **Industrial & Embedded** style applications.

---

