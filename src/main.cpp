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

    KeyHandler(InputState& i,
               Game& g,
               std::vector<std::shared_ptr<Mesh>>& meshes,
               DoorSet& g1,
               DoorSet& g2,
               DoorSet& g3,
               bool& portalFlag)
            : input(i),
              game(g),
              objectMeshes(meshes),
              gate1(g1),
              gate2(g2),
              gate3(g3),
              portalTriggered(portalFlag) {}

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

                gate1.left->position.x  = gate1.baseL;
                gate1.right->position.x = gate1.baseR;
                gate2.left->position.x  = gate2.baseL;
                gate2.right->position.x = gate2.baseR;
                gate3.left->position.x  = gate3.baseL;
                gate3.right->position.x = gate3.baseR;

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
    renderer.setClearColor(Color(0x87CEEB)); // sky blue

    Scene scene;

    // --- Camera (chase cam) ---
    PerspectiveCamera camera(60, canvas.aspect(), 0.1f, 1000.f);
    camera.position.set(0, 15, 20);

    float camDistance = 15.f;
    float camHeight   = 8.f;
    float camSmooth   = 0.1f;

    canvas.onWindowResize([&](WindowSize size) {
        camera.aspect = float(size.width()) / float(size.height());
        camera.updateProjectionMatrix();
        renderer.setSize(size);
    });

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

    // =====================================================
    //               DOORS: THREE DOUBLE GATES
    // =====================================================

    auto doorMat = MeshPhongMaterial::create({{"color", 0x8B4513}});

    auto makeDoor = [&](float x, float z, bool vertical) {
        DoorSet d;

        d.left  = Mesh::create(BoxGeometry::create(5.f, 7.f, 0.5f), doorMat);
        d.right = Mesh::create(BoxGeometry::create(5.f, 7.f, 0.5f), doorMat);

        if (!vertical) {
            // horizontal gate (doors slide apart on X-axis)
            d.left->position.set(x - 3.f, 2.f, z);
            d.right->position.set(x + 3.f, 2.f, z);
            d.baseL = d.left->position.x;
            d.baseR = d.right->position.x;
        } else {
            // vertical gate (doors slide apart on Z-axis)
            d.left->position.set(x, 2.f, z - 3.f);
            d.right->position.set(x, 2.f, z + 3.f);
            d.baseL = d.left->position.z;
            d.baseR = d.right->position.z;
        }

        d.vertical = vertical;   // ADD THIS FIELD TO DoorSet
        d.doorZ = z;

        scene.add(d.left);
        scene.add(d.right);

        return d;
    };



    // align with World.cpp gates
    DoorSet gate1 = makeDoor(-120.f, -125.f, false); // left-right
    DoorSet gate2 = makeDoor(   0.f,  100.f, true);  // vertical front-back
    DoorSet gate3 = makeDoor( 110.f, -120.f, false); // left-right



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
    };

    std::vector<BuildingPlacement> placements = {
            {villageModel,  {-150.f, -10.f, -150.f}, {60.f, 60.f, 60.f}},
            {villageModel,  {-150.f, -10.f, -100.f}, {60.f, 60.f, 60.f}},
            {mountainModel, {-200.f, -12.f,  100.f}, {150.f,150.f,150.f}},
            {castleModel,   {   5.f, -12.f, 150.f},  {80.f, 80.f, 80.f}},
            {archeryModel,  { 150.f,  -5.f, 150.f},  {60.f, 60.f, 60.f}},
            {smelterModel,  { 150.f, -15.f,-150.f},  {80.f, 80.f, 80.f}},
    };

    for (auto& bp : placements) {
        if (!bp.model) continue;
        auto obj = bp.model->clone();
        obj->position.copy(bp.position);
        obj->scale.copy(bp.scale);
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
                       portalTriggered);

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

        // --- Door opening logic based on world gates ---
        float openDist = 4.f;

        if (!gate1.vertical) {
            gate1.left->position.x  = gate1.baseL - gate1.openAmount * openDist;
            gate1.right->position.x = gate1.baseR + gate1.openAmount * openDist;
        } else {
            gate1.left->position.z  = gate1.baseL - gate1.openAmount * openDist;
            gate1.right->position.z = gate1.baseR + gate1.openAmount * openDist;
        }

        if (!gate2.vertical) {
            gate2.left->position.x  = gate2.baseL - gate2.openAmount * openDist;
            gate2.right->position.x = gate2.baseR + gate2.openAmount * openDist;
        } else {
            gate2.left->position.z  = gate2.baseL - gate2.openAmount * openDist;
            gate2.right->position.z = gate2.baseR + gate2.openAmount * openDist;
        }

        if (!gate3.vertical) {
            gate3.left->position.x  = gate3.baseL - gate3.openAmount * openDist;
            gate3.right->position.x = gate3.baseR + gate3.openAmount * openDist;
        } else {
            gate3.left->position.z  = gate3.baseL - gate3.openAmount * openDist;
            gate3.right->position.z = gate3.baseR + gate3.openAmount * openDist;
        }


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
            portalTriggered = true;
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
            camera.position.set(0.f, 180.f, 0.f);
            camera.lookAt({0.f, 0.f, 0.f});
            // You could also draw a UI text texture later here if you want.
            // For now, it's just a nice top view.
        }

        renderer.render(scene, camera);
    });
}
