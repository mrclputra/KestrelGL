#include "debug.h"
#include "ModelLoader.h"

#include <string>
#include <shader.h>

#include "lights/DirectionalLight.h"
#include "lights/PointLight.h"

// TODO: should I create some kind of scene factory?
//	it would be responsible for instantiating and setting up different scenes

// this is my test scene configuration
void dragonScene(Scene& scene) {
	logger.info("creating debug test scene");

	// ADD LIGHTS
	auto light1 = std::make_shared<DirectionalLight>(glm::vec3(-1.0f, -0.5f, -0.5f), glm::vec3(.7, 0, 0));
	auto light2 = std::make_shared<DirectionalLight>(glm::vec3(1.0f, -0.5f, 0.5f), glm::vec3(0, 0, .7));
	auto light3 = std::make_shared<DirectionalLight>(glm::vec3(0.5f, -0.5f, 1.0f), glm::vec3(0, .7, 0));
	auto light4 = std::make_shared<PointLight>(glm::vec3(-4.0, 4.0, -4.0), glm::vec3(1.0));
	scene.addLight(light1);
	scene.addLight(light2);
	scene.addLight(light3);
	scene.addLight(light4);

	//auto shader = std::make_shared<Shader>(SHADER_DIR "model.vert", SHADER_DIR "model.frag");

	// ADD ENTITY
	auto dragon = ModelLoader::load("assets/models/pbr/stanford_dragon_pbr/scene.gltf");
	if (!dragon) return;

	// set initial dragon transforms
	dragon->transform.scale = glm::vec3(0.08f, 0.08f, 0.08f);
	//dragon->transform.position = glm::vec3(0, -3, 0);
	//dragon->transform.rotation = glm::vec3(0, 90, 0);

	scene.addObject(dragon);
	scene.camera.setTarget(glm::vec3(0, 4, -1.5));

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
	auto angelBase = ModelLoader::load("assets/models/pbr/engel_statue_scan_retopology_gltf/scene.gltf");
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