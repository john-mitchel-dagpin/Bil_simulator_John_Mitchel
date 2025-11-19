# 🚗 Bilsimulator – NTNU Ålesund
**Candidate Number:** 

A simple 3D bilsimulator built with **modern C++20** and **threepp**, demonstrating  
object-oriented design, physics, collision detection, pickups, obstacles,  
unit tests, and real-time rendering.

This project is delivered individually as part of the course  
**Automatisering og intelligente systemer **.

---

## 📦 Features

### ✅ **Car physics (bicycle model)**
- Smooth acceleration / braking
- Drag force
- Max speed limits
- Steering with rate-limit
- Realistic turning using the **bicycle model**

### ✅ **Visual car model**
- Car body (box geometry)
- 4 wheels (cylinders)
- Front wheels turn visually
- Wheels spin depending on speed
- Wheels are child objects of the car body

### ✅ **3D Environment**
- Large plane "ground"
- Obstacles
- Rotating camera that follows the car
- Simple lighting

### ✅ **Pickups**
Two pickup types:
- **Speed boost** → increases car max speed
- **Growth boost** → increases car size

Pickups disappear when collected.

### ✅ **Collisions**
- Car ↔ pickups
- Car ↔ obstacles
- Simple pushback response

### ✔️ **Unit Tests**
Uses Catch2:
- Physics movement
- Collision correctness

### ✔️ **Build system**
- CMake
- FetchContent for dependencies
- C++20
- Runs on Windows (tested)

---

## 🕹️ Controls

| Key | Action |
|-----|--------|
| **W** | Accelerate forward |
| **S** | Brake / reverse |
| **A** | Steer left |
| **D** | Steer right |
| **R** | Reset car position |
| **ESC** | Close the window |

---

## 📁 File Structure

Bil_simulator_John_Mitchel/ 

├── src/ 

    │ ├── Car.hpp

    │ ├── Car.cpp

    │ ├── Game.hpp

    │ ├── Game.cpp

    │ ├── physics.hpp

    │ └── main.cpp


├── tests/

    │ └── test_physics.cpp

├── docs/

    │ └── UML_Class_Diagram.pdf


├── CMakeLists.txt

├── README.md

└── .gitignore


---

## 🧠 Design Principles 

### ✔ Abstraction
Each subsystem (Car physics, Game logic, Rendering) is separated into its own class.

### ✔ Encapsulation
Internal state (e.g., speed, steering angle) is private to `Car` and updated through methods.

### ✔ Cohesion
Each class has **one clear responsibility**:
- `Car` → physics & movement
- `Game` → world, rendering, input & high-level logic
- `main.cpp` → connects Game with threepp

### ✔ Low Coupling
`Car` has **no dependency** on threepp.  
`Game` uses `Car` purely through its interface.

### ✔ Responsibility-Driven Design
The car updates its own physics;  
Game handles objects, pickups, collisions and rendering.

---

## 🔧 Building the Project

### Requirements
- CMake 3.15+
- C++20 compiler (MSVC, Clang or GCC)
- Git
- Internet connection (FetchContent clones libraries)

### Build steps (CLion)
1. Open the project folder in CLion
2. CLion will automatically run CMake
3. Select target: **bilsim**
4. Run ▶️

---

## 🧪 Running Unit Tests

Build and run target:

    test_physics

Includes tests for:
- forward movement
- collision logic

---

## 🧭 UML Diagram

The UML diagram is stored in:
    docs/UML_Class_Diagram.pdf


It contains:
- Car class
- Game class
- Pickup struct
- Obstacle struct

---

## 📝 Reflection

### What I am satisfied with
- Learned how to use **threepp**, CMake, Git, and unit tests in one project
- Implemented realistic physics (bicycle model)
- Clean structure separating logic and rendering
- All mandatory features implemented + several extras

### What could be improved
- More realistic collision handling (bounding boxes, sweep tests)
- Better visual assets (3D models instead of primitives)
- More advanced UI (speedometer, minimap)

### What I learned
- Organizing a C++ project with many files
- Understanding transformation hierarchies (mesh parenting)
- GitHub workflow
- Debugging compiler & linker errors
- Real-time rendering with event loops

---

## 🔗 GitHub Repository

https://github.com/john-mitchel-dagpin/Bil_simulator_John_Mitchel

---

## 📦 License

Free for educational use.




