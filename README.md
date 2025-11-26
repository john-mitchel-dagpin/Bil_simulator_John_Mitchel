# Bilsimulator (Threepp-prosjekt)

Kandidantnummer: 10027

## 🎯 Prosjektbeskrivelse

Dette prosjektet er en 3D bilsimulator utviklet i C++20 ved bruk av grafikkbiblioteket threepp.
Programmet lar brukeren styre en bil i et 3D-miljø, samle opp objekter, åpne porter, navigere i en verden med bygninger, og til slutt nå en portal som avslutter spillet.

Prosjektet demonstrerer sentrale prinsipper innen objektorientert design, moderne C++, kollisjonsdeteksjon, kontinuerlig spill-løkke, rendering, input-håndtering og god modulær struktur.



## 🛠️ Hovedfunksjonalitet

## 🚗 Bilkontroll

 - Fremover/bakover-bevegelse (W/S)

 - Rotasjon venstre/høyre (A/D)

 - Hjul som roterer basert på fart

 - Forhjul følger rattutslag

 - Smooth lerping av styring og kamera



## 🔑 Objektinteraksjon

 - Pickups som ligger spredt i verden

 - Når bilen treffer en pickup:

 - Den deaktiveres i logikken

 - Den skjules visuelt

 - Den kan åpne porter (avhengig av World.cpp-logikken)



## 🏞️ Miljø & Verden

 - Flatt plan med teksturert steinsti

 - 3D-modeller lastet via OBJLoader:

 - Landsbyhus

 - Slott

 - Smelteverk

 - Fjellmodell

 - Fysiske gjerder laget av bokser

 - Tre store porter (doble dører)

 - Landsbyport (vertikal sliding)

 - Slottport (horisontal sliding)

 - Smelteverkport (vertikal sliding)



## 🌀 Portal

 - Når World.logikken rapporterer at portalen aktiveres:

 - Kamera går til god-view (top-down)

 - Brukeren mister kontroll over bilen

 - Spillet er i avslutningsfase



## ♻️ Reset System (R-tast)

- Tilbakestiller:

 - Bilen

 - Dører

 - Pickups

 - Portal-tilstand

 - Kamera og objektmesh-synlighet

 - Dette kreves i oppgaven og er implementert.



## 📂 Prosjektstruktur
src/
├─ main.cpp

├─ Game.hpp / Game.cpp

├─ World.hpp / World.cpp

├─ Car.hpp / Car.cpp

├─ Pickup.hpp / Pickup.cpp

├─ Obstacle.hpp / Obstacle.cpp

objmodels/

├─ building-village.obj

├─ building-castle.obj

├─ building-smelter.obj

├─ stone-mountain.obj

└─ textures/stonepath.png /cloud_sky.png

tests/

└─ (Catch2 enhetstester)

CMakeLists.txt

README.md


## UML-Diagram

    +--------------------+
    |       Game         |
    +--------------------+
    | - world : World    |
    +--------------------+
    | + update()         |
    | + reset()          |
    +---------+----------+
    |
    | has
    v
    +--------------------+
    |       World        |
    +--------------------+
    | - car : Car        |
    | - objects : vector |
    +--------------------+
    | + update()         |
    | + reset()          |
    | + gate1IsOpen()    |
    | + gate2IsOpen()    |
    | + gate3IsOpen()    |
    +---------+----------+
    |
    | contains
    v
    +--------------+
    |     Car      |
    +--------------+
    | position     |
    | rotation     |
    | speed        |
    +--------------+
    | + update()   |
    +--------------+

          World contains many GameObject
                |
                v
      +--------------------+
      |    GameObject      |
      +--------------------+
      | (abstract class)   |
      +--------------------+
      | + bounds()         |
      | + isActive()       |
      +---------+----------+
                |
      ---------------------
      |                   |
      v                   v
    
    +----------------+   +----------------+
    |    Pickup      |   |    Obstacle    |
    +----------------+   +----------------+
    | + onCollected()|   | (no extra)     |
    +----------------+   +----------------+
    
    
    +--------------------+
    |    InputState      |
    +--------------------+
    | accelerate : bool  |
    | brake : bool       |
    | turnLeft : bool    |
    | turnRight : bool   |
    +--------------------+

## 📘 Forklaring av UML-diagram 
### 1. Game

Game er den øverste spillkontrolleren.
Inneholder:
- Ett World-objekt

Ansvar:
- update() oppdaterer spilltilstanden hver frame
- reset() tilbakestiller spillet når brukeren trykker R

Hvorfor klassen finnes:

- For å skille spilllogikk fra visualisering, og gi en ryddig programstruktur.

### 2. World

World er selve simuleringslaget som styrer alt som finnes i spillverdenen.

Inneholder:
- Car
- En liste med GameObject-pekere
- Noen er Pickup
- Noen er Obstacle
- Tilstandsvariabler for dører (gate1, gate2, gate3)
- Portal-utløser

Ansvar:
- update() → oppdaterer bilfysikk, kollisjoner, pickups
- reset() → nullstiller hele verden 
- gate1IsOpen(), gate2IsOpen(), gate3IsOpen() → brukes av main.cpp for å åpne dører

Hvorfor klassen finnes:
- For å samle all simuleringslogikk på ett sted og holde main.cpp ren.

### 3. Car
En enkel modell av kjøretøybevegelse.

Inneholder:
  -  position
  -  rotation
  -  speed
  -  Ansvar:
  -  Beveger seg frem/bak basert på InputState
  -  Rotasjon 
  -  Enkel friksjon

Hvorfor klassen finnes:

- For å isolere reglene for bevegelse og gjøre koden mer oversiktlig.

### 4. GameObject (abstrakt baseklasse)

En virtuell baseklasse for alle objekter som bilen kan kollidere med eller plukke opp.

Inneholder:

- Virtuelle metoder: bounds(), isActive()

Hvorfor klassen finnes:

- For å kunne behandle alle objekter likt uansett type, via polymorfisme.

### 5. Pickup : GameObject

Et plukkbart objekt.

Inneholder:

- Variabel for aktiv/inaktiv tilstand

Ansvar:
- onCollected() deaktiverer objektet
- World og main.cpp bruker dette for å åpne dører, aktivere portal, skjule mesh-en osv.

Hvorfor klassen finnes:

- For å gi spillinteraksjon og progresjon.


### 6. Obstacle : GameObject

Et hinder som blokkerer bilen.

Inneholder:
- Kun en kollisjonsboks

Ansvar:

- Gir kollisjon uten ekstra logikk

Hvorfor klassen finnes:

- For å skape hindringer i miljøet.


### 7. InputState

Et enkelt struktur-objekt som lagrer spillerens input.

Inneholder:

4 bools:
- accelerate
- brake
- turnLeft
- turnRight

Ansvar:
- KeyHandler skriver inn input
- Car og Game leser det 

Hvorfor klassen finnes:

For å skille input fra simuleringslogikk på en ren måte.


## 🔧 Bygging og kjøring
- CMake 3.14+

- Compiler med støtte for C++20

- Git

- Ingen ekstra avhengigheter — threepp lastes automatisk via FetchContent


## 🧪 Enhetstester (Catch2)

Prosjektet inneholder enhetstester for:

- Bilens fysikk-funksjoner (fart, rotasjon)

- Pickup-aktivitet & deaktivering

- Verden sin logikk (porter åpnes riktig)



## 🧠 Designvalg & Refleksjon 👍 Hva jeg er fornøyd med

- God klasseinndeling: Car, World, Pickup, Obstacle, Game

- Spillogikk og rendering er godt separert

- Enkel å bygge for sensor (FetchContent, ingen kompliserte paths)

- Port-logikken med smooth sliding fungerer godt

- Portal-end-state gir en tydelig slutt

- Reset-systemet fungerer 100%

- God bruk av moderne C++: smartpekere, lambdas, auto, referanser

- Koden er ryddig og oversiktlig


### 👎 Hva kunne vært bedre

- Kollisjonsdeteksjon er enkel (AABB)

- Ingen avansert fysikk (friksjon, momentum osv.)

- UI kunne vært bedre med ImGui

- Portal-slutten kunne hatt bedre visuell feedback (tekst, overlays)

- Flere tester hadde styrket robusthet

- CI/CD workflow kunne vært lagt inn (GitHub Actions)






## 📜 Kilder & Ressurser

- threepp (MIT lisens) – https://github.com/markaren/threepp

- Enkel gratis-modellering/tekstur fra åpne ressursbibliotek

- Obj og mtl som ble brukt https://kenney.nl/assets/hexagon-kit

