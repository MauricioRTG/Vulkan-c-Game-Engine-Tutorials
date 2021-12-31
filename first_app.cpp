#include "first_app.h"

//std
#include <stdexcept>
#include <array>

namespace lve
{
	FirstApp::FirstApp()
	{
		loadModels();
		createPipelineLayout();
		createPipeline();
		createCommandBuffers();
	}//constructor

	FirstApp::~FirstApp()
	{
		vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
	}//destructor

	void FirstApp::run() 
	{
		while (!lveWindow.shouldClose())
		{
			glfwPollEvents();
			drawFrame();
		}//end while

		//By calling this function the cpu will block until all gpu operations have completed
		vkDeviceWaitIdle(lveDevice.device());
	}//end run 

	void FirstApp::loadModels()
	{
		std::vector<LveModel::Vertex> vertices{
			{{0.0f, -0.5f}},
			{{0.5f, 0.5f}},
			{{-0.5f, 0.5f}}
		};
		//Initialize the model
		lveModel = std::make_unique<LveModel>(lveDevice, vertices);
	}//end loadModels

	void FirstApp::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipelinelayout");
		}//end if
	}//end createPipelineLayout

	void FirstApp::createPipeline()
	{
		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(
			pipelineConfig,
			lveSwapChain.width(),
			lveSwapChain.height());
		pipelineConfig.renderPass = lveSwapChain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		lvePipeline = std::make_unique<LvePipeline>(
			lveDevice,
			"shaders/simple_shader.vert.spv",
			"shaders/simple_shader.frag.spv",
			pipelineConfig
		);
	}//end createPipeline

	void FirstApp::createCommandBuffers() 
	{
		commandBuffers.resize(lveSwapChain.imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lveDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}//end if 

		//Record draw commands to each buffer
		for (int i = 0; i < commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			
			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to begin recording command buffer!");
			}//end if
			
			 //First command 
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = lveSwapChain.getRenderPass();
			renderPassInfo.framebuffer = lveSwapChain.getFrameBuffer(i);

			//Defines the area where the shader loads and stores will take place
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = lveSwapChain.getSwapChainExtent();

			//Set clear values, corresponds to what we want the initial values of our framebuffer attachments to be cleared to.
			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			//let's record to our command buffer to begin this render pass
			//VK_SUBPASS_CONTENTS_INLINE, signals that the subsequent render pass commands will be directly
			//embedded in the primary command buffer itself, and that no secondary command buffer, will be used.
			//SECONDARY_COMMAND_BUFFERS, alternative is to us VK_SUBPASS_CONTENT_SECONDARY_COMMAND_BUFFERS, signaling that 
			//render pass commands will be executed from secondary command buffers.
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			lvePipeline->bind(commandBuffers[i]);
			//Commands to draw three vertives and only one instance
			lveModel->bind(commandBuffers[i]);
			lveModel->draw(commandBuffers[i]);

			vkCmdEndRenderPass(commandBuffers[i]);
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to record command buffer!");
			}//end if 
		}//end for

	}//end createCommandBuffers
	void FirstApp::drawFrame()
	{
		uint32_t imageIndex;
		//This function fetches the index of the frame we should render to next, also it automatically
		//handles all the cpu and gpu synchronization, surronding double or triple buffering. 
		//The value results determines if the process was successful.
		auto result = lveSwapChain.acquireNextImage(&imageIndex);

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}//end if

		//This function will submit the provided command buffer to our device graphics queue while 
		//handling cpu and gpu synchronization, then the command buffer will be executed, and then the swapchain
		// will present the associated color attachment image view to the display at the appropiate time
		//based on the present mode selected. 
		result = lveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}
	}//end drawFrame
}//end namespace