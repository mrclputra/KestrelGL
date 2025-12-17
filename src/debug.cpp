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
	auto light1 = std::make_shared<DirectionalLight>(glm::vec3(-1.0f, 0.2f, -0.5f), glm::vec3(.5, 0, 0));
	auto light2 = std::make_shared<DirectionalLight>(glm::vec3(1.0f, 0.2f, 0.5f), glm::vec3(0, 0, .5));
	auto light3 = std::make_shared<DirectionalLight>(glm::vec3(0.5f, 1.2f, 1.0f), glm::vec3(0, .5, 0));
	auto light4 = std::make_shared<PointLight>(glm::vec3(-3.2, 1.0, -3.2), glm::vec3(1.0));
	scene.addLight(light1);
	scene.addLight(light2);
	scene.addLight(light3);
	scene.addLight(light4);

	auto shader = std::make_shared<Shader>(SHADER_DIR "model.vert", SHADER_DIR "model.frag");

	// ADD ENTITY
	auto dragon = ModelLoader::load("assets/models/pbr/stanford_dragon_pbr/scene.gltf");
	dragon->shader = shader;
	if (!dragon) return;

	// set initial dragon transforms
	dragon->transform.scale = glm::vec3(0.08f, 0.08f, 0.08f);
	//dragon->transform.position = glm::vec3(0, -3, 0);
	//dragon->transform.rotation = glm::vec3(0, 90, 0);

	scene.addObject(dragon);
	scene.camera.setTarget(glm::vec3(0, 3, 0));

	// add floor
	auto plane = ModelLoader::load("assets/models/plane.obj");
	plane->shader = shader;
	if (!plane) return;

	// set initial plane transforms
	plane->transform.scale = glm::vec3(20, 1, 20);
	//plane->transform.position = glm::vec3(0, 0, 0);

	scene.addObject(plane);

	// other stuff
	auto sphere = ModelLoader::load("assets/models/sphere.obj");
	sphere->shader = shader;
	if (!sphere) return;

	sphere->transform.position = glm::vec3(5,2,5);
	scene.addObject(sphere);
}