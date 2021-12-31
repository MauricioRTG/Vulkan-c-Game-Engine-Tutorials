#include "lve_model.h"

//std 
#include <cassert>

namespace lve
{
	LveModel::LveModel(LveDevice& device, const std::vector<Vertex>& vertices) : lveDevice{device}
	{
		createVertexBuffers(vertices);
	}//end constructor

	LveModel::~LveModel()
	{
		vkDestroyBuffer(lveDevice.device(), vertexBuffer, nullptr);
		vkFreeMemory(lveDevice.device(), vertexBufferMemory, nullptr);
	}//end destructor

	void LveModel::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		lveDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBuffer,
			vertexBufferMemory);

		//memcpy takes the vertices data and copies it into the host (cpu) mapped memory region
		//because we have this host coherant bit, the host memory will be automatically be flushed
		//to update the device memory. If this was absent we will be required to cal vkFlushMappedMemoryRanges
		//in order for the changes to propagate. 
		void* data;
		vkMapMemory(lveDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(lveDevice.device(), vertexBufferMemory);

	}//end createVertexBuffers

	void LveModel::draw(VkCommandBuffer commandBUffer)
	{
		vkCmdDraw(commandBUffer, vertexCount, 1, 0, 0);
	}//end draw

	void LveModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}//end bind 

	std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}//end getBindingDescriptions

	std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescription(1);

		attributeDescription[0].binding = 0;
		attributeDescription[0].location = 0;
		attributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescription[0].offset = 0;
		return attributeDescription;
		
	}//end getAttributeDescriptions
}//end namespace