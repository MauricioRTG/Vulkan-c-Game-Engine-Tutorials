#pragma once

#include "lve_device.h"

#define GLM_FORCE_RADIANS
//Tells glm to expect out depth buffer values to range from 0 to 1
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace lve
{
	//The purpose of this class is to be able to take the vertex data created by or read in a file 
	//on the cpu and then allocate the memory and copy the data over our device gpu so it can be
	//render efficiently. The model manages the vulkan manager and memory objects.
	class LveModel
	{
	public:
		struct Vertex
		{
			glm::vec2 position;

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		LveModel(LveDevice &device, const std::vector<Vertex>& vertices);
		~LveModel();

		LveModel(const LveModel&) = delete;
		LveModel &operator=(const LveModel&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBUffer);

	private:
		void createVertexBuffers(const std::vector<Vertex>& vertices);

		LveDevice& lveDevice; 
		//In vulkan the buffer and its assigned memory are two separate objects.
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;
	};//end class LveDevice
}//end namespace