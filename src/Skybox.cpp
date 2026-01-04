#include "Skybox.h"
#include <stb_image.h>

Skybox::Skybox() : m_CubemapID(0), m_SkyboxVAO(0), m_SkyboxVBO(0) {
	setupGeometry();
	m_SkyboxShader = std::make_shared<Shader>(SHADER_DIR "skybox.vert", SHADER_DIR "skybox.frag");
	m_EquiToCubeShader = std::make_shared<Shader>(SHADER_DIR "equi_to_cube.vert", SHADER_DIR "equi_to_cube.frag");
	m_PrefilterShader = std::make_shared<Shader>(SHADER_DIR "prefilter.vert", SHADER_DIR "prefilter.frag");

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// TODO: move this out of the constructor
	load("assets/skybox/artist_workshop_4k.hdr");
}

Skybox::~Skybox() {
	glDeleteVertexArrays(1, &m_SkyboxVAO);
	glDeleteBuffers(1, &m_SkyboxVBO);
	glDeleteTextures(1, &m_CubemapID);
}

void Skybox::load(const std::string& path) {
	m_CubemapID = convertHDRItoCubemap(path);
	glFinish();

	// may need to cleanup the old data?
	// idk, we'll see how it pans out

	computeIrradiance();
	computePrefilterMap();
}

unsigned int Skybox::convertHDRItoCubemap(const std::string& path) {
	// load HDR image
	int width, height, nrChannels;
	float* data = stbi_loadf(path.c_str(), &width, &height, &nrChannels, 0);
	unsigned int hdrTexture = 0;

	if (data) {
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		logger.error("Failed to load HDR: " + path);
		return 0;
	}

	// setup framebuffers and cubemap texture
	unsigned int captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	unsigned int envCubemap;
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 1024, 1024, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// setup matrices for the faces
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] = {
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),	// right
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),	// left
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),	// up
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),	// down
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),	// front
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))	// back
	};

	// hdr to cubemap
	m_EquiToCubeShader->use();
	m_EquiToCubeShader->setInt("equirectangularMap", 0);
	m_EquiToCubeShader->setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	// backup current viewport
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glViewport(0, 0, 1024, 1024);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; i++) {
		m_EquiToCubeShader->setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(m_SkyboxVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	// cleanup
	glDeleteTextures(1, &hdrTexture);
	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);

	return envCubemap;
}

void Skybox::draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos) {
	glDepthFunc(GL_LEQUAL); // this is important, pass if depth is 1.0
	m_SkyboxShader->use();

	// remove translation :)
	glm::mat4 s_view = glm::mat4(glm::mat3(view));
	
	m_SkyboxShader->setMat4("view", s_view);
	m_SkyboxShader->setMat4("projection", projection);

	glBindVertexArray(m_SkyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapID);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthFunc(GL_LESS);
}

void Skybox::setupGeometry() {
	float vertices[] = {
		-1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
	};

	glGenVertexArrays(1, &m_SkyboxVAO);
	glGenBuffers(1, &m_SkyboxVBO);
	glBindVertexArray(m_SkyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_SkyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void Skybox::computeIrradiance() {
	shCoefficients.assign(9, glm::vec3(0.0f));

	// read back the cubemap faces to the CPU to calculate
	// todo: normally, this should be done via compute shader
	// ill figure this out in the future when I get to it :)

	int width = 1024;
	std::vector<float> data(width * width * 3);
	float totalWeight = 0.0f;

	for (unsigned int face = 0; face < 6; ++face) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapID);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB, GL_FLOAT, data.data());

		for (int y = 0; y < width; ++y) {
			for (int x = 0; x < width; ++x) {
				// get direction from face UV (yes really)
				// https://en.wikipedia.org/wiki/Cube_mapping
				float u = (x + 0.5f) / width * 2.0f - 1.0f;
				float v = (y + 0.5f) / width * 2.0f - 1.0f;

				glm::vec3 dir;
				if (face == 0) dir = { 1, -v, -u };			// +X
				else if (face == 1) dir = { -1, -v, u };	// -X
				else if (face == 2) dir = { u, 1, v };		// +Y
				else if (face == 3) dir = { u, -1, -v };	// -Y
				else if (face == 4) dir = { u, -v, 1 };		// +Z
				else if (face == 5) dir = { -u, -v, -1 };	// -Z
				dir = glm::normalize(dir);

				// solid angle weight
				// https://en.wikipedia.org/wiki/Solid_angle
				float diff = 1.0f + u * u + v * v;
				float weight = 4.0f / (sqrt(diff) * diff);

				glm::vec3 texel = glm::vec3(data[(y * width + x) * 3 + 0],
					data[(y * width + x) * 3 + 1],
					data[(y * width + x) * 3 + 2]);

				texel = glm::clamp(texel, glm::vec3(0.0f), glm::vec3(1000.0f));

				// project onto SH basis
				float sh[9];
				sh[0] = 0.282095f;
				sh[1] = 0.488603f * dir.y;
				sh[2] = 0.488603f * dir.z;
				sh[3] = 0.488603f * dir.x;
				sh[4] = 1.092548f * dir.x * dir.y;
				sh[5] = 1.092548f * dir.y * dir.z;
				sh[6] = 0.315392f * (3.0f * dir.z * dir.z - 1.0f);
				sh[7] = 1.092548f * dir.x * dir.z;
				sh[8] = 0.546274f * (dir.x * dir.x - dir.y * dir.y);

				for (int i = 0; i < 9; ++i) {
					shCoefficients[i] += texel * sh[i] * weight;
				}
				totalWeight += weight;
			}
		}
	}

	// normalize, this is necessary
	for (int i = 0; i < 9; ++i) {
		shCoefficients[i] *= (4.0f * 3.14159f) / totalWeight;
	}
}

void Skybox::computePrefilterMap() {
	// create cubemap that will store prefiltered mipmaps
	// -> allocate cubemap
	// -> gl_rgb16f
	// -> generate multiple mipmap levels, glTexImage2D in a loop
	// -> set filtering
	//		-> gl linear mipmap linear
	//		-> gl linear
	//		-> gl clamp to edge

	// create cubemap
	glGenTextures(1, &m_PrefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilterMap);

	for (unsigned int i = 0; i < 6; ++i) {
		// faces of the cubemap
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// setup framebuffer and renderbuffer
	// -> fbo for rendering each face of the cube
	// -> depth RBO
	// -> bind FBO and attach RBO for depth

	unsigned int captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	// todo: this piece of code is used multiple times in different functions
	//	I should move it and similar functions to a dedicated utilities file or even a class
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] = {
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};


	// use the prefilter shader
	// bind cubemap
	// set shader uniforms
	//		-> environmentMap (reuse from the skybox albedo)
	//		-> roughness
	// activate

	m_PrefilterShader->use();
	m_PrefilterShader->setMat4("projection", captureProjection);
	
	// we want to generate the mipmaps using the existing albedo map
	// as such, this function should only be called AFTER that is initialized
	// same applies for the irradiance map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_CubemapID);

	m_PrefilterShader->setInt("environmentMap", 0);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 9;

	// render each mip level
	// use viewport to mip resolution

	// after all faces and mips are rendered,
	//		-> generate mipmaps if using glteximage2d
	//		-> unbind framebuffer

	// backup viewport
	// todo: figure out how to use multiple viewports, instead of reusing the default one
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
		// resize viewport to the mip level
		unsigned int mipWidth = static_cast<unsigned int>(512 * std::pow(0.5, mip));
		unsigned int mipHeight = static_cast<unsigned int>(512 * std::pow(0.5, mip));

		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		// roughness level of the mip map is defined here
		// we want to scale it based on the number of mip levels available
		float roughness = (float)mip / (float)(maxMipLevels - 1);
		m_PrefilterShader->setFloat("roughness", roughness);

		for (unsigned int i = 0; i < 6; ++i) {
			m_PrefilterShader->setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_PrefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glBindVertexArray(m_SkyboxVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	// cleanup
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);
}