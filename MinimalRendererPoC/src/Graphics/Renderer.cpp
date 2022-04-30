#include "MrpocPch.h"
#include "Graphics/Renderer.h"

#include "SceneManagement/Node.h"
#include "SceneManagement/Scene.h"
#include "SceneManagement/Nodes/Model.h"
#include "SceneManagement/Nodes/Camera.h"

#include "Resources/Mesh.h"
#include "Resources/Shader.h"
#include "Resources/Texture.h"
#include "Resources/Material.h"
#include "Resources/ResourceHandler.h"
#include "EventSystem/EventMessenger.h"

#include "spdlog/spdlog.h"
#include <unordered_map>

namespace mrpoc
{
	Renderer::Renderer(ResourceHandler& resourceHandler) : m_activeCameraIndex(0)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		//glEnable(GL_MULTISAMPLE);
		glDepthFunc(GL_LESS);

		SetupFrameBuffers(800,600); // this might need to be altered for when we have a different default screen size. resize event should also take this into account.
		SetupScreenSpaceQuadVAO();
		m_screenShader = resourceHandler.GetShader("resources/Shaders/vs/SampleScenePass.vert");
	
		EventMessenger::GetInstance().ConnectListener<ResizeEventArgs&>("OnWindowResize", &Renderer::OnResize, this);
	}

	Renderer::~Renderer()
	{
		glDeleteFramebuffers(amountOfFrameBuffers,&m_frameBuffers[0]);

		EventMessenger::GetInstance().ConnectListener<ResizeEventArgs&>("OnWindowResize", &Renderer::OnResize, this);
	}

	void Renderer::SetActiveCameraIndex(int aNewPrimaryCameraIndex)
	{
		m_activeCameraIndex = aNewPrimaryCameraIndex;
	}

	void Renderer::Render(Scene& aSceneToRender)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,m_frameBuffers[static_cast<int>(FrameBufferType::ScenePassBuffer)]);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
		glEnable(GL_DEPTH_TEST);
		RenderScenePass(aSceneToRender);

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to on screen buffer
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		RenderFinalResultPass();
	}

	int Renderer::GetActiveCameraIndex() const
	{
		return m_activeCameraIndex;
	}

	void Renderer::ParseNode(std::shared_ptr<Node>& aNodeToParse,
		Scene& scene,
		std::unordered_map<Material*, std::queue<MeshRenderData>>& meshesToRender,
		std::vector<LightData>& lightDataVector,
		std::vector<CameraData>& cameraDataVector)
	{
		switch (aNodeToParse->GetNodeType())
		{
		case MODEL:
		{
			Model& model = scene.GetModelData(aNodeToParse->GetDataIndex());
			for(auto& mesh : model.GetMeshes())
			{
				if(meshesToRender.find(mesh->GetMaterial().get()) == meshesToRender.end())
				{
					meshesToRender.emplace(mesh->GetMaterial().get(), std::queue<MeshRenderData>());
				}

				meshesToRender[mesh->GetMaterial().get()].push(
				{ 
					aNodeToParse->GetWorldTransform(),
					*mesh
				});
			}
			break;
		}
		case LIGHT:
			lightDataVector.push_back(LightData{aNodeToParse->GetWorldTransform(), scene.GetLightData(aNodeToParse->GetDataIndex())});
			break;
		case CAMERA:
			cameraDataVector.push_back(CameraData{aNodeToParse->GetWorldTransform(), scene.GetCameraData(aNodeToParse->GetDataIndex())});
			break;
		default:
			break;
		}

		for (auto& child : aNodeToParse->GetChildren())
		{
			ParseNode(child, scene, meshesToRender, lightDataVector, cameraDataVector);
		}
	}

	void Renderer::RenderScenePass(Scene& sceneToRender)
	{	
		std::unordered_map<Material*, std::queue<MeshRenderData>> meshesToRender;
		std::vector<LightData> lightsToUse;
		std::vector<CameraData> camerasInScene;
		std::shared_ptr<Node>& SceneRoot = sceneToRender.GetRoot();

		if (SceneRoot->GetChildren().empty()) 
		{
			spdlog::critical("Tree not implemented yet, shutting down");
			assert(false);
		}

		// set all struct data
		ParseNode(SceneRoot, sceneToRender, meshesToRender, lightsToUse, camerasInScene);

		CameraData& activeCamera = camerasInScene[m_activeCameraIndex];
		glm::mat4x4 projectionViewMatrix = activeCamera.cameraData.GetProjectionMat() * glm::inverse(activeCamera.worldMatrix);

		static float time = 0;
		time += 0.0004f;

		for (auto& meshesThatUseShader : meshesToRender)
		{
			/*if (meshesThatUseShader.first->DoubleSided)
			{
				glDisable(GL_CULL_FACE);
			}
			else
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
			}*/

			while(meshesThatUseShader.second.size() > 0)
			{				
				MeshRenderData mrd = meshesThatUseShader.second.front();
				//mrd.worldMatrix = glm::rotate(glm::pi<float>() * time, glm::vec3(0, 1, 0)) * mrd.worldMatrix;
				
				meshesThatUseShader.second.pop();

				Shader& meshShader = *mrd.mesh.GetShader();
				Material& material = *meshesThatUseShader.first;
				Mesh& mesh = mrd.mesh;

				if(&meshShader != m_activeShader.get())
				{
				mesh.GetShader()->Activate();
				m_activeShader = mrd.mesh.GetShader();
				}
				mesh.Bind();

				//MVP.
				meshShader.GetParameter("u_vpMat")->SetValue(projectionViewMatrix);
				meshShader.GetParameter("u_modelMat")->SetValue(mrd.worldMatrix);
				meshShader.GetParameter("u_normalMat")->SetValue(glm::transpose(glm::inverse(glm::mat3(mrd.worldMatrix))));

				//Camera.
				meshShader.GetParameter("u_CameraPos")->SetValue(glm::vec3(activeCamera.worldMatrix[3].x, activeCamera.worldMatrix[3].y, activeCamera.worldMatrix[3].z));

				SetLightingProperties(lightsToUse, meshShader, material);
				SetMaterialProperties(meshShader, material);

				mesh.Draw();
				
				mesh.Unbind();
			}
		}
	}

	void Renderer::RenderFinalResultPass()
	{
		m_screenShader.lock()->Activate();
		glBindVertexArray(m_screenQuadVAO);
		glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_colorTextures[static_cast<int>(FrameBufferType::ScenePassBuffer)]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		m_activeShader->Activate();
	}

	void Renderer::SetupFrameBuffers(int ScreenWidth, int ScreenHeight)
	{
		glGenFramebuffers(amountOfFrameBuffers,&m_frameBuffers[0]);
		glGenRenderbuffers(amountOfFrameBuffers, &m_renderBuffers[0]);
		
		for (int i = 0; i < amountOfFrameBuffers; ++i)
		{
			glBindFramebuffer(GL_FRAMEBUFFER,m_frameBuffers[i]);

			glGenTextures(1, &m_colorTextures[i]);
			glBindTexture(GL_TEXTURE_2D, m_colorTextures[i]);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ScreenWidth, ScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glBindTexture(GL_TEXTURE_2D, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTextures[i], 0);

			/*	// use this code instead of the render buffer code if you need to sample from a frame buffer.
				
				glTexImage2D(
				GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0,
				GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
			*/

			glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffers[i]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, ScreenWidth, ScreenHeight);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBuffers[i]);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				spdlog::error("Frame buffer incomplete");
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	void Renderer::OnResize(ResizeEventArgs& e)
	{
		glDeleteFramebuffers(amountOfFrameBuffers, &m_frameBuffers[0]);
		glDeleteRenderbuffers(amountOfFrameBuffers, &m_renderBuffers[0]);

		SetupFrameBuffers(e.Width, e.Height);

		glViewport(0, 0, e.Width, e.Height);
	}

	void Renderer::SetupScreenSpaceQuadVAO()
	{
		float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,

			-1.0f,  1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f
		};

		// screen quad VAO
		glGenVertexArrays(1, &m_screenQuadVAO);
		glGenBuffers(1, &m_screenQuadVBO);
		glBindVertexArray(m_screenQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_screenQuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	}

	void Renderer::SetLightingProperties(std::vector<LightData>& lightDataVector, Shader& shader, Material& material)
	{
		int pointLightCount = 0;
		int spotLightCount = 0;
		bool hasDirectionalLight = false;

		for (auto& light : lightDataVector)
		{
			switch (light.lightData.Type)
			{
			case ELightType::POINT:
			{
				PointLight& pointLight = static_cast<PointLight&>(light.lightData);
				//light.worldMatrix = glm::translate(light.worldMatrix, glm::vec3(sin(time), 0.0f, 0.0f));
				//shader.GetParameter("u_PointLights[" + std::to_string(pointLightCount) + "].Position")->SetValue(glm::vec3(light.worldMatrix[3]));
				shader.GetParameter("u_PointLightPositions[" + std::to_string(pointLightCount) + "].Position")->SetValue(glm::vec3(light.worldMatrix[3]));
				shader.GetParameter("u_PointLights[" + std::to_string(pointLightCount) + "].Intensity")->SetValue(pointLight.Intensity);
				shader.GetParameter("u_PointLights[" + std::to_string(pointLightCount) + "].Color")->SetValue(pointLight.Color);
				shader.GetParameter("u_PointLights[" + std::to_string(pointLightCount) + "].Range")->SetValue(pointLight.Radius);
				++pointLightCount;
				break;
			}
			case ELightType::DIRECTIONAL:
			{
				//lightsToUse[0].worldMatrix = glm::rotate(sin(time) * 8, glm::vec3(1, 0, 1));
				//shader.GetParameter("u_DirectionalLight.Direction")->SetValue(glm::vec3(lightDataVector[0].worldMatrix[2]));
				shader.GetParameter("u_DirectionalLightDir")->SetValue(glm::vec3(lightDataVector[0].worldMatrix[2]));
				shader.GetParameter("u_DirectionalLight.Color")->SetValue(lightDataVector[0].lightData.Color);
				shader.GetParameter("u_DirectionalLight.Intensity")->SetValue(lightDataVector[0].lightData.Intensity * 10);
				hasDirectionalLight = true;
				break;
			}
			case ELightType::SPOT:
				++spotLightCount;
				break;
			default:
				spdlog::error("Tried to use unsupported light format!");
				break;
			}
		}

		shader.GetParameter("u_FlipNormals")->SetValue(material.FlipNormals);
		shader.GetParameter("u_HasDirectionalLight")->SetValue(hasDirectionalLight);
		shader.GetParameter("u_PointLightCount")->SetValue(pointLightCount);
		shader.GetParameter("u_UseBakedAmbientOcclusion")->SetValue(material.HasOcclusionMap);
	}

	void Renderer::SetMaterialProperties(Shader& shader, Material& material)
	{
		// Material properties
		int textureSlot = 0;
		{
			if (!material.AlbedoMap.expired())
			{
				material.AlbedoMap.lock()->Bind(textureSlot);
				shader.GetParameter("u_Material.AlbedoTexture")->SetValue(*material.AlbedoMap.lock());
				shader.GetParameter("u_hasAlbedoTexture")->SetValue(true);

				material.TexCoords[static_cast<int>(ETextureType::Albedo)]; // #TODO: see if we need this and how we are going to use it with other maps.
				textureSlot++;
			}
			else {
				shader.GetParameter("u_hasAlbedoTexture")->SetValue(false);
				shader.GetParameter("u_Material.AlbedoFactor")->SetValue(material.AlbedoFactor);
			}

			if (!material.NormalMap.expired())
			{
				textureSlot = 1;
				material.NormalMap.lock()->Bind(textureSlot);
				shader.GetParameter("u_Material.NormalTexture")->SetValue(*material.NormalMap.lock());
			}

			if (!material.MetalnessRoughnessMap.expired())
			{
				textureSlot = 2;
				material.MetalnessRoughnessMap.lock()->Bind(textureSlot);
				shader.GetParameter("u_Material.AOMROTexture")->SetValue(*material.MetalnessRoughnessMap.lock());
				shader.GetParameter("u_hasRoughnessMetalnessTexture")->SetValue(true);
			}
			else {
				shader.GetParameter("u_hasRoughnessMetalnessTexture")->SetValue(false);
				shader.GetParameter("u_Material.Metalness")->SetValue(material.MetallicFactor);
				shader.GetParameter("u_Material.Roughness")->SetValue(material.RoughnessFactor);
			}

			if (!material.EmissionMap.expired())
			{
				textureSlot = 3;
				material.EmissionMap.lock()->Bind(textureSlot);
				shader.GetParameter("u_Material.EmissionFactor")->SetValue(material.EmissionFactor);
				shader.GetParameter("u_Material.EmissionTexture")->SetValue(*material.EmissionMap.lock());
				shader.GetParameter("u_HasEmissiveTexture")->SetValue(true);
			}
			else
			{
				shader.GetParameter("u_HasEmissiveTexture")->SetValue(false);
			}
		}
	}
}