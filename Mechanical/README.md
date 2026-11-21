# Mechanical

SolidWorks models of the sprayer-unit pan/tilt turret, its sensor mounts, and a test base. Open **`Main.SLDASM`** for the full turret assembly. Keep the folder structure as-is so SolidWorks can find the referenced parts.

| File | Description |
|---|---|
| `Main.SLDASM` | Top-level turret assembly |
| `Pan_Body^Main.SLDPRT` | Pan stage body |
| `Tilt_Body^Main.SLDASM`, `Tilt_Lid^Main.SLDPRT` | Tilt head housing and lid |
| `MotorMount.SLDPRT` | Stepper motor mount |
| `Coupler_Pan.SLDPRT`, `Shaft.SLDPRT` | Pan drive coupler and shaft |
| `Coupler-Step/` | Three-piece jaw coupler (motor hub, pin hub, rubber spider) for the tilt axis |
| `PIR_Head_V2.SLDPRT`, `PIR_Bridge_Neck_V2.SLDPRT` | PIR sensor head and mounting neck |
| `TestBase^Main.SLDASM`, `Test_Base-Top/Bottom.SLDPRT` | Bench test base for bring-up |
| `PI_Board.SLDASM`, `PIR_Board.SLDASM` (+ `*-EDM/`) | PCB models exported from Altium, used for fit checks |
| `324 NEMA-17 Stepper Motor.SLDASM`, `Main Stepper Body.SLDPRT` | Adafruit 324 NEMA 17 stepper model |
| `BS612*.SLDASM`, `can`, `leads`, `lens_cap`, `window` | BS612 PIR sensor model |
| `*_Steel Ball Bearing.SLDPRT`, `*_Dowel Pin.SLDPRT` | Purchased hardware (McMaster-Carr models) |

Printed parts are made on UltiMaker printers.
