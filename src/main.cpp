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

    KeyHandler(InputState& i, Game& g) : input(i), game(g) {}

    void onKeyPressed(KeyEvent evt) override {
        switch (evt.key) {
            case Key::W: input.accelerate = true; break;
            case Key::S: input.brake      = true; break;
            case Key::A: input.turnLeft   = true; break;
            case Key::D: input.turnRight  = true; break;
            case Key::R: game.reset();            break;
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
    //                 DOOR (visual for gate 1)
    // =====================================================
    auto doorMesh = Mesh::create(
        BoxGeometry::create(4.f, 4.f, 0.5f),
        MeshPhongMaterial::create({{"color", 0x888800}})
    );
    doorMesh->position.set(0.f, 2.f, 15.f); // align with gate1 z≈15
    float doorBaseY = doorMesh->position.y;
    float doorOpenAmount = 0.f;

    scene.add(doorMesh);

    // =====================================================
    //                 GAME LOGIC
    // =====================================================
    Game game;
    InputState input;

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
            float width  = b.maxX - b.minX;
            float length = b.maxZ - b.minZ;

            // Side / far boundaries: keep as invisible colliders
            bool isBigBoundary =
                (length > 40.f || width > 20.f);

            // Gates: small-ish, near z ≈ 15 or 75
            bool isGateLike =
                (std::abs(b.maxZ + b.minZ) * 0.5f - 15.f < 2.f) ||
                (std::abs(b.maxZ + b.minZ) * 0.5f - 75.f < 2.f);

            std::shared_ptr<Mesh> mesh;

            if (isBigBoundary) {
                // Invisible tall boundary collider
                mesh = Mesh::create(
                    BoxGeometry::create(width, 0.5f, length),
                    MeshPhongMaterial::create({{"color", 0x000000}})
                );
                mesh->visible = false;
                mesh->position.set((b.minX + b.maxX) * 0.5f,
                                   0.25f,
                                   (b.minZ + b.maxZ) * 0.5f);
            } else if (isGateLike) {
                // Keep visible (small stone-ish barrier)
                mesh = Mesh::create(
                    BoxGeometry::create(width, 3.f, length),
                    MeshPhongMaterial::create({{"color", 0x444488}})
                );
                mesh->position.set((b.minX + b.maxX) * 0.5f,
                                   1.5f,
                                   (b.minZ + b.maxZ) * 0.5f);
            } else {
                // Normal boulders
                mesh = Mesh::create(
                    BoxGeometry::create(width, 2.f, length),
                    MeshPhongMaterial::create({{"color", 0x555555}})
                );
                mesh->position.set((b.minX + b.maxX) * 0.5f,
                                   1.f,
                                   (b.minZ + b.maxZ) * 0.5f);
            }

            scene.add(mesh);
            objectMeshes.push_back(mesh);

        } else {
            objectMeshes.push_back(nullptr);
        }
    }

    // =====================================================
    //                 MOUNTAIN OBJ (edges only)
    // =====================================================
    OBJLoader loader;
    auto mountainRoot = loader.load("objmodels/stone-mountain.obj");

    if (!mountainRoot) {
        std::cerr << "Failed to load mountain OBJ\n";
    } else {
        const float scale = 8.f;
        std::vector<Vector3> positions = {
            {-80, 0, -80},
            { 80, 0, -80},
            {-80, 0,  80},
            { 80, 0,  80},
            {  0, 0,  90},
            { 90, 0,   0},
        };

        for (const auto& pos : positions) {
            auto clone = mountainRoot->clone();
            clone->scale.set(scale, scale, scale);
            clone->position.copy(pos);
            scene.add(clone);
        }
    }

    // =====================================================
    //                 INPUT HANDLER
    // =====================================================
    KeyHandler handler(input, game);
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
        doorMesh->position.y = doorBaseY + doorOpenAmount * 5.f;

        // --- Hide collected pickups ---
        const auto& objs = game.world().objects();
        for (size_t i = 0; i < objs.size(); ++i) {
            auto pick = dynamic_cast<Pickup*>(objs[i].get());
            if (pick && !pick->isActive() && objectMeshes[i]) {
                objectMeshes[i]->visible = false;
            }
        }

        renderer.render(scene, camera);
    });
}
