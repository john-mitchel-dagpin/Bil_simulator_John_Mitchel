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
// KEYBOARD HANDLER
// -----------------------------------------------------

class KeyHandler : public KeyListener {
public:
    InputState& input;
    Game& game;
    std::vector<std::shared_ptr<Mesh>>& objectMeshes;
    std::shared_ptr<Mesh>& doorLeft;
    std::shared_ptr<Mesh>& doorRight;
    float& doorOpenAmount;
    float doorLeftBaseX;
    float doorRightBaseX;

    KeyHandler(InputState& i,
               Game& g,
               std::vector<std::shared_ptr<Mesh>>& meshes,
               std::shared_ptr<Mesh>& dLeft,
               std::shared_ptr<Mesh>& dRight,
               float& openAmount,
               float leftBaseX,
               float rightBaseX)
        : input(i),
          game(g),
          objectMeshes(meshes),
          doorLeft(dLeft),
          doorRight(dRight),
          doorOpenAmount(openAmount),
          doorLeftBaseX(leftBaseX),
          doorRightBaseX(rightBaseX) {}

    void onKeyPressed(KeyEvent evt) override {
        switch (evt.key) {
            case Key::W: input.accelerate = true; break;
            case Key::S: input.brake      = true; break;
            case Key::A: input.turnLeft   = true; break;
            case Key::D: input.turnRight  = true; break;

            case Key::R: {
                // Reset game logic
                game.reset();

                // Reset door state
                doorOpenAmount = 0.f;
                if (doorLeft)  doorLeft->position.x  = doorLeftBaseX;
                if (doorRight) doorRight->position.x = doorRightBaseX;

                // Reset pickup visibility
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
    params.title("Bilsimulator - Mountain Test")
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

    // --- Ground plane (big meadow) ---
    auto ground = Mesh::create(
        PlaneGeometry::create(200, 200),
        MeshPhongMaterial::create({{"color", 0x558855}})
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
    auto tireGeo = CylinderGeometry::create(0.5f, 0.5f, 0.4f, 24);
    tireGeo->rotateZ(math::PI / 2);
    auto rimGeo = CylinderGeometry::create(0.3f, 0.3f, 0.45f, 12);
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

    // Front wheels use a steering pivot group (no wobble)
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

    // Rear wheels: just visuals
    auto rlWheel = makeWheelVisual();
    auto rrWheel = makeWheelVisual();
    rlWheel->position.set(-wheelX, wheelY, -wheelZ);
    rrWheel->position.set( wheelX, wheelY, -wheelZ);
    carMesh->add(rlWheel);
    carMesh->add(rrWheel);

    float steeringAngle = 0.f; // front wheels steering
    const float steeringLerp = 0.25f; // smoothing toward target

    // =====================================================
    //                 DOORS (double panel gate)
    // =====================================================
    const float doorZ = 35.f; // align with gate1 (0, 35) in World.cpp

    auto doorMat = MeshPhongMaterial::create({{"color", 0x8B4513}}); // brown

    auto doorLeft  = Mesh::create(
        BoxGeometry::create(1.8f, 4.f, 0.5f),
        doorMat
    );
    auto doorRight = Mesh::create(
        BoxGeometry::create(1.8f, 4.f, 0.5f),
        doorMat
    );

    doorLeft->position.set(-1.f, 2.f, doorZ);
    doorRight->position.set( 1.f, 2.f, doorZ);

    scene.add(doorLeft);
    scene.add(doorRight);

    float doorLeftBaseX  = doorLeft->position.x;
    float doorRightBaseX = doorRight->position.x;
    float doorOpenAmount = 0.f; // 0 = closed, 1 = fully open

    // =====================================================
    //                 GAME LOGIC
    // =====================================================
    Game game;
    InputState input;


    static int lastCount = -1;
    if (lastCount != game.world().objects().size()) {
        // TODO: rebuild objectMeshes + scene versions of meshes
    }
    lastCount = game.world().objects().size();


    // Visual meshes for pickups / obstacles
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
            mesh->position.set((b.minX + b.maxX) * 0.5f,
                               0.8f,
                               (b.minZ + b.maxZ) * 0.5f);
            scene.add(mesh);
            objectMeshes.push_back(mesh);

        } else if (obstacle) {
            auto b = obstacle->bounds();
            float cx = (b.minX + b.maxX) * 0.5f;
            float cz = (b.minZ + b.maxZ) * 0.5f;
            float width  = b.maxX - b.minX;
            float length = b.maxZ - b.minZ;

        } else {
            objectMeshes.push_back(nullptr);
        }
    }


    // =====================================================
    //                 LOAD OBJ WITH MTL + TEXTURE
    // =====================================================
    auto loadModel = [&](const std::string& name) -> std::shared_ptr<Group> {

        std::string objPath = "objmodels/" + name + ".obj";

        OBJLoader loader;
        auto root = loader.load(objPath);

        if (!root) {
            std::cerr << "Failed to load: " << objPath << "\n";
            return nullptr;
        }

        // Scale uniform style (your models are small)
        root->scale.set(4.f, 4.f, 4.f);

        return root;
    };

    // =====================================================
    //                 MOUNTAIN OBJ (edges only)
    // =====================================================

    auto mountain = loadModel("stone-mountain");
    if (mountain) {
        std::vector<Vector3> positions = {
            {-80, 0, -80},
            { 80, 0, -80},
            {-80, 0,  80},
            { 80, 0,  80}
        };

        for (auto& pos : positions) {
            auto m = mountain->clone();
            m->position.copy(pos);
            scene.add(m);
        }
    }

    auto building1 = loadModel("building-village");
    auto building2 = loadModel("building-castle");
    auto building3 = loadModel("building-archery");
    auto building4 = loadModel("building-smelter");

    std::vector<std::pair<std::shared_ptr<Group>, Vector3>> buildingPlacements = {
        {building1, {-30, 0, -10}},
        {building2, { 35, 0,  20}},
        {building3, {-40, 0,  40}},
        {building4, { 20, 0,  60}}
    };

    for (auto& [model, pos] : buildingPlacements) {
        if (!model) continue;
        auto c = model->clone();
        c->position.copy(pos);
        scene.add(c);
    }


    // =====================================================
    //                 INPUT HANDLER
    // =====================================================
    KeyHandler handler(input, game,
                       objectMeshes,
                       doorLeft,
                       doorRight,
                       doorOpenAmount,
                       doorLeftBaseX,
                       doorRightBaseX);

    canvas.addKeyListener(handler);

    // =====================================================
    //                 MAIN LOOP
    // =====================================================
    canvas.animate([&]() {

        float dt = 1.f / 60.f;
        game.update(dt, input);

        const auto& car = game.world().car();

        // --- Sync car mesh ---
        carMesh->position.x = car.position().x;
        carMesh->position.z = car.position().z;
        carMesh->rotation.y = car.rotation();

        float s = car.getVisualScale();
        carMesh->scale.set(s, s, s);

        // --- Steering: smooth, no wobble ---
        float targetSteer = 0.f;
        if (input.turnLeft)  targetSteer =  0.45f;
        if (input.turnRight) targetSteer = -0.45f;

        steeringAngle += (targetSteer - steeringAngle) * steeringLerp;
        flSteer->rotation.y = steeringAngle;
        frSteer->rotation.y = steeringAngle;

        // --- Wheel spin (around own axis only) ---
        float spin = car.speed() * dt * 7.f;
        flWheel->rotation.x += spin;
        frWheel->rotation.x += spin;
        rlWheel->rotation.x += spin;
        rrWheel->rotation.x += spin;

        // --- Chase camera ---
        float fx = std::sin(car.rotation());
        float fz = std::cos(car.rotation());

        Vector3 desired(
            car.position().x - fx * camDistance,
            camHeight,
            car.position().z - fz * camDistance
        );

        camera.position.lerp(desired, camSmooth);
        camera.lookAt({car.position().x, 0.f, car.position().z});

        // --- Door opening based on all pickups ---
        if (game.world().allPickupsCollected()) {
            doorOpenAmount = std::min(1.f, doorOpenAmount + dt);
        }

        // Slide doors apart horizontally
        const float openDistance = 3.f;
        doorLeft->position.x  = doorLeftBaseX  - doorOpenAmount * openDistance;
        doorRight->position.x = doorRightBaseX + doorOpenAmount * openDistance;

        // --- Hide collected pickups ---
        const auto& objs = game.world().objects();
        for (size_t i = 0; i < objs.size() && i < objectMeshes.size(); ++i) {
            auto pick = dynamic_cast<Pickup*>(objs[i].get());
            if (pick && !pick->isActive() && objectMeshes[i]) {
                objectMeshes[i]->visible = false;
            }
        }

        renderer.render(scene, camera);
    });
}
