#include "stdafx.h"
#include "application.h"
#include "vertex.h"
#include "uniform.h"
#include "tools/timer.h"
#include <algorithm> 
#include <fstream>
#include <set>

namespace xengine
{

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const int MAX_FRAMES_IN_FLIGHT = 2;

//======================================================================================================================
Application::Application(uint32_t _width,
						 uint32_t _height)
: window_(std::make_unique<Window>(_width, _height, "Vulkan Engine"))
{}
//======================================================================================================================
bool Application::Init()
{
	// todo: return false instead of exception
	if(!window_->Init())
	{
		return false;
	}

	return InitVulkan();
}
//======================================================================================================================
void Application::Run()
{
	MainLoop();
	Cleanup();
}
//======================================================================================================================
bool Application::InitVulkan()
{
	CreateInstance();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateCommandPool();
	if(!CreateTextureImage())
	{
		return false;
	}
	CreateTextureSampler();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffer();
	CreateSyncObjects();
	return true;
}
//======================================================================================================================
bool Application::CreateInstance()
{
	instance_		= std::make_unique<Instance>();
	bool isCreated	= instance_->Create();
	return isCreated;
}
//======================================================================================================================
bool Application::CreateSurface()
{
	surface_		= std::make_unique<Surface>(*instance_, window_->GetWindow());
	bool isCreated	= surface_->Create();
	return isCreated;
}
//======================================================================================================================
void Application::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance_.get()->GetInstance(), &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance_.get()->GetInstance(), &deviceCount, devices.data());
	for (const auto& device : devices)
	{
		indices_ = FindQueueFamilies(device);
		if (IsDeviceSuitable_(device))
		{
			physicalDevice_ = device;
			break;
		}
	}

	if (physicalDevice_ == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}
//======================================================================================================================
void Application::CreateLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {indices_.graphicsFamily.value(), indices_.presentFamily.value()};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex	= queueFamily;
		queueCreateInfo.queueCount			= 1;
		queueCreateInfo.pQueuePriorities	= &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (instance_.get()->IsValidationLayersEnabled())
	{
		deviceCreateInfo.enabledLayerCount		= static_cast<uint32_t>(instance_.get()->GetValidationLayers().size());
		deviceCreateInfo.ppEnabledLayerNames	= instance_.get()->GetValidationLayers().data();
	}
	else
	{
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &logicalDevice_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(logicalDevice_, indices_.graphicsFamily.value(), 0, &graphicsQueue_);
	vkGetDeviceQueue(logicalDevice_, indices_.presentFamily.value(), 0, &presentQueue_);
}
//======================================================================================================================
void Application::CreateSwapChain()
{
	VkSurfaceCapabilitiesKHR capabilities		= surface_.get()->GetCapabilities(std::ref(physicalDevice_));
	VkSurfaceFormatKHR		surfaceFormat		= ChooseSwapSurfaceFormat(surface_.get()->GetFormats(std::ref(physicalDevice_)));
	VkPresentModeKHR		presentMode			= ChooseSwapPresentMode(surface_.get()->GetPresentModes(std::ref(physicalDevice_)));
	VkExtent2D				extent				= ChooseSwapExtent(capabilities);

	uint32_t				imageCount			= surface_.get()->GetCapabilities(std::ref(physicalDevice_)).minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType			= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface			= surface_.get()->GetSurface();
	createInfo.minImageCount	= imageCount;
	createInfo.imageFormat		= surfaceFormat.format;
	createInfo.imageColorSpace	= surfaceFormat.colorSpace;
	createInfo.imageExtent		= extent;
	createInfo.imageArrayLayers	= 1;
	createInfo.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice_);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode			= VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount	= 2;
		createInfo.pQueueFamilyIndices		= queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode			= VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount	= 0;
		createInfo.pQueueFamilyIndices		= nullptr;
	}

	createInfo.preTransform		= capabilities.currentTransform;
	createInfo.compositeAlpha	= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode		= presentMode;
	createInfo.clipped			= VK_TRUE;
	createInfo.oldSwapchain		= VK_NULL_HANDLE;

	//debug
	std::cout << "Surface: " << surface_ << std::endl;
	std::cout << "Min Image Count: " << createInfo.minImageCount << std::endl;
	std::cout << "Image Format: " << createInfo.imageFormat << std::endl;
	std::cout << "Image Extent: " << createInfo.imageExtent.width << "x" << createInfo.imageExtent.height << std::endl;

	swapChain_ = VK_NULL_HANDLE;
	VkResult result = vkCreateSwapchainKHR(logicalDevice_, &createInfo, nullptr, &swapChain_);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(logicalDevice_, swapChain_, &imageCount, nullptr);
	swapChainImages_.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice_, swapChain_, &imageCount, swapChainImages_.data());

	swapChainImageFormat_	= surfaceFormat.format;
	swapChainExtent_		= extent;
}
//======================================================================================================================
bool Application::CreateImageViews()
{
	swapChainImageViews_.resize(swapChainImages_.size());

	for (size_t i = 0; i < swapChainImages_.size(); ++i)
	{
		VkImageView imageView = Texture::CreateTextureImageView(std::ref(logicalDevice_), swapChainImages_.at(i), VK_FORMAT_R8G8B8A8_SRGB);
		if(imageView == VK_NULL_HANDLE)
		{
			return false;
		}
		swapChainImageViews_[i] = imageView;
	}
	return true;
}
//======================================================================================================================
void Application::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format			= swapChainImageFormat_;
	colorAttachment.samples			= VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment	= 0;
    colorAttachmentRef.layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount	= 1;
    subpass.pColorAttachments		= &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass		= VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass		= 0;
	dependency.srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask	= 0;
	dependency.dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask	= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount	= 1;
	renderPassInfo.pAttachments		= &colorAttachment;
	renderPassInfo.subpassCount		= 1;
	renderPassInfo.pSubpasses		= &subpass;

	if (vkCreateRenderPass(logicalDevice_, &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}
//======================================================================================================================
void Application::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding			= 0;
	uboLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount	= 1;
	uboLayoutBinding.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers	= nullptr; // Optional
	uboLayoutBinding.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding			= 1;
	samplerLayoutBinding.descriptorCount	= 1;
	samplerLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers	= nullptr;
	samplerLayoutBinding.stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount		= static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings		= bindings.data();

	if (vkCreateDescriptorSetLayout(logicalDevice_, &layoutInfo, nullptr, &descriptorSetLayout_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}
//======================================================================================================================
void Application::CreateGraphicsPipeline()
{
	auto vertShaderCode = ReadFile_("../src/shaders/vert.spv");
	auto fragShaderCode = ReadFile_("../src/shaders/frag.spv");

	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage	= VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module	= vertShaderModule;
	vertShaderStageInfo.pName	= "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage	= VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module	= fragShaderModule;
	fragShaderStageInfo.pName	= "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	auto bindingDescription		= Vertex::GetBindingDescription();
	auto attributeDescriptions	= Vertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount	= 1;
	vertexInputInfo.vertexAttributeDescriptionCount	= static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions		= &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions	= attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType						= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable	= VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount	= 1;
	viewportState.scissorCount	= 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable			= VK_FALSE;
	rasterizer.rasterizerDiscardEnable	= VK_FALSE;
	rasterizer.polygonMode				= VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth				= 1.0f;
	rasterizer.cullMode					= VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace				= VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable			= VK_FALSE;
	rasterizer.depthBiasConstantFactor	= 0.0f; // Optional
	rasterizer.depthBiasClamp			= 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor		= 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable	= VK_FALSE;
	multisampling.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading		= 1.0f; // Optional
	multisampling.pSampleMask			= nullptr; // Optional
	multisampling.alphaToCoverageEnable	= VK_FALSE; // Optional
	multisampling.alphaToOneEnable		= VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask			= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable			= VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor	= VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor	= VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp			= VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor	= VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor	= VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp			= VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable		= VK_FALSE;
	colorBlending.logicOp			= VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount	= 1;
	colorBlending.pAttachments		= &colorBlendAttachment;
	colorBlending.blendConstants[0]	= 0.0f; // Optional
	colorBlending.blendConstants[1]	= 0.0f; // Optional
	colorBlending.blendConstants[2]	= 0.0f; // Optional
	colorBlending.blendConstants[3]	= 0.0f; // Optional

	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount	= static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates		= dynamicStates.data();

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount			= 1; // Optional
	pipelineLayoutInfo.pSetLayouts				= &descriptorSetLayout_; // Optional
	pipelineLayoutInfo.pushConstantRangeCount	= 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges		= nullptr; // Optional

	if (vkCreatePipelineLayout(logicalDevice_, &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount				= 2;
	pipelineInfo.pStages				= shaderStages;
	pipelineInfo.pVertexInputState		= &vertexInputInfo;
	pipelineInfo.pInputAssemblyState	= &inputAssembly;
	pipelineInfo.pViewportState			= &viewportState;
	pipelineInfo.pRasterizationState	= &rasterizer;
	pipelineInfo.pMultisampleState		= &multisampling;
	pipelineInfo.pColorBlendState		= &colorBlending;
	pipelineInfo.pDynamicState			= &dynamicState;
	pipelineInfo.layout					= pipelineLayout_;
	pipelineInfo.renderPass				= renderPass_;
	pipelineInfo.subpass				= 0;
	pipelineInfo.basePipelineHandle		= VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(logicalDevice_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(logicalDevice_, fragShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice_, vertShaderModule, nullptr);
}
//======================================================================================================================
void Application::CreateFramebuffers()
{
	swapChainFramebuffers_.resize(swapChainImageViews_.size());
	for (size_t i = 0; i < swapChainImageViews_.size(); ++i)
	{
		VkImageView attachments[] =
		{
			swapChainImageViews_[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= renderPass_;
		framebufferInfo.attachmentCount	= 1;
		framebufferInfo.pAttachments	= attachments;
		framebufferInfo.width			= swapChainExtent_.width;
		framebufferInfo.height			= swapChainExtent_.height;
		framebufferInfo.layers			= 1;

		if (vkCreateFramebuffer(logicalDevice_, &framebufferInfo, nullptr, &swapChainFramebuffers_[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}
//======================================================================================================================
void Application::CreateCommandPool()
{
	commandPool_ = std::make_unique<CommandPool>(std::ref(logicalDevice_),
												 std::ref(physicalDevice_),
												 indices_);
	commandPool_.get()->Create();
}
//======================================================================================================================
bool Application::CreateTextureImage()
{
	texture_ = std::make_unique<Texture>(std::ref(logicalDevice_),
										 std::ref(physicalDevice_),
										 indices_);
	if(!texture_.get()->Create("../src/textures/test.png"))
	{
		return false;
	}
	texture_.get()->TransitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB,
										  VK_IMAGE_LAYOUT_UNDEFINED,
										  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
										  commandPool_.get(),
										  graphicsQueue_);
	texture_.get()->CopyBufferToImage(commandPool_.get(), graphicsQueue_);

	// To be able to start sampling from the texture image in the shader
	texture_.get()->TransitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB,
										  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
										  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
										  commandPool_.get(),
										  graphicsQueue_);
	textureImageView_ = Texture::CreateTextureImageView(std::ref(logicalDevice_), texture_.get()->GetImage(), VK_FORMAT_R8G8B8A8_SRGB);
	return true;
}
//======================================================================================================================
bool Application::CreateTextureSampler()
{
	textureSampler_ = Texture::CreateTextureSampler(std::ref(logicalDevice_),
													std::ref(physicalDevice_));
	return textureSampler_ != VK_NULL_HANDLE;
}
//======================================================================================================================
void Application::CreateVertexBuffer()
{
	Buffer stagingBuffer(sizeof(vertices[0]) * vertices.size(), std::ref(logicalDevice_));
	stagingBuffer.CreateBuffer(physicalDevice_,
							   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(logicalDevice_, stagingBuffer.GetBufferMemory(), 0, stagingBuffer.GetSize(), 0, &data);
	memcpy(data, vertices.data(), (size_t)stagingBuffer.GetSize());
	vkUnmapMemory(logicalDevice_, stagingBuffer.GetBufferMemory());

	vertexBuffer_ = std::make_unique<Buffer>(sizeof(vertices[0]) * vertices.size(), std::ref(logicalDevice_));
	vertexBuffer_.get()->CreateBuffer(physicalDevice_,
									  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
									  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	CommandBuffer commandBuffer(std::ref(logicalDevice_), std::ref(physicalDevice_), indices_);
	commandBuffer.Create(commandPool_.get());
	commandBuffer.CopyBuffer(stagingBuffer.GetBuffer(), vertexBuffer_->GetBuffer(), vertexBuffer_->GetSize());
	commandBuffer.SubmitAndWaitIdle(graphicsQueue_);
}
//======================================================================================================================
void Application::CreateIndexBuffer()
{
	Buffer stagingBuffer(sizeof(vertices[0]) * vertices.size(), std::ref(logicalDevice_));
	stagingBuffer.CreateBuffer(physicalDevice_,
							   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(logicalDevice_, stagingBuffer.GetBufferMemory(), 0, stagingBuffer.GetSize(), 0, &data);
	memcpy(data, indices.data(), (size_t)stagingBuffer.GetSize());
	vkUnmapMemory(logicalDevice_, stagingBuffer.GetBufferMemory());

	indexBuffer_ = std::make_unique<Buffer>(sizeof(vertices[0]) * vertices.size(), std::ref(logicalDevice_));
	indexBuffer_.get()->CreateBuffer(physicalDevice_,
									 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
									 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	CommandBuffer commandBuffer(std::ref(logicalDevice_), std::ref(physicalDevice_), indices_);
	commandBuffer.Create(commandPool_.get());
	commandBuffer.CopyBuffer(stagingBuffer.GetBuffer(), indexBuffer_->GetBuffer(), indexBuffer_->GetSize());
	commandBuffer.SubmitAndWaitIdle(graphicsQueue_);
}
//======================================================================================================================
void Application::CreateUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers_.reserve(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped_.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		uniformBuffers_.emplace_back(std::make_unique<Buffer>(bufferSize, std::ref(logicalDevice_)));
		uniformBuffers_[i].get()->CreateBuffer(physicalDevice_,
											   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
											   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		vkMapMemory(logicalDevice_, uniformBuffers_[i].get()->GetBufferMemory(), 0, bufferSize, 0, &uniformBuffersMapped_[i]);
	}
}
//======================================================================================================================
void Application::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount	= static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	poolSizes[1].type				= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount	= static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount	= static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes		= poolSizes.data();
	poolInfo.maxSets		= static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(logicalDevice_, &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}
//======================================================================================================================
void Application::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout>	layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout_);
	VkDescriptorSetAllocateInfo			allocInfo{};
	allocInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool		= descriptorPool_;
	allocInfo.descriptorSetCount	= static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts			= layouts.data();
	descriptorSets_.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(logicalDevice_, &allocInfo, descriptorSets_.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers_[i].get()->GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView		= textureImageView_;
		imageInfo.sampler		= textureSampler_;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet			= descriptorSets_[i];
		descriptorWrites[0].dstBinding		= 0;
		descriptorWrites[0].dstArrayElement	= 0;
		descriptorWrites[0].descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount	= 1;
		descriptorWrites[0].pBufferInfo		= &bufferInfo;

		descriptorWrites[1].sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet			= descriptorSets_[i];
		descriptorWrites[1].dstBinding		= 1;
		descriptorWrites[1].dstArrayElement	= 0;
		descriptorWrites[1].descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount	= 1;
		descriptorWrites[1].pImageInfo		= &imageInfo;

		vkUpdateDescriptorSets(logicalDevice_, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}
//======================================================================================================================
bool Application::CreateCommandBuffer()
{
	commandBuffers_.reserve(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		commandBuffers_.emplace_back(std::make_unique<CommandBuffer>(std::ref(logicalDevice_),
																	 std::ref(physicalDevice_),
																	 indices_));
		if(!commandBuffers_[i].get()->Create(commandPool_.get()))
		{
			return false;
		}
	}
	return true;
}
//======================================================================================================================
void Application::CreateSyncObjects()
{
	imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(logicalDevice_, &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]) != VK_SUCCESS
			|| vkCreateSemaphore(logicalDevice_, &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS
			|| vkCreateFence(logicalDevice_, &fenceInfo, nullptr, &inFlightFences_[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create semaphores!");
		}
	}
}
//======================================================================================================================
void Application::RecreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window_.get()->GetWindow(), &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window_.get()->GetWindow(), &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(logicalDevice_);

	CleanupSwapChain();

	CreateSwapChain();
	CreateImageViews();
	CreateFramebuffers();
}
//======================================================================================================================
VkShaderModule  Application::CreateShaderModule(const std::vector<char>& _code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize	= _code.size();
	createInfo.pCode	= reinterpret_cast<const uint32_t*>(_code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}
//======================================================================================================================
void Application::MainLoop()
{
	while (!glfwWindowShouldClose(window_.get()->GetWindow()))
	{
		glfwPollEvents();
		DrawFrame();
	}

	vkDeviceWaitIdle(logicalDevice_);
}
//======================================================================================================================
void Application::DrawFrame()
{
	vkWaitForFences(logicalDevice_, 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(logicalDevice_, swapChain_, UINT64_MAX, imageAvailableSemaphores_[currentFrame_], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	UpdateUniformBuffer(currentFrame_);

	vkResetFences(logicalDevice_, 1, &inFlightFences_[currentFrame_]);
	
	vkResetCommandBuffer(commandBuffers_[currentFrame_].get()->GetBuffer(), /*VkCommandBufferResetFlagBits*/ 0);
	RecordCommandBuffer(commandBuffers_[currentFrame_].get()->GetBuffer(), imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[]		= {imageAvailableSemaphores_[currentFrame_]};
	VkPipelineStageFlags waitStages[]	= {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount		= 1;
	submitInfo.pWaitSemaphores			= waitSemaphores;
	submitInfo.pWaitDstStageMask		= waitStages;
	submitInfo.commandBufferCount		= 1;
	submitInfo.pCommandBuffers			= &commandBuffers_[currentFrame_].get()->GetBuffer();

	VkSemaphore signalSemaphores[]		= {renderFinishedSemaphores_[currentFrame_]};
	submitInfo.signalSemaphoreCount		= 1;
	submitInfo.pSignalSemaphores		= signalSemaphores;

	if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, inFlightFences_[currentFrame_]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount	= 1;
	presentInfo.pWaitSemaphores		= signalSemaphores;

	VkSwapchainKHR swapChains[]		= {swapChain_};
	presentInfo.swapchainCount		= 1;
	presentInfo.pSwapchains			= swapChains;
	presentInfo.pImageIndices		= &imageIndex;
	presentInfo.pResults			= nullptr;

	result = vkQueuePresentKHR(presentQueue_, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window_.get()->FramebufferResized())
	{
		window_.get()->FramebufferResizedReset();
		RecreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}
//======================================================================================================================
void Application::Cleanup()
{
	CleanupSwapChain();

	vkDestroySampler(logicalDevice_, textureSampler_, nullptr);
	vkDestroyDescriptorPool(logicalDevice_, descriptorPool_, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice_, descriptorSetLayout_, nullptr);

	vkDestroyPipelineLayout(logicalDevice_, pipelineLayout_, nullptr);
	vkDestroyPipelineLayout(logicalDevice_, pipelineLayout_, nullptr);
	vkDestroyRenderPass(logicalDevice_, renderPass_, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(logicalDevice_,	renderFinishedSemaphores_[i],	nullptr);
		vkDestroySemaphore(logicalDevice_,	imageAvailableSemaphores_[i],	nullptr);
		vkDestroyFence(logicalDevice_,		inFlightFences_[i],				nullptr);
	}

	uniformBuffers_.clear();
	vertexBuffer_.reset();
	indexBuffer_.reset();
	commandPool_.reset();
	texture_.reset(); //test

	vkDestroyDevice(logicalDevice_, nullptr);

	vkDestroySurfaceKHR(instance_.get()->GetInstance(), surface_.get()->GetSurface(), nullptr);
	surface_.reset();
	instance_.reset();
}
//======================================================================================================================
void Application::CleanupSwapChain()
{
	for (auto framebuffer : swapChainFramebuffers_)
	{
		vkDestroyFramebuffer(logicalDevice_, framebuffer, nullptr);
	}

	for (auto imageView : swapChainImageViews_)
	{
		vkDestroyImageView(logicalDevice_, imageView, nullptr);
	}

	vkDestroySwapchainKHR(logicalDevice_, swapChain_, nullptr);
}
//======================================================================================================================
void Application::UpdateUniformBuffer(uint32_t _currentImage)
{
	static auto startTime	= std::chrono::high_resolution_clock::now();
	auto currentTime		= std::chrono::high_resolution_clock::now();
	float time				= std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	//std::cout << "time: " << time << "\n";
	//Timer timer;
	UniformBufferObject ubo{};
	ubo.model_ = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view_ = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj_ = glm::perspective(glm::radians(45.0f), swapChainExtent_.width / (float) swapChainExtent_.height, 0.1f, 10.0f);
	ubo.proj_[1][1] *= -1;

	memcpy(uniformBuffersMapped_[_currentImage], &ubo, sizeof(ubo));
}
//======================================================================================================================
void Application::RecordCommandBuffer(VkCommandBuffer	_commandBuffer,
									  uint32_t			_imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags				= 0; // Optional
	beginInfo.pInheritanceInfo	= nullptr; // Optional

	if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass			= renderPass_;
	renderPassInfo.framebuffer			= swapChainFramebuffers_[_imageIndex];
	renderPassInfo.renderArea.offset	= {0, 0};
	renderPassInfo.renderArea.extent	= swapChainExtent_;
	VkClearValue clearColor				= {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	renderPassInfo.clearValueCount		= 1;
	renderPassInfo.pClearValues			= &clearColor;
	vkCmdBeginRenderPass(_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) swapChainExtent_.width;
	viewport.height = (float) swapChainExtent_.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent_;
	vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);

	VkBuffer vertexBuffers[]	= {vertexBuffer_.get()->GetBuffer()};
	VkDeviceSize offsets[]		= {0};

	vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(_commandBuffer, indexBuffer_.get()->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);

	vkCmdBindDescriptorSets(_commandBuffer,
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							pipelineLayout_,
							0,
							1,
							&descriptorSets_[currentFrame_],
							0,
							nullptr);
	vkCmdDrawIndexed(_commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(_commandBuffer);

	if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}
//======================================================================================================================
QueueFamilyIndices Application::FindQueueFamilies(VkPhysicalDevice _device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(_device, i, surface_.get()->GetSurface(), &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		++i;
	}

	return indices;
}
//======================================================================================================================
std::vector<char> Application::ReadFile_(const std::string& _filename)
{
	std::ifstream file(_filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}
//======================================================================================================================
VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& _availableFormats)
{
	for (const auto& availableFormat : _availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return _availableFormats[0];
}
//======================================================================================================================
VkPresentModeKHR Application::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& _availablePresentModes)
{
	for (const auto& availablePresentMode : _availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}
//======================================================================================================================
VkExtent2D Application::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& _capabilities)
{
	if (_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return _capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window_.get()->GetWindow(), &width, &height);

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width	= std::clamp(actualExtent.width, _capabilities.minImageExtent.width, _capabilities.maxImageExtent.width);
		actualExtent.height	= std::clamp(actualExtent.height, _capabilities.minImageExtent.height, _capabilities.maxImageExtent.height);

		return actualExtent;
	}
}
//======================================================================================================================
bool Application::IsDeviceSuitable_(VkPhysicalDevice _device)
{
	bool extensionsSupported	= CheckDeviceExtensionSupport_(_device);

	bool swapChainAdequate		= false;
	if (extensionsSupported)
	{
		std::vector<VkSurfaceFormatKHR>	formats			= surface_.get()->GetFormats(_device);
		std::vector<VkPresentModeKHR>	presentModes	= surface_.get()->GetPresentModes(_device);
		swapChainAdequate = !formats.empty() && !presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(_device, &supportedFeatures);

	return indices_.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}
//======================================================================================================================
bool Application::CheckDeviceExtensionSupport_(VkPhysicalDevice _device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		std::cout << "Available extension: " << extension.extensionName << "\n";
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

}