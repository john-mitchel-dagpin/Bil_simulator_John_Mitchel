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
    //          🔧 WHEELS
    // ================================

    auto tireGeo = CylinderGeometry::create(0.5f, 0.5f, 0.4f, 24);
    tireGeo->rotateZ(math::PI / 2);

    auto rimGeo = CylinderGeometry::create(0.3f, 0.3f, 0.45f, 12);
    rimGeo->rotateZ(math::PI / 2);

    auto tireMat = MeshPhongMaterial::create({{"color", 0x111111}});
    auto rimMat  = MeshPhongMaterial::create({{"color", 0xffffff}});

    auto makeWheel = [&]() {
        auto group = Group::create();
        auto tire = Mesh::create(tireGeo, tireMat);
        auto rim  = Mesh::create(rimGeo, rimMat);
        group->add(tire);
        group->add(rim);
        return group;
    };

    float wheelY = 0.0f;
    float wheelX = 1.1f;
    float wheelZ = 1.6f;

    auto wheelFL = makeWheel();
    auto wheelFR = makeWheel();
    auto wheelRL = makeWheel();
    auto wheelRR = makeWheel();

    wheelFL->position.set(-wheelX, wheelY,  wheelZ);
    wheelFR->position.set( wheelX, wheelY,  wheelZ);
    wheelRL->position.set(-wheelX, wheelY, -wheelZ);
    wheelRR->position.set( wheelX, wheelY, -wheelZ);

    carMesh->add(wheelFL);
    carMesh->add(wheelFR);
    carMesh->add(wheelRL);
    carMesh->add(wheelRR);

    std::vector<std::shared_ptr<Group>> wheels = {wheelFL, wheelFR, wheelRL, wheelRR};

    // --- Game logic ---
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
    //       MAIN GAME LOOP
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

        // --- Wheel spinning ---
        float spin = car.speed() * dt * 7.0f;
        for (auto& w : wheels) {
            w->rotation.x += spin;
        }

        // --- Steering wheels ---
        float steer = 0.f;
        if (input.turnLeft)  steer = 0.5f;
        if (input.turnRight) steer = -0.5f;

        wheelFL->rotation.y = steer;
        wheelFR->rotation.y = steer;

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
