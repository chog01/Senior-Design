# Design Decisions

Key engineering decisions for Garden Sentry, with the options considered and the reasoning behind each choice. Entries marked **Open** are still being evaluated.

| # | Area | Decision |
|---|---|---|
| 1 | Architecture | Central Pi 5 for recognition; Pi Zero 2 W + Teensy 4.0 per sprayer unit |
| 2 | Motion control | Teensy 4.0 handles real-time motor control; Pi Zero 2 W handles camera and networking |
| 3 | Motor drivers | TMC2209 in UART mode on one shared bus |
| 4 | Motor supply | 24 V VM for 12 V-rated steppers |
| 5 | Power source | Wall power |
| 6 | Position sensing | Absolute magnetic sensing, no homing routine |
| 7 | Wiring | Cables routed through the center post, no slip ring |
| 8 | Power states | Low-power idle, PIR wake |
| 9 | Night vision | IMX462 IR-cut camera + 850 nm IR LED ring |
| 10 | Water system | Tank + pump, regulated to ~35 psi |
| 11 | PCB partitioning | Three boards: Turret_Driver, PI_Board, PIR_Board |
| 12 | Board protection | eFuse on the input, plus per-rail, solenoid, and I/O protection as good practice |
| 13 | Build | In-house reflow assembly |
| 14 | Demo plan | Phased: motors, click-to-aim, and recognition first; then water and full integration |

---

## 1. Distributed compute: central Pi 5, Pi Zero 2 W per unit

**Decision:** A single Raspberry Pi 5 runs image recognition for the whole yard. Each sprayer unit carries a Pi Zero 2 W that streams video to the Pi 5 over Wi-Fi and receives aim commands.

**Alternatives considered:** A Pi 5 (8 GB) on every unit; Teensy 4.0 + ESP32-C6; ESP32-S3.

**Why:** The product is meant to scale to multiple units covering one area. A Pi 5 per unit was too expensive and hard to source. Centralizing recognition keeps each unit cheap and small, while the Pi Zero 2 W still provides Linux, a CSI camera interface, and Wi-Fi.

## 2. Split real-time control from the Pi

**Decision:** A Teensy 4.0 on the Turret_Driver board drives the motors and reads position sensors. The Pi Zero 2 W talks to it over UART.

**Alternatives considered:** Teensy 3.1, Raspberry Pi Pico, driving the motors directly from the Pi.

**Why:** Step generation and closed-loop aiming need deterministic timing that Linux doesn't guarantee. The Teensy 4.0 (600 MHz Cortex-M7) has headroom for the control loop, and a UART link keeps the Pi-to-Teensy interface simple.

## 3. TMC2209 drivers in UART mode

**Decision:** Two TMC2209 drivers, configured over a single shared UART (Teensy pins 0/1) and addressed individually.

**Alternatives considered:** A4988.

**Comparison:**

| | TMC2209 | A4988 |
|---|---|---|
| Motor supply range | 4.75–29 V | 8–35 V |
| Microstepping | Up to 1/256 (lower settings interpolated to 256) | Up to 1/16 |
| Configuration | Single-wire UART: current, microsteps, chopper mode, hold current | Trim pot for current, strap pins for microsteps |
| Chopper modes | StealthChop2 (near-silent) and SpreadCycle (high-speed torque) | Fixed mixed-decay chopper |
| Diagnostics | Over-temp, short, open-load, and stall flags readable over UART | None beyond thermal shutdown |
| Load features | StallGuard4 stall detection, CoolStep load-adaptive current | None |
| Output MOSFETs | ~0.2 Ω, so it runs cooler | Higher R<sub>DS(on)</sub>, so it runs hotter at the same current |
| Cost and complexity | More expensive; UART setup adds firmware work | Cheap, STEP/DIR only, works with no setup |

**Why TMC2209 won:**
- **Smooth, precise aiming.** Interpolated microstepping and StealthChop remove the low-speed vibration and resonance the A4988 shows. That matters when the turret moves slowly to track a target and has to settle on a precise angle.
- **Quiet operation.** A unit meant to deter animals shouldn't whine while idle or tracking.
- **Current set in firmware.** Run current suits the 350 mA motors, and a lower hold current cuts heat and power while the turret is stationary. No trim pots to tune or drift.
- **Visibility into faults.** The Teensy can read driver status and react to a short, over-temperature, or stall during operation instead of silently losing steps.
- **Fewer pins.** Both drivers share one UART (unique addresses), so configuring them costs only two Teensy pins.

**Trade-offs accepted:**
- **Voltage margin.** The 29 V absolute maximum leaves little headroom above the 24 V rail. SMBJ26A TVS diodes on each VM rail clamp transients below that limit.
- **Firmware work.** The UART driver configuration has to be written and tested before the motors move at all.
- **Higher cost.** It costs more per driver than an A4988, which is acceptable with only two axes.

## 4. 24 V motor supply

**Decision:** VM is 24 V, although the Adafruit 324 NEMA 17 motors are rated 12 V / 350 mA.

**Alternatives considered:** 12 V VM.

**Why:** With a current-regulating chopper driver, the coil current is limited by the driver, not the supply. A higher supply voltage makes the current rise faster each step, which preserves torque at higher speeds, and fast aiming needs that. The 24 V water solenoid (14 W, ~0.5 A) also runs from the same rail. The trade-off is a more complex power section (eFuse, buck converter, higher-voltage TVS), which was weighed and accepted.

## 5. Wall power

**Decision:** The unit runs from a wall supply.

**Alternatives considered:** A custom 3S Li-ion pack, a LiFePO4 pack, solar charging.

**Why:** The unit is demonstrated indoors at the senior design expo, not deployed long-term. A small solar cell couldn't supply enough power at a reasonable price, and wall power removes battery charging and runtime concerns for the demo.

## 6. Absolute position sensing, no homing

**Decision:** Each axis uses a magnetic angle sensor for absolute position. Pan uses an Adafruit TMAG5273 3D Hall sensor with a ring magnet.

**Alternatives considered:** Limit-switch homing on startup; AS5600 sensors on both axes.

**Why:** When the PIR detects motion, the turret has to aim almost immediately. A homing sweep at wake-up would cost time and could scare the animal off before the spray. Absolute sensors give a known position at power-up.

**Open:** For tilt, choosing between an AS5600 on the far arm and a magnet embedded in the printed motor-side hub (two glued halves). The hub option keeps the tilt shaft solid.

## 7. No slip ring

**Decision:** All wiring runs down through the center post. There's no slip ring, so pan travel is limited.

**Alternatives considered:** A slip ring for continuous 360° pan, with water and electrical paths kept separate.

**Why:** It removes a costly part and a failure point, and avoids passing water and power through the same rotating joint. A unit doesn't need endless rotation to cover its area.

**Open:** The final pan range hasn't been set yet. The goal is as much travel as the center-post cable routing allows without straining the wiring. It will be set by testing cable flex on the assembled turret and enforced with soft limits in firmware, using the absolute position sensor (#6).

## 8. Low-power idle with PIR wake

**Decision:** The unit idles in a low-power state, and PIR sensors wake it. The IR LEDs turn on only after motion is detected.

**Why:** The IR illuminator and camera processing are the biggest continuous loads. Keeping them off until there's motion cuts idle power and heat.

**Designed for battery operation:** The expo unit runs on wall power (see #5), but a real yard deployment would be battery-powered, with no outlet near the garden. The power architecture is built so that switch needs no redesign:
- **Idle draw decides runtime.** A sprayer unit spends almost all its time waiting. If the camera, IR LEDs, and motors stay off until a PIR trips, average current is set by the PIR sensors and sleep current, not by the active loads. That's the difference between a battery lasting hours and lasting days.
- **PIR sensors are nearly free to run.** The BS612 draws only microamps, so leaving several always on costs almost nothing.
- **Active loads are short bursts.** Detection, aiming, and spraying last seconds, so a modest pack can supply the peak current while the average stays low.
- **The input stage is already set up for a battery.** A 6S Li-ion pack (25.2 V fully charged) can replace the wall supply directly. The eFuse's 19 V undervoltage cutoff (about 3.2 V per cell) already protects it from over-discharge, and its 26.6 V overvoltage cutoff sits just above a full charge (see #12).
- **Absolute position sensing helps too.** With no homing sweep (see #6), the unit can sleep completely and still aim right after waking, so the motors don't have to stay powered while idle.

## 9. Night vision

**Decision:** Waveshare IMX462 IR-cut camera with six OSRAM SFH 4554 850 nm IR LEDs on the PI_Board, switched by a MOSFET from a Pi GPIO.

**Alternatives considered:** IMX291 camera.

**Why:** Deer and small animals are most active at night, so the unit has to see in low light. The IMX462 has strong near-IR sensitivity and an IR-cut filter, so daytime color stays correct. 850 nm LEDs light the scene with only a faint visible glow.

## 10. Water system: tank + pump

**Decision:** Pump → regulator → solenoid valve → flexible tube → nozzle, targeting ~35 psi for a few yards of range. The pump is powered separately; the Turret_Driver switches only the solenoid.

**Alternatives considered:** Feeding straight from a garden hose spigot.

**Why:** A tank makes the demo self-contained, with no spigot at the expo. The regulator holds the nozzle pressure steady, so range stays repeatable as pump output varies.

## 11. Three-board partitioning

**Decision:** Turret_Driver (power, Teensy, motor drivers, solenoid), PI_Board (Pi Zero 2 W + IR LEDs), and PIR_Board (one sensor per board), connected by cables.

**Why:** The camera and IR LEDs have to face the target, while the power electronics don't. Separate boards let each part sit where it works mechanically, and keep the 24 V motor switching away from the Pi and camera. A small standalone PIR board can be placed and aimed independently, and up to three can plug into the main board. Each PIR sensor covers about 120°, so three mounted 120° apart give the unit full 360° motion coverage.

## 12. Board-level protection

**Decision:** A TPS26600 eFuse protects the 24 V input. Each motor rail, the solenoid, and the logic I/O have their own protection on top of that. Several of these aren't strictly needed for an indoor expo build, but they were included as good design practice so the board would survive wiring mistakes during bring-up and transfer to a battery-powered product.

### Input stage: why an eFuse replaced the original circuit

**Original design:** a MOSFET reverse-polarity circuit, a board fuse, and an SMBJ24A TVS diode on the 24 V input.

**Problems with it:**
- **The TVS couldn't protect the drivers.** The TMC2209 is rated for 29 V absolute maximum. A TVS that stays off at 24 V doesn't start conducting until about 27 V and is rated to clamp at 38.9 V at full current, well above the drivers' limit. At 24 V, no standard TVS sits between normal operation and 29 V.
- **Inrush current.** Hot-plugging the supply into several hundred µF of bulk capacitance causes a current spike that can blow a small board fuse.
- **A blown fuse has to be replaced.** A fuse protects against shorts only once, and replacing it means desoldering.
- **No battery protection.** Nothing stopped the board from draining a battery pack below a safe voltage.

**The TPS26600 eFuse fixes all four.** It's one IC and six passives, and it replaced the MOSFET circuit and fuse:

| Function | Setting | How |
|---|---|---|
| Overvoltage cutoff | Trips at 26.6 V | R1/R2/R4 divider (931 kΩ / 13 kΩ / 44.2 kΩ) on UVLO/OVP. It disconnects the load instead of clamping, so VM can't reach the drivers' 29 V limit. |
| Undervoltage cutoff | 19.0 V | Same divider. Matches about 3.2 V per cell on a 6S Li-ion pack, so a future battery can't be over-discharged. |
| Current limit | ~2.0 A | R5 = 6.04 kΩ. Above the ~1.5 A worst-case load (solenoid + both motors + 5 V rail) but well below anything that would damage traces. |
| Soft start | ~10 ms ramp | C4 = 47 nF on dV/dT. Charges the bulk capacitors gently, so plug-in inrush stays near 0.5 A. |
| Reverse polarity | Blocks down to −60 V | Internal back-to-back FETs. RTN is kept on its own net, separate from GND, which is required for this to work. |
| Fault response | Auto-retry | The TPS26600 variant restarts on its own after a fault clears, so it doesn't need a reset. |

The input TVS was changed to an **SMBJ33CA bidirectional** part. With the eFuse handling overvoltage, the TVS only has to keep surges under the eFuse's 60 V rating. It's bidirectional so a reversed supply doesn't forward-bias it and short the input. C3 (1 µF, 100 V) is TI's recommended minimum input capacitance.

**Trade-off:** The eFuse blocks reverse current, so motor back-EMF can no longer flow back into the supply. The bulk capacitors and TVS diodes at each driver now have to absorb that energy on their own (see below).

### Motor rails

- **Per-driver fuses (F1, F2, 1 A).** Each TMC2209's VM branch has its own fuse, so a failed driver or shorted motor takes out only that axis instead of the whole board.
- **Per-driver TVS (D2, D3, SMBJ26A)** at each driver's VS pins to catch back-EMF spikes locally.
- **Bulk and decoupling capacitance:** 100 µF electrolytic plus 100 nF ceramics at each driver, placed at the VS pins. They absorb most back-EMF before the TVS has to act.
- **Voltage derating.** Every capacitor on VM is rated 50 V or more, at least 2× the 24 V operating voltage. Ceramic capacitors lose capacitance under DC bias, and electrolytics last longer when run well below their rating.
- **Phase filtering.** 470 pF capacitor arrays (C1, C21) on the motor phase outputs at the connectors damp ringing from the chopper's fast edges and reduce noise radiated through the motor cables.

### Solenoid

- **Flyback diode (D5, SS14 Schottky)** across the solenoid coil. When Q1 turns off, the coil's stored energy circulates through the diode instead of spiking Q1's drain.
- **Fail-safe off.** SOL_CTRL has a 100 kΩ pull-down, so the solenoid can't fire while the Teensy is booting, resetting, or unpowered.

### Logic I/O and fail-safe defaults

- **Drivers disabled by default.** The TMC2209 EN line has a 10 kΩ pull-up (EN is active-low), so the motors stay unpowered until firmware deliberately enables them.
- **Series resistors and pull-downs on inputs.** PIR and control lines pass through a 100 Ω resistor array (RN1) with 100 kΩ pull-downs (RN2). The series resistance limits current into the Teensy pins from cable transients and ESD. The pull-downs keep each line at a defined level if a sensor is unplugged.
- **Isolated PIR supplies.** Each PIR's 3.3 V feed has local 10 µF + 100 nF decoupling and a 33 Ω series resistor. PIR sensors are very sensitive to supply noise, which causes false triggers. The resistor isolates each sensor from the logic rail and the other sensors, and limits current if a PIR cable gets shorted.
- **Single-wire UART resistor.** R13 (1 kΩ) sits between the Teensy TX pin and the shared TMC UART wire. A driver's reply can then override TX on the shared line without two outputs driving against each other.
- **Teensy USB back-feed protection.** The Teensy's VUSB–VIN trace is cut and replaced with an SS14 Schottky (D4) from the 5 V rail. The board can power the Teensy, but plugging in USB for programming won't back-feed the 5 V rail or the computer's USB port.
- **Non-loading activity LEDs.** The UART TX/RX LEDs are driven through Schottky diodes (U6, U7) into a Schmitt-trigger buffer (SN74LVC2G14) with an RC pulse stretcher. The indicators never load or distort the UART signals, and short bursts stay visible.

### Debug and visibility

- **Rail-status LEDs** on +24 V, VM, +5 V, +3.3 V, and the solenoid output show at a glance which rails are up. If an LED is off downstream of the eFuse, the eFuse has tripped.
- **Status, Fault, and MotorReady LEDs** driven by the Teensy.
- **Labeled test points** on every power rail, the motor sense resistors, the solenoid, and the UART lines, so measurements during bring-up don't need probing on component pads.
- **Isolation zone.** A copper-free gap separates the motor-power section from the logic section.

## 13. In-house reflow assembly

**Decision:** The V1 boards are assembled in-house by reflow soldering: solder paste, hand-placed parts, then reflow.

**Alternatives considered:** Hand soldering with an iron; turnkey assembly by the fab house.

**Why reflow instead of an iron:** Most of the board can't be soldered reliably with an iron. The TMC2209s and LMR33630 are QFN packages with exposed thermal pads underneath, the TPS26600 is an HTSSOP with a thermal pad, and most passives are 0402. Reflow solders the hidden thermal pads, which the drivers and regulator need for heat dissipation, and gives consistent joints across hundreds of small pads.

**Why not fab-house assembly:** Mainly cost. Assembly fees and setup charges add a lot at prototype quantities, and with the equipment and skills to reflow in-house, paying a fab house for assembly wasn't necessary.

## 14. Phased demo plan

**Decision:** The first-semester checkpoint shows the major subsystems working on their own: the turret motors driven and aimed with manual click-to-aim, and image recognition running on the Pi camera feed. Second semester adds the water spray and integrates the pieces, so recognition drives the aim automatically.

**Why:** Each subsystem (motors, drivers, position sensing, Pi-to-Teensy communication, camera, and recognition) is proven on its own before they're combined. Integration problems are then easier to isolate. The water system comes last because it depends on accurate aiming and adds the most mechanical risk.
