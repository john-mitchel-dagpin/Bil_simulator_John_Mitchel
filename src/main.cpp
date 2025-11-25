#include <threepp/threepp.hpp>
#include <threepp/loaders/OBJLoader.hpp>
#include "Game.hpp"
#include "Pickup.hpp"
#include "Obstacle.hpp"
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>

using namespace threepp;

// -----------------------------------------------------
// DOOR STRUCT
// -----------------------------------------------------

struct DoorSet {
    std::shared_ptr<Mesh> left;
    std::shared_ptr<Mesh> right;
    float baseL{};
    float baseR{};
    float openAmount{0.f};
    float doorZ{};
    bool vertical = false;
};

// -----------------------------------------------------
// KEYBOARD HANDLER
// -----------------------------------------------------

class KeyHandler : public KeyListener {
public:
    InputState& input;
    Game& game;
    std::vector<std::shared_ptr<Mesh>>& objectMeshes;
    DoorSet& gate1;
    DoorSet& gate2;
    DoorSet& gate3;
    bool& portalTriggered;
    std::shared_ptr<Mesh> endScreen;


    KeyHandler(InputState& i,
               Game& g,
               std::vector<std::shared_ptr<Mesh>>& meshes,
               DoorSet& g1,
               DoorSet& g2,
               DoorSet& g3,
               bool& portalFlag,
               std::shared_ptr<Mesh> endScreenMesh)

            : input(i),
              game(g),
              objectMeshes(meshes),
              gate1(g1),
              gate2(g2),
              gate3(g3),
              portalTriggered(portalFlag),
              endScreen(endScreenMesh) {}

    void onKeyPressed(KeyEvent evt) override {
        switch (evt.key) {
            case Key::W: input.accelerate = true; break;
            case Key::S: input.brake      = true; break;
            case Key::A: input.turnLeft   = true; break;
            case Key::D: input.turnRight  = true; break;

            case Key::R: {
                // Reset world logic
                game.reset();

                // Reset doors
                gate1.openAmount = gate2.openAmount = gate3.openAmount = 0.f;

                auto resetGate = [&](DoorSet& g) {
                    if (!g.vertical) {
                        // horizontal gate resets on X
                        g.left->position.x  = g.baseL;
                        g.right->position.x = g.baseR;
                    } else {
                        // vertical gate resets on Z
                        g.left->position.z  = g.baseL;
                        g.right->position.z = g.baseR;
                    }
                };
                endScreen->visible = false;


                resetGate(gate1);
                resetGate(gate2);
                resetGate(gate3);


                // Reset portal state
                portalTriggered = false;

                // Reset pickup mesh visibility
                const auto& objs = game.world().objects();
                for (std::size_t i = 0; i < objs.size() && i < objectMeshes.size(); ++i) {
                    auto pickup = dynamic_cast<Pickup*>(objs[i].get());
                    if (pickup && objectMeshes[i]) {
                        objectMeshes[i]->visible = true;
                    }
                }

                break;
            }

            default: break;
        }
    }

    void onKeyReleased(KeyEvent evt) override {
        switch (evt.key) {
            case Key::W: input.accelerate = false; break;
            case Key::S: input.brake      = false; break;
            case Key::A: input.turnLeft   = false; break;
            case Key::D: input.turnRight  = false; break;
            default: break;
        }
    }
};



// -----------------------------------------------------
// MAIN
// -----------------------------------------------------

int main() {

    // --- Window / renderer ---
    Canvas::Parameters params;
    params.title("Bilsimulator - Mountain World")
            .size(1280, 720)
            .resizable(true)
            .antialiasing(4);

    Canvas canvas(params);





    GLRenderer renderer(canvas.size());
    renderer.autoClearColor = true;
    renderer.setClearColor(Color(0xA3C9F9));


    Scene scene;

    // --- Camera (chase cam) ---
    PerspectiveCamera camera(60, canvas.aspect(), 0.1f, 1000.f);
    camera.position.set(0, 15, 20);
    canvas.onWindowResize([&](WindowSize size) {
    camera.aspect = float(size.width()) / float(size.height());
    camera.updateProjectionMatrix();
    renderer.setSize(size);
    });


    float camDistance = 15.f;
    float camHeight   = 8.f;
    float camSmooth   = 0.1f;


    // --- Lighting ---
    auto sun = DirectionalLight::create(0xffffff, 1.5f);
    sun->position.set(50, 100, 20);
    scene.add(sun);

    auto ambient = AmbientLight::create(0xffffff, 0.5f);
    scene.add(ambient);

    // --- Ground plane (400x400) with stone path texture ---
    auto texture = TextureLoader().load("objmodels/textures/stonepath.png");
    texture->wrapS = TextureWrapping::Repeat;
    texture->wrapT = TextureWrapping::Repeat;
    texture->repeat.set(8, 8);   // repeats the pattern

    auto groundMat = MeshPhongMaterial::create({
        {"map", texture}
    });

    auto ground = Mesh::create(
        PlaneGeometry::create(400, 400),
        groundMat
    );

    ground->rotation.x = -math::PI / 2;
    scene.add(ground);




    // --- Car body ---
    auto carMesh = Mesh::create(
            BoxGeometry::create(2, 1, 4),
            MeshPhongMaterial::create({{"color", 0xff0000}})
    );
    carMesh->position.y = 0.5f;
    scene.add(carMesh);

    // =====================================================
    //                 WHEELS (no wobble)
    // =====================================================
    auto tireGeo = CylinderGeometry::create(0.8f, 0.8f, 0.4f, 24);
    tireGeo->rotateZ(math::PI / 2);
    auto rimGeo = CylinderGeometry::create(0.5f, 0.5f, 0.45f, 5);
    rimGeo->rotateZ(math::PI / 2);

    auto tireMat = MeshPhongMaterial::create({{"color", 0x111111}});
    auto rimMat  = MeshPhongMaterial::create({{"color", 0xffffff}});

    auto makeWheelVisual = [&]() {
        auto g = Group::create();
        g->add(Mesh::create(tireGeo, tireMat));
        g->add(Mesh::create(rimGeo, rimMat));
        return g;
    };

    float wheelY = 0.0f;
    float wheelX = 1.1f;
    float wheelZ = 1.6f;

    // Front wheels pivot (steering)
    auto flSteer = Group::create();
    auto frSteer = Group::create();
    auto flWheel = makeWheelVisual();
    auto frWheel = makeWheelVisual();
    flSteer->add(flWheel);
    frSteer->add(frWheel);

    flSteer->position.set(-wheelX, wheelY,  wheelZ);
    frSteer->position.set( wheelX, wheelY,  wheelZ);

    carMesh->add(flSteer);
    carMesh->add(frSteer);

    // Rear wheels
    auto rlWheel = makeWheelVisual();
    auto rrWheel = makeWheelVisual();
    rlWheel->position.set(-wheelX, wheelY, -wheelZ);
    rrWheel->position.set( wheelX, wheelY, -wheelZ);
    carMesh->add(rlWheel);
    carMesh->add(rrWheel);

    float steeringAngle = 0.f;
    const float steeringLerp = 0.25f;

    auto fenceMat = MeshPhongMaterial::create({{"color", 0x553311}});

    auto makeFence = [&](float x, float z, float w, float h) {
        auto mesh = Mesh::create(BoxGeometry::create(w, 2.f, h), fenceMat);
        mesh->position.set(x, 1.f, z);
        return mesh;
    };

    // Village fence
    scene.add(makeFence(-120.f, -158.f, 1.f, 55.f)); // south
    scene.add(makeFence(-120.f, -92.f, 1.f, 55.f));  // north
    scene.add(makeFence(-160.f, -185.f, 80.f, 1.f));  // west
    scene.add(makeFence(-160.f,  -65.f, 80.f, 1.f));  // east

    // Castle fence
    scene.add(makeFence(27.f, 100.f, 45.f, 1.f)); // north fence left
    scene.add(makeFence(-20.f, 100.f, 30.f, 1.f));  // north fence right
    scene.add(makeFence(50.f,150.f,1.f,100.f));    // west
    scene.add(makeFence(-35.f, 150.f,1.f,100.f));    // east

    // Smelter fence

    scene.add(makeFence(110.f, -110.f, 1.f, 9.f)); // north
    scene.add(makeFence(110.f, -160.f, 1.f, 70.f)); // south
    scene.add(makeFence(155.f, -105.f,90.f,1.f));    // west
    scene.add(makeFence(155.f,-195.f,90.f,1.f));      // east



    // =====================================================
    //               DOORS: THREE DOUBLE GATES
    // =====================================================

    auto doorMat = MeshPhongMaterial::create({{"color", 0x8B4513}});

    auto makeDoor = [&](float x, float z, bool vertical) {
        DoorSet d;

        // Each door panel
        float doorWidth  = 5.f;
        float doorHeight = 7.f;
        float doorDepth  = 0.5f;

        d.left  = Mesh::create(BoxGeometry::create(doorWidth, doorHeight, doorDepth), doorMat);
        d.right = Mesh::create(BoxGeometry::create(doorWidth, doorHeight, doorDepth), doorMat);

        // Save gate orientation
        d.vertical = vertical;




        if (!vertical) {
            // --------------------------------------------------
            // HORIZONTAL GATE
            // (castle gate: sliding left/right on X axis)
            // --------------------------------------------------
            d.left->position.set(x - 3.f, 2.f, z);
            d.right->position.set(x + 3.f, 2.f, z);

            d.baseL = d.left->position.x;
            d.baseR = d.right->position.x;
        } else {
            // --------------------------------------------------
            // VERTICAL GATE
            // (village + smelter: sliding forward/back on Z axis)
            // --------------------------------------------------
            d.left->position.set(x, 2.f, z - 3.f);
            d.right->position.set(x, 2.f, z + 3.f);

            d.baseL = d.left->position.z;
            d.baseR = d.right->position.z;

            d.left->rotation.y  = math::PI / 2;
            d.right->rotation.y = math::PI / 2;
        }

        d.doorZ = z;

        scene.add(d.left);
        scene.add(d.right);

        return d;
    };




    // align with World.cpp gates
    DoorSet gate1 = makeDoor(-120.f, -125.f, true);   // village = vertical
    DoorSet gate2 = makeDoor(   0.f,  100.f, false);  // castle = horizontal
    DoorSet gate3 = makeDoor( 110.f, -120.f, true);   // smelter = vertical

    auto portalMat = MeshPhongMaterial::create({
    {"color", 0x00ccff}
    });
    portalMat->emissive = Color(0x00aaff);
    portalMat->transparent = true;
    portalMat->opacity = 0.9f;

    auto portalMesh = Mesh::create(
        PlaneGeometry::create(14.f, 18.f),
        portalMat
    );

    // Make it vertical
    portalMesh->rotation.y = math::PI / 2;
    portalMesh->position.set(-150.f, 0.f, 120.f);
    scene.add(portalMesh);



    // ------------------------------------
    // End screen when hit hidden portal
    // ------------------------------------

    auto endTexture = TextureLoader().load("objmodels/textures/cloud_sky.png");
    auto endMat = MeshBasicMaterial::create({{"map", endTexture}});
    endMat->transparent = true;

    auto endScreen = Mesh::create(
        PlaneGeometry::create(200, 120),   // big enough to fill view
        endMat
    );

    // Position high above the world so it stays out of normal view
    endScreen->position.set(0, 200, 0);

    // Face downward (toward god view camera)
    endScreen->rotation.x = -threepp::math::PI / 2;

    endScreen->visible = false;
    scene.add(endScreen);







    // =====================================================
    //                 GAME LOGIC
    // =====================================================
    Game game;
    InputState input;

    // Visual meshes for pickups
    std::vector<std::shared_ptr<Mesh>> objectMeshes;
    objectMeshes.reserve(game.world().objects().size());

    for (const auto& obj : game.world().objects()) {

        auto pickup   = dynamic_cast<Pickup*>(obj.get());
        auto obstacle = dynamic_cast<Obstacle*>(obj.get());

        if (pickup) {
            auto mesh = Mesh::create(
                    SphereGeometry::create(0.8f, 16, 16),
                    MeshPhongMaterial::create({{"color", 0x00ff00}})
            );
            auto b = pickup->bounds();
            mesh->position.set(
                    (b.minX + b.maxX) * 0.5f,
                    0.8f,
                    (b.minZ + b.maxZ) * 0.5f
            );
            scene.add(mesh);
            objectMeshes.push_back(mesh);

        } else if (obstacle) {
            // no visible mesh for obstacles here (pure colliders)
            objectMeshes.push_back(nullptr);
        } else {
            objectMeshes.push_back(nullptr);
        }
    }

    // =====================================================
    //                 LOAD OBJ MODELS
    // =====================================================
    auto loadModel = [&](const std::string& name) -> std::shared_ptr<Group> {
        std::string objPath = "objmodels/" + name + ".obj";
        OBJLoader loader;
        auto root = loader.load(objPath);
        if (!root) {
            std::cerr << "Failed to load: " << objPath << "\n";
            return nullptr;
        }
        root->scale.set(4.f, 4.f, 4.f);
        return root;
    };

    auto mountainModel = loadModel("stone-mountain");
    auto villageModel  = loadModel("building-village");
    auto castleModel   = loadModel("building-castle");
    auto archeryModel  = loadModel("building-archery");
    auto smelterModel  = loadModel("building-smelter");

    struct BuildingPlacement {
        std::shared_ptr<Group> model;
        Vector3 position;
        Vector3 scale;
        Vector3 rotation;
    };

    std::vector<BuildingPlacement> placements = {
            {villageModel,  {-150.f, -11.f, -150.f}, {60.f, 60.f, 60.f},{0,180,0}},
            {villageModel,  {-150.f, -11.f, -100.f}, {60.f, 60.f, 60.f},{0,180,0}},
            {mountainModel, {-180.f, -14.f,  100.f}, {150.f,150.f,150.f}},
            {castleModel,   {   5.f, -15.f, 150.f},  {80.f, 80.f, 80.f},{0,-90,0}},
            {archeryModel,  { 150.f,  -13.f, 150.f},  {60.f, 60.f, 60.f}},
            {smelterModel,  { 150.f, -15.f,-150.f},  {80.f, 80.f, 80.f}},
    };

    for (auto& bp : placements) {
        if (!bp.model) continue;
        auto obj = bp.model->clone();
        obj->position.copy(bp.position);
        obj->scale.copy(bp.scale);
        obj->rotation.set(
            threepp::math::degToRad(bp.rotation.x),
            threepp::math::degToRad(bp.rotation.y),
            threepp::math::degToRad(bp.rotation.z)

            );
        scene.add(obj);
    }

    // =====================================================
    //                 PORTAL STATE
    // =====================================================
    bool portalTriggered = false;

    // =====================================================
    //                 INPUT HANDLER
    // =====================================================
    KeyHandler handler(input,
                       game,
                       objectMeshes,
                       gate1,
                       gate2,
                       gate3,
                       portalTriggered,
                       endScreen);

    canvas.addKeyListener(handler);

    // =====================================================
    //                 MAIN LOOP
    // =====================================================
    canvas.animate([&]() {

        float dt = 1.f / 60.f;

        auto& world = game.world();

        // game update only if not in portal end-state
        if (!portalTriggered) {
            game.update(dt, input);
        }

        const auto& car = world.car();

        // --- Sync car mesh ---
        carMesh->position.x = car.position().x;
        carMesh->position.z = car.position().z;
        carMesh->rotation.y = car.rotation();

        float s = car.getVisualScale();
        carMesh->scale.set(s, s, s);

        // --- Steering (front wheels) ---
        float targetSteer = 0.f;
        if (input.turnLeft)  targetSteer =  0.6f;
        if (input.turnRight) targetSteer = -0.6f;

        steeringAngle += (targetSteer - steeringAngle) * steeringLerp;
        flSteer->rotation.y = steeringAngle;
        frSteer->rotation.y = steeringAngle;

        // --- Wheel spin ---
        float spin = car.speed() * dt * 7.f;
        flWheel->rotation.x += spin;
        frWheel->rotation.x += spin;
        rlWheel->rotation.x += spin;
        rrWheel->rotation.x += spin;

        //-----------------------------------------------------------
        // GATE DOOR OPENING SYNCHRONIZED WITH WORLD.CPP LOGIC
        //-----------------------------------------------------------
        float openSpeed = dt * 1.5f;   // nice smooth opening
        float openDist  = 6.f;         // how far doors slide apart

        auto syncGate = [&](DoorSet& gate, bool worldGateIsOpen) {

            // Smooth approach: openAmount approaches 1 if open, 0 if closed
            float target = worldGateIsOpen ? 1.f : 0.f;
            gate.openAmount += (target - gate.openAmount) * openSpeed;

            if (!gate.vertical) {
                // Horizontal door (slides on X)
                gate.left->position.x  = gate.baseL - gate.openAmount * openDist;
                gate.right->position.x = gate.baseR + gate.openAmount * openDist;
            } else {
                // Vertical door (slides on Z)
                gate.left->position.z  = gate.baseL - gate.openAmount * openDist;
                gate.right->position.z = gate.baseR + gate.openAmount * openDist;
            }
        };

        // Sync with WORLD state:
        syncGate(gate1, world.gate1IsOpen());
        syncGate(gate2, world.gate2IsOpen());
        syncGate(gate3, world.gate3IsOpen());


        // --- Hide collected pickups ---
        const auto& objs = world.objects();
        for (size_t i = 0; i < objs.size() && i < objectMeshes.size(); ++i) {
            auto pick = dynamic_cast<Pickup*>(objs[i].get());
            if (pick && !pick->isActive() && objectMeshes[i]) {
                objectMeshes[i]->visible = false;
            }
        }

        // --- Portal trigger from world ---
        if (world.portalTriggered()) {
            if (!portalTriggered) {
                portalTriggered = true;
                endScreen->visible = true;

                // Print end message to console (always works)
                std::cout << "The end, thanks for playing (OOP Project)" << std::endl;
            }
        }


        // --- Camera: chase or god view ---
        if (!portalTriggered) {

            float fx = std::sin(car.rotation());
            float fz = std::cos(car.rotation());

            Vector3 desired(
                    car.position().x - fx * camDistance,
                    camHeight,
                    car.position().z - fz * camDistance
            );

            camera.position.lerp(desired, camSmooth);
            camera.lookAt({car.position().x, 5.f, car.position().z});

        } else {
            // God-view
            camera.position.set(0.f, 350.f, 0.f);
            camera.lookAt({0.f, 0.f, 30.f});
        }




        renderer.render(scene, camera);
    });
}