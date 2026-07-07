# ❄️ Snow Burrowing Bot

**A modular, snake-like, screw-driven robot for subsurface locomotion and avalanche search & rescue**

> Developed at the **Indian Institute of Technology, Mandi** — School of Mechanical & Materials Engineering, School of Biosciences and Bioengineering, School of Computing and Electrical Engineering, and School of Civil and Environmental Engineering.

<p align="center">
  <img src="docs/images/snow_burrowing_bot.jpg" alt="Snow Burrowing Bot prototype" width="600"/>
  <br/>
  <em>Prototype of the Snow Burrowing Bot</em>
</p>

---

## 📖 Overview

Avalanche victims often become trapped beneath compacted snow, where locating and reaching them with existing rescue methods (dogs, probes, transceivers) is slow and difficult. Inspired by **NASA JPL's EELS (Exobiology Extant Life Surveyor)**, the **Snow Burrowing Bot** is a proof-of-concept, snake-like robot made of articulated cylindrical segments with rotating helical outer shells. Screw-driven propulsion combined with a front-mounted auger allows the robot to burrow into compacted snow, while servo-actuated joints enable basic steering along shallow subsurface paths.

This prototype focuses on demonstrating forward locomotion, initial burrowing, and reliable segment articulation in compacted granular media — an early step toward a deployable robotic platform for avalanche search and rescue.

| | |
|---|---|
| **Hardware type** | Modular snake-like robotic system for subsurface locomotion & excavation |
| **Subject area** | Engineering and material science |
| **Closest commercial analog** | None — closest related equipment: surface snow blowers, avalanche probe/transceiver systems (detection only, no burrowing) |
| **Open source license** | MIT (firmware) / CERN-OHL-S (hardware/design files) |

---

## 🧩 Table of Contents

- [Overview](#-overview)
- [Motivation](#-motivation)
- [System Architecture](#-system-architecture)
- [CAD Models](#-cad-models)
- [Mechanical Analysis (FEA)](#-mechanical-analysis-fea)
- [Build Instructions](#-build-instructions)
- [Operation Instructions](#-operation-instructions)
- [Validation & Results](#-validation--results)
- [Capabilities & Limitations](#-capabilities--limitations)
- [Future Work](#-future-work)
- [Repository Structure](#-repository-structure)
- [Authors & Acknowledgements](#-authors--acknowledgements)
- [License](#-license)
- [References](#-references)

---

## 🎯 Motivation

Avalanche accidents are among the most common hazards faced by skiers and mountaineers. Once snow compacts after a slide, two problems dominate rescue efforts:

1. **Finding** people buried deep in the snow.
2. **Moving safely** across unstable, compacted terrain to reach them.

Existing surface robots and soil-digging robots are not suited to snow, which behaves differently under compaction. The Snow Burrowing Bot addresses this gap with a purpose-built subsurface mechanism inspired by snake-robot and auger-drilling architectures.

**Target functional scope:**

- 🌡️ **Victim localization** via environmental sensors (temperature, CO₂, acoustic/vibrational signals)
- ⛏️ **Controlled excavation** that maintains a safe buffer distance from the detected victim
- 💓 **Physiological monitoring** (SpO₂, pulse rate) via biometric sensing
- 🫁 **Oxygen delivery inlet** to sustain victim respiration during excavation

---

## 🏗️ System Architecture

The robot is a **4-segment articulated snake robot**. Each segment consists of:

- **Inner hollow-cylindrical backbone** (PVC pipe) — houses the DC drive motor, gearbox, and electronics
- **Outer rotating helical shell** — a concentric cylinder with an embedded helical fin that produces screw-drive propulsion when spun
- **Two servo motors per joint** — control inter-segment bending for steering

**Propulsion:** A coaxially-mounted DC geared motor drives a gearbox connected to the outer helical shell, spinning it to propel the segment forward like a screw. A **front-mounted auger drill** loosens compacted snow ahead of the lead segment, pulling the body forward into the burrowed tunnel.

**Electronics:** Each segment carries a microcontroller (ESP32) for local motor control. Motors are driven via Cytron MDD3A dual-channel drivers; servos via a PCA9685 16-channel PWM driver. Power is distributed from a central battery pack through DC-DC buck converters, split between high-current loads (drive motors, auger) and low-voltage systems (servos, sensors, controllers).

**Control:** Firmware is written in Arduino C/C++. The robot supports manual wireless operation today, with a semi-autonomous mode (preset movement sequences) planned for future versions. Live telemetry (motor speed, current draw via ACS712 sensors, battery voltage) is streamed to a base station.

**Safety:** Inline fuses, voltage regulators, and overcurrent protection guard against stalls and surges. A wireless emergency-stop signal can halt the system instantly. The modular segment design allows quick field repair without full disassembly.

---

## 🛠️ CAD Models

The full CAD model is maintained on **Onshape**:

🔗 **[Snow Burrowing Bot — Onshape CAD Model](https://cad.onshape.com/documents/0c7534588280f36afd0d21d6/w/f7cbfca13859299204f496aa/e/a737bc384b3fefcb3a17f5e3?renderMode=0&uiState=6a3d79e3d2becb310d0a8cdc)**

All design files are released under the **CERN-OHL-S** open hardware license.

---

## 🔬 Mechanical Analysis (FEA)

### Finite Element Analysis of the 3D-Printed Spur–Ring Gear System

Performed in **ANSYS Workbench 2025 R1** under static loading to assess structural behavior, contact stresses, and material adequacy of the 3D-printed (PLA+) gear pair.

**Geometry & Material**

| Spur Gear | Ring Gear |
|---|---|
| Teeth: 26 | Teeth: 43 |
| Module: 1.25 | Module: 1.25 |
| Pressure angle: 20° | Pressure angle: 20° |
| Shaft diameter: 6 mm | Material: PLA+ (assumed isotropic) |
| | Elastic modulus: 3.5 GPa |
| | Poisson's ratio: 0.33 |
| | Yield strength (approx.): 55 MPa |

**Meshing:** Patch-conforming tetrahedral mesh, program-controlled advanced options, with local contact sizing of 5×10⁻⁴ m at the gear mesh interface.

**Boundary Conditions:** Frictional contact (μ = 0.3, "Adjust to Touch") between spur and ring gear; cylindrical supports restricting radial/axial motion while allowing rotation; driving torque of **2 N·m** applied to the spur gear.

**Results:**
- Maximum Principal Stress: **44 MPa**, located at the root of the spur gear teeth
- Equivalent Elastic Strain and Equivalent (von-Mises) Stress evaluated across the assembly
- Minimum Factor of Safety > 1, confirming the PLA gear will not fail or permanently deform under the applied load

**Conclusion:** The simplified isotropic-PLA model captures the essential load-carrying behavior and critical stress regions. Future refinement (mesh convergence studies, anisotropic material modeling, experimental validation) will improve model accuracy.

---

## 🔧 Build Instructions

The build is organized into three stages: **mechanical construction**, **electronic integration**, and **system testing**.

### 1. Mechanical Construction

- Each body segment = inner cylindrical backbone (ID = 45 mm, wall = 3–5 mm, length = 250 mm) + outer rotating helical shell (OD = 110 mm).
- Inner pipe houses the DC drive motor (High Torque Precision Encoder DC Geared Motor) and gearbox, plus internal wiring.
- Outer shell is 3D-printed (PLA filament) and coupled to the gearbox output for screw-like propulsion.
- Four segments are interconnected via servo-driven joints (16 kgcm dual-shaft servos) mounted on aluminium brackets fixed to the inner pipe.
- A detachable auger drill head, driven by an independent DC geared motor, is mounted on the front segment for initial snow penetration.

### 2. Electronic Integration

- Each drive motor → Cytron MDD3A dual-channel driver → ESP32 central microcontroller.
- Servos → PCA9685 16-channel PWM driver.
- Power → central battery pack → 200 W / 4015 DC-DC buck converters for stable voltage regulation.
- Optional sensors (GY-91 IMU or encoders) can be added in future revisions.
- All electronics are housed inside the PVC inner pipe for protection from cold and snow exposure.

### 3. Software and Control

- Firmware written in **Arduino C++**, coordinating motor/servo timing and direction per segment.
- Proportional control manages drive-motor speed and servo position.
- Safety limits guard against motor overload (torque/current checks).
- Live telemetry (motor speed, battery level via ACS712 sensors) streams to the control interface.
- Supports manual and semi-autonomous test modes.

### 4. Assembly Verification

- Test each module individually: motor direction, servo range, encoder feedback.
- After full assembly, verify joint motion, auger rotation, and wireless control stability.
- Validate burrowing capability in an indoor snow-box before field trials.
- Power all tests from the 12 V / 50 A industrial supply.

### ⚠️ Safety Precautions

- Wear insulated gloves and eye protection during mechanical testing.
- Check all power connections before applying voltage.
- Keep an emergency stop available during all burrowing experiments.
- Conduct field tests only under safe, supervised snow conditions.

---

## ▶️ Operation Instructions

1. **Pre-deployment checks** — mechanical alignment, sealing, battery charge, and secure connections for motors/servos/control modules.
2. **Indoor validation** — test propulsion, joint motion, and auger performance in a snow box or small snow area before outdoor use.
3. **Control modes:**
   - **Manual** — operator uses a wireless controller for auger, motor speed, and joint movement.
   - **Semi-autonomous** — preset programs control segment rotation, servo movement, and speed; segments alternate rotation direction to balance torque and maintain stability.
4. **Live telemetry** — motor current (ACS712), voltage, and control status are streamed to the operator throughout.
5. **Startup procedure** — begin at low speed; the auger loosens snow ahead while screw-driven segments push the bot forward. On excess torque/current draw, the system auto-stops and briefly reverses the auger to clear snow and prevent stalling.
6. **Post-test** — retrieve the bot, power down, inspect for snow/mechanical damage, and log data (burrowing depth, power consumption, performance vs. snow density).
7. **Field safety** — always test in controlled areas, avoiding avalanche-prone zones, following standard cold-weather robotics safety procedures.

---

## 📊 Validation & Results

Testing was performed in a controlled indoor environment using compacted artificial snow-like material, since it offers repeatable conditions for early-stage subsurface robot validation.

| Metric | Result |
|---|---|
| Gear ratio | 28:45 |
| Rotation speed | ~37 RPM |
| Forward displacement | 6–8 cm/s (density-dependent) |
| Max stress in gear (FEA) | 44 MPa (root of spur gear teeth) |
| Minimum Factor of Safety | > 1 |

- **Locomotion:** Screw-driven shells produced consistent forward motion on moderately compacted material; occasional slipping occurred in softer, loosely packed snow, consistent with expected screw-propulsion behavior in granular media.
- **Burrowing:** The auger successfully loosened compacted material for shallow penetration; deeper penetration was limited by motor torque and increased resistive force in denser material.
- **Steering:** Servo joints allowed directional control, though turning radius remained large due to segment length and snow resistance.
- **Electrical/Structural:** Communication and control remained stable; current spikes occurred during high-load digging. PVC/PLA structure held integrity overall, with minor deformation at segment joints under high torque.

---

## ✅ Capabilities & Limitations

**Capabilities**
- Screw-driven propulsion enables forward locomotion in compacted granular material
- Auger mechanism effectively loosens packed material for shallow subsurface progress
- Servo-controlled joints allow steering (with limited turning sharpness)

**Limitations**
- Current draw increases significantly during high-load excavation
- Minor structural deformation at high-torque segment interfaces
- Locomotion performance depends heavily on snow density/compaction
- Burrowing depth limited by available torque and motor power
- Battery consumption rises notably during continuous drilling
- Wireless communication weakens as the robot goes deeper
- Motor efficiency drops at low temperatures
- Auger performance decreases in icy or extremely dense snow

---

## 🚀 Future Work

- Increase torque output and integrate sensor feedback for semi-autonomous operation
- Add dedicated sensor modules for thermal/gas detection and victim vitals (SpO₂, pulse)
- Add oxygen delivery inlet for sustained victim respiration during excavation
- Develop interchangeable auger attachments for different snow densities
- Optimize auger geometry for deeper, more efficient penetration
- Scale the modular design to more segments for deeper burrowing applications
- Improve high-torque motor drivers and current handling for continuous digging

**Potential uses for other researchers:**
- Modular snake-like platform for subsurface locomotion in snow, sand, soil, or granular media research
- Testbed for screw-drive propulsion, auger mechanics, and multi-segment steering studies
- Open hardware platform for integrating novel sensor modules for subterranean exploration
- Low-cost robotic architecture for education, prototyping, and field robotics
- Expandable toward rescue robotics, environmental sampling, and planetary analog research

---

## 📂 Repository Structure

```
snow-burrowing-bot/
├── firmware/                # Arduino C/C++ control code
├── docs/
│   ├── Snow_Burrowing_Bot.pdf
│   └── images/
│       └── snow_burrowing_bot.jpg
├── LICENSE                  # MIT (firmware)
├── LICENSE-HARDWARE         # CERN-OHL-S (hardware/design files)
└── README.md
```

> 📌 CAD models are maintained on **[Onshape](https://cad.onshape.com/documents/0c7534588280f36afd0d21d6/w/f7cbfca13859299204f496aa/e/a737bc384b3fefcb3a17f5e3?renderMode=0&uiState=6a3d79e3d2becb310d0a8cdc)** rather than versioned in this repository.

---

## 👥 Authors & Acknowledgements

**Authors:** Dharmesh Sahu, Kamlesh Kumar, Baivab Momin, Somanshu Juneja, Pandyam Siri Chandana, Saransh Goyal — under the guidance of **Dr. Gaurav Bhutani**.

All authors contributed equally across conceptualization, methodology, investigation, software, validation, mechanical analysis, resources, data curation, visualization, and manuscript writing/review.

**Acknowledgements:** Dr. Gaurav Bhutani (SMME, faculty mentor), Mr. Amit Kumar (TA), Dr. Jagadeesh Kardiyam (CAIR), Mr. Rakesh Kumar (Junior Technical Superintendent), and Mr. Raj Kumar (Technician) for their support in manufacturing and workshop processes.

### Ethics Statement
This work involves the design and implementation of a robotic system for a socially positive humanitarian purpose — search and rescue — intended to support human rescuers and reduce risk to life in hazardous environments. All hardware design, testing, and experimental procedures complied with IIT Mandi safety protocols.

---

## 📄 License

- **Firmware:** [MIT License](LICENSE)
- **Hardware / CAD / design files:** [CERN-OHL-S v2](https://ohwr.org/cern_ohl_s_v2.txt)

---

## 📚 References

1. ICDO. "Avalanches." International Civil Defence Organisation, 2019. [Link](https://icdo.org/about-icdo/disasters/avalanches.html)
2. National Geographic. "Avalanche." 2020. [Link](https://education.nationalgeographic.org/resource/avalanche/)
3. NASA JPL. "Exobiology Extant Life Surveyor (EELS)." 2023. [Link](https://www-robotics.jpl.nasa.gov/how-we-do-it/systems/exobiology-extant-life-surveyor-eels/)
4. A. Hunt et al. "Snake-like robot for extreme terrain exploration." *Science Robotics*, 2024. [DOI](https://www.science.org/doi/10.1126/scirobotics.adh8332)
5. S. Smith et al. "Sensing through snow for search and rescue robotics." *IEEE Transactions on Robotics*, 2018. [Link](https://ieeexplore.ieee.org/document/8460916)
6. K. Brugger et al. "Avalanche survival and rescue." *Wilderness & Environmental Medicine*, 2018. [Link](https://pmc.ncbi.nlm.nih.gov/articles/PMC5877370/)
7. T. Goldman et al. "Bioinspired snake robots for subterranean mobility." arXiv:2306.01748, 2023. [Link](https://arxiv.org/abs/2306.01748)
8. Safeback. "Avalanche Safety: Rescue Equipment and Techniques." 2023. [Link](https://www.safeback.no/articles/avalanche-safety/rescue-equipment-and-techniques)
9. J. Jiang, X. Chen, Y. Zhou. "A framework for modular and distributable control of reconfigurable robotic systems." *IEEE ICRA*, 2016. DOI:10.1109/ICRA.2016.7474862
10. NASA JPL. "EELS Bot Demonstration Video." [Link](https://youtu.be/ifCIDT4X9AM)
11. NASA JPL Robotics. "EELS Project Overview." [Link](https://www.jpl.nasa.gov/robotics-at-jpl/eels/)
12. Frontiers in Robotics and AI. "Burrowing Robots and Subterranean Locomotion." 2025. [Link](https://www.frontiersin.org/journals/robotics-and-ai/articles/10.3389/frobt.2025.1525186/full)
13. MIT Digger Finger Project. [Link](https://sites.google.com/view/diggerfinger)
14. ScienceDaily. "Robots Learn to Move in Granular Media." 2021. [Link](https://www.sciencedaily.com/releases/2021/06/210616154236.htm)
15. Universe Today. "Burrowing Mole-Bot for Subsurface Planetary Exploration." [Link](https://www.universetoday.com/articles/burrowing-mole-bot-could-characterize-other-planets-soil)
16. ResearchGate. "Bio-inspired Dual-auger Self-burrowing Robots in Granular Media." 2023. [Link](https://www.researchgate.net/publication/370864796_Bio-inspired_Dual-auger_Selfburrowing_Robots_in_Granular_Media)
17. Sandia National Labs. "Avalanche Rescue Modeling." 2000. [Link](https://www.sandia.gov/labnews/2000/01/28/avalanchestory)
18. Politecnico di Torino. "Avalanche Causes and Rescue Systems – Thesis." [Link](https://webthesis.biblio.polito.it/18675)
19. MIR Labs Journal. "Bio-inspired Robotics Review." [Link](https://www.mirlabs.net/jnic/secured/Volume4-Issue1/Paper2.pdf)
20. US Patent US20070157925A1. "Burrowing Robotic Mechanisms." [Link](https://patents.google.com/patent/US20070157925A1/en)

---

<p align="center"><i>Built with ❄️ at IIT Mandi — bringing subsurface robotics closer to real avalanche rescue.</i></p>
