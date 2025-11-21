#include <threepp/threepp.hpp>
#include "Game.hpp"
#include "Pickup.hpp"
#include "Obstacle.hpp"

#include <vector>
#include <memory>

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
    params.title("Bilsimulator").size(1280, 720).antialiasing(4);

    Canvas canvas(params);

    // --- Renderer (old API: takes size only) ---
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
    float camSmooth   = 0.1f; // 0 = instant snap, closer to 1 = slower movement

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

    // --- Car body mesh ---
    auto carMesh = Mesh::create(
        BoxGeometry::create(2, 1, 4),
        MeshPhongMaterial::create({{"color", 0xff0000}})
    );
    carMesh->position.y = 0.5f;
    scene.add(carMesh);


    //                  WHEEL SETUP
    // =====================================================

    auto wheelMaterial = MeshPhongMaterial::create({{"color", 0x111111}});
    auto wheelGeo = CylinderGeometry::create(0.4f, 0.4f, 0.3f, 16);

    // Front Left wheel (local space relative to carMesh)
    auto frontLeftWheel = Mesh::create(wheelGeo, wheelMaterial);
    frontLeftWheel->rotation.z = math::PI / 2; // lay the wheel on its side
    frontLeftWheel->position.set(-1.1f, 0.4f, 1.5f);
    carMesh->add(frontLeftWheel);

    // Front Right wheel
    auto frontRightWheel = Mesh::create(wheelGeo, wheelMaterial);
    frontRightWheel->rotation.z = math::PI / 2;
    frontRightWheel->position.set(1.1f, 0.4f, 1.5f);
    carMesh->add(frontRightWheel);

    // Rear Left wheel
    auto rearLeftWheel = Mesh::create(wheelGeo, wheelMaterial);
    rearLeftWheel->rotation.z = math::PI / 2;
    rearLeftWheel->position.set(-1.1f, 0.4f, -1.5f);
    carMesh->add(rearLeftWheel);

    // Rear Right wheel
    auto rearRightWheel = Mesh::create(wheelGeo, wheelMaterial);
    rearRightWheel->rotation.z = math::PI / 2;
    rearRightWheel->position.set(1.1f, 0.4f, -1.5f);
    carMesh->add(rearRightWheel);

    std::vector<std::shared_ptr<Mesh>> allWheels = {
        frontLeftWheel, frontRightWheel, rearLeftWheel, rearRightWheel
    };

    // Steering + spin state
    float steerAngle = 0.f;
    const float maxSteerAngle = math::PI / 6; // 30 degrees
    const float steerSmooth   = 0.15f;

    float wheelRotation = 0.f;
    const float wheelSpinFactor = 5.f; // tweak if too fast/slow


    //                  GAME LOGIC
    // =====================================================

    Game game;
    InputState input;

    // --- Visual meshes for pickups & obstacles ---
    std::vector<std::shared_ptr<Mesh>> objectMeshes;
    objectMeshes.reserve(game.world().objects().size());

    for (const auto& obj : game.world().objects()) {

        auto pickup = dynamic_cast<Pickup*>(obj.get());
        auto obstacle = dynamic_cast<Obstacle*>(obj.get());

        if (pickup) {
            // create a green sphere for pickups
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
            // create a blue box for obstacles
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

    // --- Main loop (no dt parameter in this threepp version) ---
    canvas.animate([&]() {

        float dt = 1.f / 60.f; // fixed timestep
        game.update(dt, input);

        // --- Sync car mesh with logic ---
        const auto& car = game.world().car();
        carMesh->position.x = car.position().x;
        carMesh->position.z = car.position().z;
        carMesh->rotation.y = car.rotation();
        float s = car.getVisualScale();
        carMesh->scale.set(s, s, s);


        //        WHEEL SPIN + STEERING ANIMATION
        // =====================================================

        // Steering angle based on input
        float targetSteer = 0.f;
        if (input.turnLeft) {
            targetSteer = maxSteerAngle;
        } else if (input.turnRight) {
            targetSteer = -maxSteerAngle;
        }

        steerAngle += (targetSteer - steerAngle) * steerSmooth;

        // Apply steering to front wheels only
        frontLeftWheel->rotation.y = steerAngle;
        frontRightWheel->rotation.y = steerAngle;

        // Spin wheels based on car speed
        wheelRotation += car.speed() * dt * wheelSpinFactor;

        for (auto& w : allWheels) {
            // base orientation already rotated around Z in setup
            w->rotation.x = wheelRotation;
        }



        float fx = std::sin(car.rotation());
        float fz = std::cos(car.rotation());

        Vector3 desiredPos(
            car.position().x - fx * camDistance,
            camHeight,
            car.position().z - fz * camDistance
        );

        camera.position.lerp(desiredPos, camSmooth);
        camera.lookAt({car.position().x, 0.f, car.position().z});

        // --- Update pickup visibility (hide collected ones) ---
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
