
#include <threepp/threepp.hpp>
#include "Game.hpp"

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

    // --- Window setup (your version supports Canvas::Parameters) ---
    Canvas::Parameters params;
    params.title("Bilsimulator").size(1280, 720).antialiasing(4);

    Canvas canvas(params);

    // --- Renderer (old API: only takes size()) ---
    GLRenderer renderer(canvas.size());
    renderer.setClearColor(Color(0x202020));

    // --- Scene ---
    Scene scene;

    // --- Camera (use aspect() because your version supports it) ---
    PerspectiveCamera camera(60, canvas.aspect(), 0.1f, 1000.f);
    camera.position.set(0, 15, 20);
    camera.lookAt(0, 0, 0);

    // --- Chase camera settings ---
    float camDistance = 15.f;
    float camHeight = 8.f;
    float camSmooth = 0.1f; // lower = smoother, delayed movement

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

    // --- Car mesh ---
    auto carMesh = Mesh::create(
        BoxGeometry::create(2, 1, 4),
        MeshPhongMaterial::create({{"color", 0xff0000}})
    );
    carMesh->position.y = 0.5f;
    scene.add(carMesh);

    // --- Game logic system ---
    Game game;
    InputState input;

    // --- Keyboard handler (MUST be a reference in your API) ---
    KeyHandler handler(input, game);
    canvas.addKeyListener(handler);

    // --- Animation loop (your version has NO dt parameter) ---
    canvas.animate([&]() {

        float dt = 1.f / 60.f; // simulate 60FPS fixed timestep
        game.update(dt, input);

        // --- Sync car mesh with car logic ---
        const auto& car = game.world().car();
        carMesh->position.x = car.position().x;
        carMesh->position.z = car.position().z;
        carMesh->rotation.y = car.rotation();

        // ------------------------------
        //     CHASE CAMERA LOGIC
        // ------------------------------

        // Compute forward direction
        float fx = std::sin(car.rotation());
        float fz = std::cos(car.rotation());

        // Desired camera position (behind + above)
        Vector3 desiredPos(
            car.position().x - fx * camDistance,
            camHeight,
            car.position().z - fz * camDistance
        );

        // Smooth interpolation
        camera.position.lerp(desiredPos, camSmooth);

        // Always look at the car
        camera.lookAt({car.position().x, 0.f, car.position().z});

        // ------------------------------

        renderer.render(scene, camera);
    });
}
