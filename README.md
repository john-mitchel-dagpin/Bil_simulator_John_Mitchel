🚗 Bil Simulator – C++ & Threepp

NTNU – OOP C++ Programming
Candidate Number: <>

📌 Project Description

This project is a 3D car simulator built using C++ and Threepp (three.js for C++).
The goal of the project is to demonstrate:

Object-oriented programming (classes: Car, Game, Pickup, Obstacle)

Simple physics (acceleration, braking, drag, steering)

Collision detection

Real-time rendering using Threepp

Event handling (keyboard controls)

Basic game loop structures

The player controls a simple car, collects pickups, and avoids obstacles while the 3D scene updates in real time.

🎮 Features
✔ Movement & Physics

Forward, backward, left, right controls

Bicycle-model steering

Speed, drag, brake force

Smooth steering return to center

Local wheel rotation based on velocity

✔ Pickup System

Pickups placed on the map give bonuses:

Green → Speed boost

Purple → Car size increase

✔ Obstacles

Gray boxes placed around the map.
Colliding with one:

Stops the car

Pushes it back slightly

✔ Rendering

Ground plane

Car body + 4 wheels

Ambient lighting

Third-person chase camera

🎮 Controls
Key	Action
W	Accelerate
S	Reverse / Brake
A	Turn left
D	Turn right
R	Reset car & pickups
ESC	Quit

🛠️ Technologies Used
Component	Library
Rendering	threepp
Geometry & Materials	Threepp Meshes
Windowing & Input	GLFW (via threepp)
Unit Testing	Catch2
Build System	CMake


