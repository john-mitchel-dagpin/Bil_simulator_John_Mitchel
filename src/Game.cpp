#include "Game.hpp"
#include <iostream>

using namespace threepp;

// --------------------------
// Key Listener
// --------------------------

class GameKeyListener : public KeyListener {

public:
    bool& forward;
    bool& backward;
    bool& left;
    bool& right;
    Game& game;

    GameKeyListener(bool& f, bool& b, bool& l, bool& r, Game& g)
        : forward(f), backward(b), left(l), right(r), game(g) {}

    void onKeyPressed(KeyEvent evt) override {
        if (evt.key == Key::W) forward = true;
        if (evt.key == Key::S) backward = true;
        if (evt.key == Key::A) left = true;
        if (evt.key == Key::D) right = true;
        if (evt.key == Key::R) game.reset();
    }

    void onKeyReleased(KeyEvent evt) override {
        if (evt.key == Key::W) forward = false;
        if (evt.key == Key::S) backward = false;
        if (evt.key == Key::A) left = false;
        if (evt.key == Key::D) right = false;
    }
};

// --------------------------
// Game
// --------------------------

Game::Game(Canvas& canvas, GLRenderer& renderer)
    : canvas_(canvas), renderer_(renderer) {

    scene_  = Scene::create();
    camera_ = PerspectiveCamera::create(60, canvas.aspect(), 0.1f, 200.f);
    scene_->add(camera_);

    initScene();
}

void Game::initScene() {

    // Ambient light
    scene_->add(AmbientLight::create(Color::white, 1.0f));

    // Floor
    auto floorGeom = PlaneGeometry::create(100, 100);
    auto floorMat  = MeshPhongMaterial::create();
    floorMat->color = Color::lightgrey;
    auto floor = Mesh::create(floorGeom, floorMat);
    floor->rotation.x = -math::PI / 2;
    scene_->add(floor);

    // Car body
    auto bodyGeom = BoxGeometry::create(1.5f, 0.5f, 1.0f);
    auto bodyMat  = MeshPhongMaterial::create();
    bodyMat->color = Color::red;
    carMesh_ = Mesh::create(bodyGeom, bodyMat);
    scene_->add(carMesh_);

    // Wheels (children of car)
    {
        auto wheelGeom = CylinderGeometry::create(0.3f, 0.3f, 0.2f, 16);
        auto wheelMat  = MeshPhongMaterial::create();
        wheelMat->color = Color::black;

        std::vector<Vector3> offsets = {
            {-0.55f, -0.25f,  0.45f}, // FL
            { 0.55f, -0.25f,  0.45f}, // FR
            {-0.55f, -0.25f, -0.45f}, // RL
            { 0.55f, -0.25f, -0.45f}  // RR
        };

        for (auto& off : offsets) {
            auto w = Mesh::create(wheelGeom, wheelMat);
            w->rotation.x = math::PI / 2.f;
            w->position.copy(off);
            carMesh_->add(w);
            wheels_.push_back(w);
        }
    }

    spawnPickups();

    // Obstacles
    {
        std::vector<::Vec2> positions = {
            {3, 2}, {-2, -3}, {0, 4}
        };

        auto boxGeom = BoxGeometry::create(1.2f, 1.2f, 1.2f);
        auto boxMat  = MeshPhongMaterial::create();
        boxMat->color = Color::darkgrey;

        for (auto& pos : positions) {
            Obstacle ob;
            ob.pos = pos;
            ob.radius = 1.0f;

            ob.mesh = Mesh::create(boxGeom, boxMat);
            ob.mesh->position.set(pos.x, 0.f, pos.y);

            obstacles_.push_back(ob);
            scene_->add(ob.mesh);
        }
    }
}

void Game::spawnPickups() {

    pickups_.clear();
    pickupMeshes_.clear();

    pickups_.push_back({ {5, 0}, 0.8f, false, Pickup::SPEED });
    pickups_.push_back({ {-4, 3}, 0.8f, false, Pickup::BIGNESS });
    pickups_.push_back({ {2, -4}, 0.8f, false, Pickup::SPEED });

    for (auto& p : pickups_) {

        auto mat = MeshPhongMaterial::create();
        mat->color = (p.type == Pickup::SPEED ? Color::green : Color::purple);

        auto geom = SphereGeometry::create(0.5f, 16, 16);

        auto m = Mesh::create(geom, mat);
        m->position.set(p.pos.x, 0.f, p.pos.y);

        pickupMeshes_.push_back(m);
        scene_->add(m);
    }
}

void Game::reset() {

    car_.reset();
    carRenderScale_ = 1.f;
    carMesh_->scale.set(1,1,1);

    for (auto& p : pickups_) p.collected = false;
    for (auto& m : pickupMeshes_) m->visible = true;
}

void Game::handlePickup(Pickup& p, std::shared_ptr<Mesh>& mesh) {

    if (p.collected) return;

    p.collected = true;
    mesh->visible = false;

    if (p.type == Pickup::SPEED) {
        car_.maxSpeed *= 1.5f;
        std::cout << "Speed +50%\n";
    } else {
        carRenderScale_ *= 1.5f;
        carMesh_->scale.set(carRenderScale_, carRenderScale_, carRenderScale_);
        std::cout << "Size +50%\n";
    }
}

void Game::run() {

    bool forward=false, backward=false, left=false, right=false;

    GameKeyListener kl(forward, backward, left, right, *this);
    canvas_.addKeyListener(kl);

    // resize
    canvas_.onWindowResize([&](const WindowSize& s){
        renderer_.setSize({s.width(), s.height()});
        camera_->aspect = s.aspect();
        camera_->updateProjectionMatrix();
    });

    Clock clock;

    canvas_.animate([&]() {

        float dt = clock.getDelta();

        car_.update(dt, forward, backward, left, right);

        // position the 3D car
        carMesh_->position.x = car_.position.x;
        carMesh_->position.z = car_.position.y;
        carMesh_->rotation.y = -car_.heading;

        // wheel rotation + steering
        {
            const float wheelRadius = 0.3f;
            float spin = (car_.speed / wheelRadius) * dt;

            for (size_t i = 0; i < wheels_.size(); i++) {

                // front wheels steer
                if (i == 0 || i == 1) {
                    wheels_[i]->rotation.y = car_.steerAngle;
                }

                wheels_[i]->rotation.z -= spin;
            }
        }

        // pickups
        for (size_t i = 0; i < pickups_.size(); i++) {
            auto& p = pickups_[i];
            auto& m = pickupMeshes_[i];

            if (!m->visible) continue;

            if (car_.checkCollision(p.pos, p.radius, carRadius_)) {
                handlePickup(p, m);
            }
        }

        // obstacles
        for (auto& ob : obstacles_) {
            if (car_.checkCollision(ob.pos, ob.radius, carRadius_)) {
                car_.speed = 0;
            }
        }

        // camera follow
        float dist = 6.f;
        camera_->position.x = car_.position.x - std::cos(car_.heading)*dist;
        camera_->position.z = car_.position.y - std::sin(car_.heading)*dist;
        camera_->position.y = 3.f;
        camera_->lookAt(carMesh_->position);

        renderer_.render(*scene_, *camera_);
    });
}
