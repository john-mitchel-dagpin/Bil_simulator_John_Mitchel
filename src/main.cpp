#include <threepp/threepp.hpp>
#include "Game.hpp"
#include "Pickup.hpp"
#include "Obstacle.hpp"

#include <vector>
#include <memory>
#include <algorithm>

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
    renderer.setClearColor(Color(0x202530)); // darker sky-ish

    // --- Scene ---
    Scene scene;

    // --- Camera with chase behaviour ---
    PerspectiveCamera camera(60, canvas.aspect(), 0.1f, 500.f);
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

    // --- Lighting: fantasy-ish ---
    auto ambient = AmbientLight::create(0x555555);
    scene.add(ambient);

    auto dirLight = DirectionalLight::create(0xffffff, 0.9f);
    dirLight->position.set(30, 50, 10);
    scene.add(dirLight);

    // --- Ground: big green meadow ---
    auto ground = Mesh::create(
        PlaneGeometry::create(200, 200),
        MeshPhongMaterial::create({{"color", 0x3a7f3a}})
    );
    ground->rotation.x = -math::PI / 2;
    scene.add(ground);

    // --- Pixel-style stone path tiles down center ---
    auto makePathTile = [&](float z, int index) {
        unsigned int color = (index % 2 == 0) ? 0x777777 : 0x888888;
        auto tile = Mesh::create(
            BoxGeometry::create(4.f, 0.2f, 4.f),
            MeshPhongMaterial::create({{"color", color}})
        );
        tile->position.set(0.f, 0.1f, z);
        scene.add(tile);
    };

    int tileIndex = 0;
    for (float z = -40.f; z <= 140.f; z += 4.f) {
        makePathTile(z, tileIndex++);
    }

    // --- Lakes: glowing water planes on sides ---
    auto waterMat = MeshPhongMaterial::create({
        {"color", 0x3399ff},
        {"transparent", true},
        {"opacity", 0.7f}
    });

    auto makeLake = [&](float x, float z, float w, float h) {
        auto lake = Mesh::create(
            PlaneGeometry::create(w, h),
            waterMat
        );
        lake->rotation.x = -math::PI / 2;
        lake->position.set(x, 0.02f, z);
        scene.add(lake);
    };

    makeLake(-9.f, -10.f, 16.f, 16.f);
    makeLake( 9.f, -5.f, 14.f, 12.f);
    makeLake(-9.f,  40.f, 16.f, 18.f);
    makeLake( 9.f,  55.f, 18.f, 18.f);

    // --- Trees: simple fantasy trees from primitives ---
    auto trunkGeo = CylinderGeometry::create(0.3f, 0.4f, 2.0f, 12);
    auto leavesGeo = SphereGeometry::create(1.0f, 16, 16);
    auto trunkMat = MeshPhongMaterial::create({{"color", 0x5b3b18}});
    auto leavesMat = MeshPhongMaterial::create({{"color", 0x1ea34a}});

    auto makeTree = [&](float x, float z) {
        auto tree = Group::create();
        auto trunk = Mesh::create(trunkGeo, trunkMat);
        auto leaves = Mesh::create(leavesGeo, leavesMat);
        trunk->position.y = 1.f;
        leaves->position.y = 2.2f;
        tree->add(trunk);
        tree->add(leaves);
        tree->position.set(x, 0.f, z);
        scene.add(tree);
    };

    // place trees mostly along edges
    for (float z = -30.f; z <= 120.f; z += 12.f) {
        makeTree(-10.5f, z + 3.f);
        makeTree( 10.5f, z - 4.f);
    }
    makeTree(-7.f, -15.f);
    makeTree( 7.f, -18.f);
    makeTree(-8.f, 30.f);
    makeTree( 8.f, 38.f);
    makeTree(-7.f, 80.f);
    makeTree( 7.f, 92.f);

    // --- Rocks: scattered decorations ---
    auto rockGeo = SphereGeometry::create(0.7f, 12, 12);
    auto rockMat = MeshPhongMaterial::create({{"color", 0x555555}});

    auto makeRock = [&](float x, float z) {
        auto rock = Mesh::create(rockGeo, rockMat);
        rock->position.set(x, 0.5f, z);
        scene.add(rock);
    };

    makeRock(-2.f, -12.f);
    makeRock( 3.f, -6.f);
    makeRock(-3.f, 28.f);
    makeRock( 4.f, 42.f);
    makeRock( 0.f, 62.f);
    makeRock(-4.f, 88.f);
    makeRock( 4.f, 104.f);

    // --- Car body ---
    auto carMesh = Mesh::create(
        BoxGeometry::create(2, 1, 4),
        MeshPhongMaterial::create({{"color", 0xff3333}})
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

    auto rlVisual = makeWheelVisual();
    rlVisual->position.set(-wheelX, wheelY, -wheelZ);
    carMesh->add(rlVisual);

    auto rrVisual = makeWheelVisual();
    rrVisual->position.set(wheelX, wheelY, -wheelZ);
    carMesh->add(rrVisual);

    std::vector<std::shared_ptr<Group>> spinWheels = {flVisual, frVisual, rlVisual, rrVisual};

    float steeringAngle = 0.f;
    const float maxSteerAngle = 0.45f;
    const float steerLerp     = 0.25f;
    const float wheelSpinFactor = 7.f;

    // ================================
    //          DOORS / GATES
    // ================================
    auto gateStoneMat1 = MeshPhongMaterial::create({{"color", 0xaaa67f}});
    auto gateStoneMat2 = MeshPhongMaterial::create({{"color", 0x8f6bbf}});

    auto door1Mesh = Mesh::create(
        BoxGeometry::create(4.f, 6.f, 0.6f),
        gateStoneMat1
    );
    door1Mesh->position.set(0.f, 3.f, 15.f);
    scene.add(door1Mesh);

    auto door2Mesh = Mesh::create(
        BoxGeometry::create(4.f, 6.f, 0.6f),
        gateStoneMat2
    );
    door2Mesh->position.set(0.f, 3.f, 75.f);
    scene.add(door2Mesh);

    float door1BaseY = door1Mesh->position.y;
    float door2BaseY = door2Mesh->position.y;
    float door1OpenAmount = 0.f;
    float door2OpenAmount = 0.f;
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
            // pickups: glowing green spheres
            auto mesh = Mesh::create(
                SphereGeometry::create(0.8f, 16, 16),
                MeshPhongMaterial::create({{"color", 0x00ff55}})
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

            // heuristics: big/wide -> cliff wall, small -> rock block
            std::shared_ptr<Mesh> mesh;

            if (width > 4.f || length > 20.f) {
                // cliff-like walls along valley
                mesh = Mesh::create(
                    BoxGeometry::create(width, 4.f, length),
                    MeshPhongMaterial::create({{"color", 0x28305a}})
                );
                mesh->position.set(cx, 2.f, cz);
            } else if (width > 2.5f && length < 2.f) {
                // likely a gate collider, we already draw doors separately -> make invisible
                mesh = Mesh::create(
                    BoxGeometry::create(width, 0.1f, length),
                    MeshPhongMaterial::create({{"color", 0x000000}})
                );
                mesh->visible = false;
                mesh->position.set(cx, 0.05f, cz);
            } else {
                // regular rock obstacles
                mesh = Mesh::create(
                    BoxGeometry::create(width, 2.f, length),
                    MeshPhongMaterial::create({{"color", 0x444444}})
                );
                mesh->position.set(cx, 1.f, cz);
            }

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

        // --- Steering animation (simple + smooth) ---
        float targetSteer = 0.f;
        if (input.turnLeft)  targetSteer =  maxSteerAngle;
        if (input.turnRight) targetSteer = -maxSteerAngle;
        steeringAngle += (targetSteer - steeringAngle) * steerLerp;

        flSteer->rotation.y = steeringAngle;
        frSteer->rotation.y = steeringAngle;

        // --- Wheel spinning ---
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

        // --- Door 1 animation (opens after some pickups) ---
        if (game.world().gate1IsOpen()) {
            door1OpenAmount = std::min(1.f, door1OpenAmount + doorOpenSpeed * dt);
        } else {
            door1OpenAmount = std::max(0.f, door1OpenAmount - doorOpenSpeed * dt);
        }
        door1Mesh->position.y = door1BaseY + door1OpenAmount * 5.f;

        // --- Door 2 animation (opens when all pickups collected) ---
        if (game.world().gate2IsOpen()) {
            door2OpenAmount = std::min(1.f, door2OpenAmount + doorOpenSpeed * dt);
        } else {
            door2OpenAmount = std::max(0.f, door2OpenAmount - doorOpenSpeed * dt);
        }
        door2Mesh->position.y = door2BaseY + door2OpenAmount * 5.f;

        // --- Update pickups visibility (handles reset) ---
        const auto& objects = game.world().objects();
        for (std::size_t i = 0; i < objects.size(); ++i) {
            if (!objectMeshes[i]) continue;
            if (auto pickup = dynamic_cast<Pickup*>(objects[i].get())) {
                objectMeshes[i]->visible = pickup->isActive();
            }
        }

        renderer.render(scene, camera);
    });
}
