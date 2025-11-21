#include <threepp/threepp.hpp>
#include "Game.hpp"
#include "Pickup.hpp"
#include "Obstacle.hpp"

#include <vector>
#include <memory>
#include <algorithm> // for std::clamp, std::min

using namespace threepp;

class KeyHandler : public KeyListener {
public:
    InputState& input;
    Game& game;

    KeyHandler(InputState& i, Game& g) : input(i), game(g) {}

    void onKeyPressed(KeyEvent evt) override {
        switch (evt.key) {
            case Key::W: input.accelerate = true; break;
            case Key::S: input.brake = true; break;
            case Key::A: input.turnLeft = true; break;
            case Key::D: input.turnRight = true; break;
            case Key::R: game.reset(); break;
            default: break;
        }
    }

    void onKeyReleased(KeyEvent evt) override {
        switch (evt.key) {
            case Key::W: input.accelerate = false; break;
            case Key::S: input.brake = false; break;
            case Key::A: input.turnLeft = false; break;
            case Key::D: input.turnRight = false; break;
            default: break;
        }
    }
};

int main() {

    // --- Window / canvas ---
    Canvas::Parameters params;
    params.title("Bilsimulator")
          .size(1280, 720)
          .resizable(true)
          .antialiasing(4);

    Canvas canvas(params);

    // --- Renderer ---
    GLRenderer renderer(canvas.size());
    renderer.setClearColor(Color(0x202020));

    // --- Scene ---
    Scene scene;

    // --- Camera with chase behaviour ---
    PerspectiveCamera camera(60, canvas.aspect(), 0.1f, 1000.f);
    camera.position.set(0, 15, 20);
    camera.lookAt(0, 0, 0);

    float camDistance = 15.f;
    float camHeight   = 8.f;
    float camSmooth   = 0.1f;

    // --- Resize handling ---
    canvas.onWindowResize([&](WindowSize size) {
        camera.aspect = float(size.width()) / float(size.height());
        camera.updateProjectionMatrix();
        renderer.setSize(size);
    });

    // --- Lighting ---
    auto light = DirectionalLight::create(0xffffff, 1.0f);
    light->position.set(10, 20, 10);
    scene.add(light);

    // --- Ground plane ---
    auto ground = Mesh::create(
        PlaneGeometry::create(200, 200),
        MeshPhongMaterial::create({{"color", 0x444444}})
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

    // ================================
    //          WHEELS (hierarchy)
    // ================================
    auto tireGeo = CylinderGeometry::create(0.5f, 0.5f, 0.4f, 24);
    tireGeo->rotateZ(math::PI / 2);

    auto rimGeo = CylinderGeometry::create(0.3f, 0.3f, 0.45f, 12);
    rimGeo->rotateZ(math::PI / 2);

    auto tireMat = MeshPhongMaterial::create({{"color", 0x111111}});
    auto rimMat  = MeshPhongMaterial::create({{"color", 0xffffff}});

    // Visual wheel (tire + rim) grouped together
    auto makeWheelVisual = [&]() {
        auto visual = Group::create();
        auto tire = Mesh::create(tireGeo, tireMat);
        auto rim  = Mesh::create(rimGeo, rimMat);
        visual->add(tire);
        visual->add(rim);
        return visual;
    };

    float wheelY = 0.0f;
    float wheelX = 1.1f;
    float wheelZ = 1.6f;

    // Front: steering group -> visual (spinning) group
    auto flSteer = Group::create();
    auto flVisual = makeWheelVisual();
    flSteer->add(flVisual);
    flSteer->position.set(-wheelX, wheelY, wheelZ);
    carMesh->add(flSteer);

    auto frSteer = Group::create();
    auto frVisual = makeWheelVisual();
    frSteer->add(frVisual);
    frSteer->position.set(wheelX, wheelY, wheelZ);
    carMesh->add(frSteer);

    // Rear: only visual (no steering)
    auto rlVisual = makeWheelVisual();
    rlVisual->position.set(-wheelX, wheelY, -wheelZ);
    carMesh->add(rlVisual);

    auto rrVisual = makeWheelVisual();
    rrVisual->position.set(wheelX, wheelY, -wheelZ);
    carMesh->add(rrVisual);

    // Wheels that spin
    std::vector<std::shared_ptr<Group>> spinWheels = {flVisual, frVisual, rlVisual, rrVisual};

    // Steering state (just for front steer groups)
    float steeringAngle = 0.f;
    const float maxSteerAngle = 0.45f;   // ~25 degrees
    const float steerLerp     = 0.25f;   // smoothing factor [0..1]

    const float wheelSpinFactor = 7.f;

    // ================================
    //          DOOR / GATE
    // ================================
    auto doorMesh = Mesh::create(
        BoxGeometry::create(4.f, 4.f, 0.5f),
        MeshPhongMaterial::create({{"color", 0x888800}})
    );
    doorMesh->position.set(0.f, 2.f, 12.f); // same z as gate obstacle
    scene.add(doorMesh);

    float doorBaseY = doorMesh->position.y;
    float doorOpenAmount = 0.f;      // 0 = closed, 1 = fully open
    const float doorOpenSpeed = 1.0f;

    // ================================
    //          GAME LOGIC
    // ================================
    Game game;
    InputState input;

    // --- Visual meshes for pickups & obstacles ---
    std::vector<std::shared_ptr<Mesh>> objectMeshes;
    objectMeshes.reserve(game.world().objects().size());

    for (const auto& obj : game.world().objects()) {

        auto pickup = dynamic_cast<Pickup*>(obj.get());
        auto obstacle = dynamic_cast<Obstacle*>(obj.get());

        if (pickup) {
            auto mesh = Mesh::create(
                SphereGeometry::create(0.8f, 16, 16),
                MeshPhongMaterial::create({{"color", 0x00ff00}})
            );

            auto b = pickup->bounds();
            float cx = (b.minX + b.maxX) * 0.5f;
            float cz = (b.minZ + b.maxZ) * 0.5f;

            mesh->position.set(cx, 0.8f, cz);
            scene.add(mesh);
            objectMeshes.push_back(mesh);

        } else if (obstacle) {
            auto b = obstacle->bounds();
            float cx = (b.minX + b.maxX) * 0.5f;
            float cz = (b.minZ + b.maxZ) * 0.5f;
            float width  = (b.maxX - b.minX);
            float length = (b.maxZ - b.minZ);

            auto mesh = Mesh::create(
                BoxGeometry::create(width, 2.f, length),
                MeshPhongMaterial::create({{"color", 0x3333ff}})
            );

            mesh->position.set(cx, 1.f, cz);
            scene.add(mesh);
            objectMeshes.push_back(mesh);

        } else {
            objectMeshes.push_back(nullptr);
        }
    }

    // --- Keyboard handler ---
    KeyHandler handler(input, game);
    canvas.addKeyListener(handler);

    // ================================
    //          MAIN LOOP
    // ================================
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

        // --- Simple, stable steering (A/D → left/right) ---
        float targetSteer = 0.f;
        if (input.turnLeft)  targetSteer =  maxSteerAngle;
        if (input.turnRight) targetSteer = -maxSteerAngle;

        // Smooth towards target but no weird overshoot
        steeringAngle += (targetSteer - steeringAngle) * steerLerp;

        // Apply to front steer groups ONLY
        flSteer->rotation.y = steeringAngle;
        frSteer->rotation.y = steeringAngle;

        // --- Wheel spinning (spin only visual groups) ---
        float spin = car.speed() * dt * wheelSpinFactor;
        for (auto& w : spinWheels) {
            w->rotation.x += spin;
        }

        // --- Chase camera ---
        float fx = std::sin(car.rotation());
        float fz = std::cos(car.rotation());

        Vector3 desiredPos(
            car.position().x - fx * camDistance,
            camHeight,
            car.position().z - fz * camDistance
        );

        camera.position.lerp(desiredPos, camSmooth);
        camera.lookAt({car.position().x, 0.f, car.position().z});

        // --- Door opening animation ---
        if (game.world().allPickupsCollected() && doorOpenAmount < 1.f) {
            doorOpenAmount = std::min(1.f, doorOpenAmount + doorOpenSpeed * dt);
        }
        doorMesh->position.y = doorBaseY + doorOpenAmount * 5.f;

        // --- Update pickups visibility ---
        const auto& objects = game.world().objects();
        for (std::size_t i = 0; i < objects.size(); ++i) {
            if (!objectMeshes[i]) continue;
            auto pickup = dynamic_cast<Pickup*>(objects[i].get());
            if (pickup && !pickup->isActive()) {
                objectMeshes[i]->visible = false;
            }
        }

        renderer.render(scene, camera);
    });
}
