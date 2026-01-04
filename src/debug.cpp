#include "Scene.h"
#include "ModelLoader.h"

#include <string>
#include <logger.h>

#include "lights/DirectionalLight.h"
#include "lights/PointLight.h"

// TODO: should I create some kind of scene factory?
//	it would be responsible for instantiating and setting up different scenes

// this is my test scene configuration
static void baseScene(Scene& scene) {
	logger.info("creating dragon debug scene");

	// ADD LIGHTS
	scene.addLight(std::make_shared<DirectionalLight>(glm::vec3(-1), glm::vec3(255, 254, 247)));
	//scene.addLight(std::make_shared<DirectionalLight>(glm::vec3(-1, -1, -1), glm::vec3(228, 112, 37)));
	//scene.addLight(std::make_shared<DirectionalLight>(glm::vec3(1, -1, -1), glm::vec3(239, 192, 112)));
	//scene.addLight(std::make_shared<DirectionalLight>(glm::vec3(-1, -1, 1), glm::vec3(255, 245, 182)));

	// ADD ENTITY
	auto object = ModelLoader::load("assets/models/stanford_dragon_pbr/scene.gltf");
	object->name = "stanford_dragon";
	object->transform.scale = glm::vec3(0.08);
	object->transform.rotation = glm::vec3(0, 0, 0);
	scene.addObject(object);

	auto sphere = ModelLoader::load("assets/models/sphere.obj");
	sphere->transform.position = glm::vec3(-1.3, 6, 3);
	sphere->name = "omega_sphere";
	scene.addObject(sphere);

	// camera
	scene.camera.setTarget(glm::vec3(0, 4, 0));
}

static void sphereScene(Scene& scene) {
	logger.info("creating spheres scene");

	// LIGHTS
	scene.addLight(std::make_shared<DirectionalLight>(glm::vec3(-1), glm::vec3(255, 254, 247)));

	// OBJECTS
	auto sphere0 = ModelLoader::load("assets/models/sphere.obj", "sphere_gold");
	sphere0->transform.position = glm::vec3(0.0f);
	sphere0->material->albedo = glm::vec3(0.56f, 0.5f, 0.19f);
	sphere0->material->roughness = 0.27f;
	sphere0->material->metalness = 1.0f;
	scene.addObject(sphere0);

	auto sphere1 = ModelLoader::load("assets/models/sphere.obj", "sphere_titanium");
	sphere1->transform.position = glm::vec3(2.5f, 0.0f, 0.0f);
	sphere1->material->albedo = glm::vec3(0.62f);
	sphere1->material->roughness = 0.43f;
	sphere1->material->metalness = 1.0f;
	scene.addObject(sphere1);

	//auto dragon = ModelLoader::load("assets/models/stanford_dragon_pbr/scene.gltf", "stanford_dragon");
	//dragon->transform.scale = glm::vec3(0.07);
	//dragon->transform.position = glm::vec3(1.0f, -2.0f, -3.0f);
	//scene.addObject(dragon);
}

static void dragonScene2(Scene& scene) {
	logger.info("creating dragon debug scene");

	// ADD LIGHTS
	auto light1 = std::make_shared<DirectionalLight>(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(.7, 0, 0));
	auto light2 = std::make_shared<DirectionalLight>(glm::vec3(1.0f, -0.5f, 0.5f), glm::vec3(0, 0, .7));
	auto light3 = std::make_shared<DirectionalLight>(glm::vec3(0.5f, -0.5f, 1.0f), glm::vec3(0, .7, 0));
	auto light4 = std::make_shared<PointLight>(glm::vec3(-4.0, 4.0, -4.0), glm::vec3(1.0));
	scene.addLight(light1);
	scene.addLight(light2);
	scene.addLight(light3);
	scene.addLight(light4);

	// ADD ENTITY
	auto dragon = ModelLoader::load("assets/models/stanford_dragon_pbr/scene.gltf");
	if (!dragon) return;

	// set initial dragon transforms
	dragon->transform.scale = glm::vec3(0.08f, 0.08f, 0.08f);
	//dragon->transform.position = glm::vec3(0, -3, 0);
	//dragon->transform.rotation = glm::vec3(0, 90, 0);

	scene.addObject(dragon);
	scene.camera.setTarget(glm::vec3(0, 4, -3.0));

	// add floor
	auto floor = ModelLoader::load("assets/models/cube.obj");
	if (!floor) return;

	// set initial plane transforms
	floor->transform.scale = glm::vec3(20, 0.02, 20);
	floor->transform.position = glm::vec3(0, -0.01, 0);
	//plane->transform.position = glm::vec3(0, 0, 0);

	scene.addObject(floor);

	// other stuff
	auto sphere = ModelLoader::load("assets/models/sphere.obj");
	if (!sphere) return;
	sphere->transform.position = glm::vec3(3.5 ,4 ,3.5);
	sphere->transform.scale = glm::vec3(1.0);
	scene.addObject(sphere);

	// angels!!
	auto angelBase = ModelLoader::load("assets/models/engel_statue_scan_retopology_gltf/scene.gltf");
	if (!angelBase) return;

	// angel 1
	//auto angel1 = std::make_shared<Object>(*angelBase); // copy
	//angel1->transform.scale = glm::vec3(.4f);
	//angel1->transform.rotation = glm::vec3(90, 0, -90);
	//angel1->transform.position = glm::vec3(-7, 0, -7);
	//scene.addObject(angel1);

	// angel 2
	auto angel2 = std::make_shared<Object>(*angelBase); // copy
	angel2->transform.scale = glm::vec3(.4f);
	angel2->transform.rotation = glm::vec3(90, 0, -42);
	angel2->transform.position = glm::vec3(-1, 0, -7);
	scene.addObject(angel2);
}

static void lionScene(Scene& scene) {
	logger.info("creating lion debug scene");

	// lights
	// RGB :)
	//scene.addLight(std::make_shared<DirectionalLight>(glm::vec3(-1, -1, -1), glm::vec3(0, 1, 0)));
	//scene.addLight(std::make_shared<DirectionalLight>(glm::vec3(1, -1, -1), glm::vec3(1, 0, 0)));
	//scene.addLight(std::make_shared<DirectionalLight>(glm::vec3(-1, -1, 1), glm::vec3(0, 0, 1)));

	//scene.addLight(std::make_shared<DirectionalLight>(glm::vec3(-1, -1, -1), glm::vec3(228, 112, 37)));
	//scene.addLight(std::make_shared<DirectionalLight>(glm::vec3(1, -1, -1), glm::vec3(239, 192, 112)));
	//scene.addLight(std::make_shared<DirectionalLight>(glm::vec3(-1, -1, 1), glm::vec3(255, 245, 182)));

	scene.addLight(std::make_shared<DirectionalLight>(glm::vec3(-1, -1, -1), glm::vec3(242, 238, 228)));

	// add objects
	auto lion = ModelLoader::load("assets/models/lion_crushing_a_serpent/scene.gltf");
	lion->transform.scale = glm::vec3(.8);
	lion->transform.position = glm::vec3(1, 0, -20);
	scene.addObject(lion);

	auto sphere = ModelLoader::load("assets/models/sphere.obj");
	sphere->transform.position = glm::vec3(-7, 6, -5);
	scene.addObject(sphere);

	//auto plane = ModelLoader::load("assets/models/cube.obj");
	//plane->transform.scale = glm::vec3(10, .2, 10);
	//plane->transform.position = glm::vec3(0, -5, 0);
	//scene.addObject(plane);

	scene.camera.setTarget(glm::vec3(0, 2, 0));
	//scene.camera.setTarget(glm::vec3(7, -6, -10));
}