#include <memory>
#include <glm.hpp>

#include "IO.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "Scene.h"
#include "Primitives/Plane.h"
#include "Primitives/Sphere.h"
#include "Primitives/Box.h"
#include "DialetricTable.h"
#include "JobSystem.h"
#include "Model.h"

//#TODO: Implement nested refractions.
//#TODO: Implement nested refractive intersections - check what happens when a plane intersects with a dialectric.

using namespace CRT;

const int viewPortWidth =  1920;
const int viewPortHeight = 1080;

void CreateMaterials(ResourceManager& resourceManager);
void CreateScene(Scene& scene, ResourceManager& resources);

int main(int argc, char** argv)
{
	std::shared_ptr<JobSystem> jobSystem = std::make_shared<JobSystem>();
	jobSystem->Initialize();

	Scene scene;
	ResourceManager resourceManager;
	resourceManager.Initialize();
	
	CreateMaterials(resourceManager);
	CreateScene(scene, resourceManager);

	//Load the skysphere.
	//std::shared_ptr<Image> hdriSkySphere = IO::LoadImage("res/HDRI/old_hall_4k.hdr");
	//std::shared_ptr<Image> hdriSkySphere = IO::LoadImage("res/HDRI/kiara_4_mid-morning_4k.hdr");

	//Creating the renderer and rendering the image.
	RendererConfig config;
	config.DoFSampleRate = ESampleRate::X1;
	config.AASampleRate = ESampleRate::X4;
	config.EnableShadows = true;
	config.BackGroundColor = glm::vec3(0.0f);

	Renderer renderer;
	renderer.Initialize(jobSystem, viewPortWidth * viewPortHeight, config);
	renderer.SetActiveCamera(0);
	//renderer.SetSkySphere(hdriSkySphere);
	glm::vec3* colorData = nullptr;

#if defined(_DEBUG)
	for(int i = 1; i <= 4; i++)
	{
		colorData = renderer.Render(scene);
		printf(std::string("Finished BVH layer " + std::to_string(i) + ", now image output starting!\n").c_str());
		CRT::IO::StorePNG(std::string("Debug_RenderResult_" + std::to_string(i) + ".png").c_str(), viewPortWidth, viewPortHeight, 3, colorData, true);
		scene.SetBVHDebugLayer(i);
	}
#else
	colorData = renderer.Render(scene);
	//Storing the scene to an image.
	printf("Done processing the image, now image output starting!\n");
	CRT::IO::StorePNG("RenderResult.png", viewPortWidth, viewPortHeight, 3, colorData, true);
#endif
	return 0;
}

void CreateMaterials(ResourceManager& resourceManager)
{
	std::shared_ptr<Material> Dialetric = std::make_shared<Material>();
	Dialetric->AlbedoCoefficient = glm::vec3(0.0f);
	Dialetric->SpecularCoefficient = glm::vec3(0.5f);
	Dialetric->PhongExponent = 100;
	Dialetric->AbsorbanceCoefficient = glm::vec3(0.001f, 2, 2);
	Dialetric->Reflectivity = 0.01f;
	Dialetric->RefractiveIndex = DialetricIndexTable::GetDialetricIndex(EDialetricType::GLASS);
	Dialetric->IsDialetic = true;
	resourceManager.AddMaterial("Dialetric", Dialetric);

	std::shared_ptr<Material> reflectiveGrey = std::make_shared<Material>();
	reflectiveGrey->AlbedoCoefficient = glm::vec3(0.7f);
	reflectiveGrey->SpecularCoefficient = glm::vec3(0.5f);
	reflectiveGrey->PhongExponent = 100;
	reflectiveGrey->IsReflective = true;
	reflectiveGrey->Reflectivity = 1.0f;
	resourceManager.AddMaterial("ReflectiveGrey", reflectiveGrey);

	std::shared_ptr<Material> diffuseGreen = std::make_shared<Material>();
	diffuseGreen->AlbedoCoefficient = glm::vec3(0.0f, 1.0f, 0.0f);
	diffuseGreen->SpecularCoefficient = glm::vec3(0.5f);
	diffuseGreen->PhongExponent = 100;
	resourceManager.AddMaterial("DiffuseGreen", diffuseGreen);

	std::shared_ptr<Material> diffuseGrey = std::make_shared<Material>();
	diffuseGrey->AlbedoCoefficient = glm::vec3(0.5f, 0.5f, 0.5f);
	diffuseGrey->SpecularCoefficient = glm::vec3(0.5f);
	diffuseGrey->PhongExponent = 100;
	resourceManager.AddMaterial("DiffuseGrey", diffuseGrey);

	std::shared_ptr<Material> diffuseRed = std::make_shared<Material>();
	diffuseRed->AlbedoCoefficient = glm::vec3(1.0f, 0.0f, 0.0f);
	diffuseRed->SpecularCoefficient = glm::vec3(0.5f);
	diffuseRed->PhongExponent = 100;
	resourceManager.AddMaterial("DiffuseRed", diffuseRed);
}

void CreateScene(Scene& scene, ResourceManager& resources)
{
	//Create the camera.
	std::shared_ptr<Camera> camera = std::make_shared<Camera>();
	camera->Initialize(viewPortWidth, viewPortHeight, ECameraType::PERSPECTIVE);
	camera->GetTransform().Translate(glm::vec3(-10.0f, 30.0f, -30.0f));
	camera->GetTransform().LookAt(glm::vec3(0.0f, 0.0f, 0.0f));
	camera->SetFieldOfView(60.0f);
	camera->SetAperture(0.001f);
	camera->SetFocalLength(1.0f);

	scene.AddCamera(camera);

	//std::shared_ptr<Plane> groundPlane = std::make_shared<Plane>();
	//groundPlane->Initialize(resources.GetMaterial("ReflectiveGrey"), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1000, 0.001f, 1000));
	//scene.AddPrimitive(groundPlane);

	//std::shared_ptr<Mesh> model = IO::LoadWavefrontFile(resources, "Res/Models/Obj/Stanford/StanfordBunny.obj");
	//std::shared_ptr<Mesh> model = IO::LoadWavefrontFile(resources, "Res/Models/Obj/Stanford/StanfordDragon.obj");
	//std::shared_ptr<Mesh> model = IO::LoadWavefrontFile(resources, "Res/Models/Obj/Stanford/StandfordRGBDragon.obj");
	//std::shared_ptr<Mesh> model = IO::LoadWavefrontFile(resources, "Res/Models/Obj/CornellBox/cornellbox.obj");
	std::shared_ptr<Mesh> model = IO::LoadWavefrontFile(resources, "Res/Models/Obj/PicaPica/PicaPica_Small.obj");

	scene.AddPrimitive(model);

	//std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>();
	//sphere->Initialize(resources.GetMaterial(ResourceManager::GetDefaultMaterialName()), glm::vec3(0.0f, 2.0f, 0.0f));
	//scene.AddPrimitive(sphere);

	//Create the lights.
	std::shared_ptr<PointLight> mainLight = std::make_shared<PointLight>();
	mainLight->Initialize(ELightType::POINT, glm::vec3(0.0f, 10.0f, 0.0f));
	mainLight->SetRange(50.0f);
	mainLight->SetIntensity(0.7f);
	scene.AddLight(mainLight);

	scene.GenerateBVH();
}