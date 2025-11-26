# Bilsimulator (Threepp-prosjekt)

Kandidantnummer: 10027

## 🎯 Prosjektbeskrivelse

Dette prosjektet er en 3D bilsimulator utviklet i C++20 ved bruk av grafikkbiblioteket threepp.
Programmet lar brukeren styre en bil i et 3D-miljø, samle opp objekter, åpne porter, navigere i en verden med bygninger, og til slutt nå og finne den skjulte portalen som avslutter spillet.

Prosjektet demonstrerer sentrale prinsipper innen objektorientert design, moderne C++, kollisjonsdeteksjon, kontinuerlig spill-løkke, rendering, input-håndtering og god modulær struktur.

### 📘 Hvordan bruke programmet

Programmet er en enkel bilsimulator der du styrer en bil i et 3D-miljø.
Målet er å utforske verden, plukke opp objekter, åpne porter og til slutt nå portalen for å avslutte spillet.

### 🎮 Taster og kontroller
    Tast	Funksjon
    W	Kjør fremover
    S	Kjør bakover / brems
    A	Sving venstre
    D	Sving høyre
    R	Reset hele spillet (tilbakestill verden)
    ESC	Avslutt (vanlig vinduslukking)

### 🚗 Bilkontroll

- Bilen akselererer fremover når du holder inne W.
- Bilen bremser når du trykker S.
- A og D roterer bilen rundt sin egen akse.
- Forhjulene svinger uavhengig, og hjulene spinner basert på farten.

### 🔑 Pickups og porter

I verden ligger det ulike pickups som kan samles ved å kjøre på dem.
Når begge pickups tilhørende en port er samlet inn, åpnes porten automatisk.
Portene åpner seg visuelt i main.cpp (glir fra hverandre) når logikken i World registrerer at begge pickups er inaktive.

### 🌀 Portal og avslutning

Når du kjører inn i portalen:
Kameraet skifter automatisk til god-view (oversiktskamera).
Bilen slutter å motta input.
Spillet går inn i avslutningsfasen.

### ♻️ Reset (R-tasten)

Når du trykker R, tilbakestilles hele verden: Bilen flyttes tilbake til startposisjon. Fart, rotasjon og størrelse blir nullstilt. Alle pickups blir aktive igjen. Alle porter lukkes.
Portalen deaktiveres. Alle mesh-objekter i main.cpp synkroniseres med logikken i World.
Dette kreves eksplisitt i prosjektoppgaven og er fullstendig implementert.

### 🖼️ 3D-modeller og miljø

Spillet inkluderer flere ferdigmodellerte obj-modeller:
- Landsby
- Smelteverk
- Slott
- Fjell
- Gjerder
- Portaler og dører
- Teksturer lastes fra objmodels/textures/.


### 🏞️ Miljø & Verden

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

Prosjektet inneholder et sett med enhetstester implementert med Catch2 for å verifisere sentral spilllogikk. 
Dette gjør prosjektet mer robust, og viser at logikken er testet uavhengig av grafikkmotoren. 
Testene kjøres via CTest og ligger i tests/-mappen.

### ✔ Testet funksjonalitet

### 1. Bilens fysikk (test_car.cpp)

Tester at:
- Bilen akselererer når accelerate = true
- Friksjon reduserer farten når spilleren slipper gassen
- Bremsing fungerer (farten reduseres raskere)
- Rotasjon og posisjon oppdateres som forventet

Hvorfor er dette viktig?
Car er kjernen av spillmekanikken. Hvis fysikken fungerer feil, fungerer hele spillet dårlig. Testene sikrer stabil og forutsigbar oppførsel.

### 2. Pickup-objekter (test_pickup.cpp)

Tester at:
- En pickup deaktiveres etter at bilen kjører på den
- Speed-boost eller size-change påvirker bilen riktig
- Pickups rapporterer riktig aktiv/inaktiv tilstand

Hvorfor er dette viktig?
Pickups er en sentral del av progresjonen (porter og gameplay). Testene garanterer at spillet ikke låser seg pga. feil logikk.

### 3. GameObject-grunnklasse (test_gameobject.cpp)

Tester at:
- Kollisjonsbokser (AABB) fungerer som forventet
- Aktiv/deaktivert logikk oppfører seg riktig

Hvorfor er dette viktig?
GameObject er baseklassen til alt annet i verden. Hvis den er stabil, er resten mer pålitelig.

### 4. World-logikk (test_world.cpp)

Tester at:
- Verden oppdaterer alle objekter og bilen
- Pickups åpner porter riktig (når begge er samlet)
- Gate-status rapporteres korrekt
- Portal utløses ved kollisjon

Hvorfor er dette viktig?
World binder alt sammen. Testene verifiserer spillflyten uten å måtte starte hele hovedprogrammet.

### 5. Kollisjonsdeteksjon (test_collision.cpp)

Tester at:
- Bilen stopper når den kjører inn i et hindrer (Obstacle)
- Intersect-funksjonen fungerer som forventet

Hvorfor er dette viktig?
Kollisjon er kritisk for alle miljøobjekter. Feil her kan gjøre spillet uspillbart.

### 6. Game-kontrolleren (test_game.cpp)

Tester at:
- Game.update() faktisk delegere til World.update()
- Game.reset() nullstiller verden korrekt

Hvorfor er dette viktig?
Game er toppnivåklassen som styrer hele simulasjonen


## 🧠 Designvalg & Refleksjon 👍 Hva jeg er fornøyd med

- God klasseinndeling: Car, World, GameObject, Pickup, Obstacle, Game bidrar til lav kobling og høy kohesjon.

- Separasjon av ansvar: Rendering og spilllogikk holdes helt adskilt. World/Car styrer logikk; main.cpp styrer kun grafikk og inputs.

- Bruk av moderne C++: smartpekere (unique_ptr), lambdaer, auto, referanser og RAII.

- Reset-systemet fungerer godt: hele spillet kan tilbakestilles uten restart.

- Pickups og porter: Logikken for progresjon (samle 2 pickups → åpner en port) fungerer ryddig og er lett å utvide.

- Enhetstestene: De gir trygghet for korrekt logikk og viser god forståelse av testbar arkitektur.

- Koden er enkel å bygge for sensor: FetchContent laster inn alt automatisk.

## 👎 Hva kunne vært bedre

- Fysikken er enkel: Ingen momentum, akselerasjonskurver eller avansert styring.

- Kollisjonsdeteksjon: AABB er enkelt og fungerer, men ikke optimalt for rotasjon eller komplekse modeller.

- Ingen UI (ImGui): En liten UI-overlay (HUD) kunne gjort spillet mer brukervennlig.

- Portal-sluttscene: Kunne hatt bedre visuelle effekter eller animert tekst.

- CI/CD (GitHub Actions): Kunne gjort at tester kjørte automatisk ved hver commit.

- Flere tester: Selv om dekningen er god, kunne for eksempel dør-animasjon og boost-timer vært ytterligere testet.

- Bevegelsen er kun i x og z planet. Kunne utviklet det slik at bilen kunne bevege 3 dimensjonalt. For eksempel kjøre opp et fjell eller en rampe, falle av fra en klippe, ha tyngdekraften i spillet. 

## 📜 Kilder & Ressurser

- threepp (MIT lisens) – https://github.com/markaren/threepp

- Enkel gratis-modellering/tekstur fra åpne ressursbibliotek

- Obj og mtl som ble brukt https://kenney.nl/assets/hexagon-kit

