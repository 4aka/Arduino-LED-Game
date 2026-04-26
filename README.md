# LED Strip Pixel Shooter (Addressable LED Strip Game)

Interactive arcade game based on Arduino Nano and WS2812B addressable LED strip. The player must defend their "base" from colored pixels falling from above by shooting at them with pixels of the corresponding color.

## 🛠 Hardware

The following components are required to assemble the project:
* **Microcontroller:** Arduino Nano.
* **LED Strip:** WS2812B (from 30 to 60 pixels).
* **Buttons:** 4 tactile momentary push buttons (Red shot, Green shot, Blue shot, Level up).
* **Resistor (for the strip):** 1 pc. rated at 330 Ohm – 470 Ohm. Installed in series between the Arduino pin (D6) and the `Data In` input of the strip to protect the first diode.
* **Capacitor (optional, but recommended):** Electrolytic, 1000 µF (6.3V or more). Installed in parallel with the strip's power supply (between 5V and GND) to smooth out current spikes.
* **Resistors for buttons:** **NOT NEEDED**. The internal pull-up resistors of the microcontroller are activated in the firmware (`INPUT_PULLUP`). 

### Pin Connection Diagram:
* **Strip (Data):** Pin `D6` (via resistor)
* **Green Button:** Pin `D2` ↔ `GND`
* **Red Button:** Pin `D3` ↔ `GND`
* **Blue Button:** Pin `D4` ↔ `GND`
* **Level Up Button:** Pin `D5` ↔ `GND`

---

## 💻 Software (Libraries)

One third-party library is used for the firmware to work:
* **[FastLED](https://github.com/FastLED/FastLED)** — a powerful and fast library for working with addressable strips. Installed via the Library Manager in the Arduino IDE.

---

## 🎮 Detailed Game Logic

### 1. Core Mechanics
* The game takes place on a one-dimensional LED strip. 
* **Top of the strip** (last pixel) — this is where "enemies" (falling pixels) are generated. They fall down towards the player.
* **Bottom of the strip** (0th pixel) — the player's "base". This pixel is always illuminated with a faint white light. 
* Enemies appear randomly: they have one of 3 colors (Red, Green, Blue) and are generated with a random distance from each other (from 1 to 5 empty steps).

### 2. Shooting and Controls
* The player uses buttons to generate a shot (Red, Green, or Blue).
* The shot flies out from the 1st position (right above the "base") and flies upwards to meet the enemies.
* **Feature:** The speed of the user's pixel (shot) is **twice as fast** as the falling speed of the enemies.
* Buttons are processed on the "press" event (state change from `HIGH` to `LOW`) with minimal debounce (50 ms), allowing the player to make very rapid series of shots without blocking the system. Up to 40 objects can be on the strip simultaneously.

### 3. Collision Mechanics
Since objects move towards each other at different speeds, both a direct collision and an "intersection" are checked (so that pixels don't skip over each other). Upon the meeting of a shot and an enemy:
* **Colors match:** Both pixels are destroyed. +1 successful hit is counted.
* **Colors DO NOT match:** The player's shot disappears, and the enemy pixel **is guaranteed to change its color** to any of the other three available (but not the one it was) and continues to fall down.

### 4. Levels and Progression
* There are 5 difficulty levels in the game. The level determines the falling speed of the enemies.
* **1st level:** Enemies fall at a speed of 2 pixels per second. With each level, the speed increases proportionally.
* The level increases **automatically** after every 50 accurate shots, or **manually** using the "Level Up" button.
* Level up indication: the entire strip briefly flashes with white light.

### 5. Game Over
* If the player didn't manage to shoot down an enemy, and the falling pixel reaches the 0th position (touches the "base"):
  1. Movement on the strip stops for 300 ms (so the player can see exactly which pixel caused the defeat).
  2. The entire strip flashes red for 500 ms.
  3. The game resets completely: the hit counter goes back to zero, the level returns to the 1st, and the game starts over.
