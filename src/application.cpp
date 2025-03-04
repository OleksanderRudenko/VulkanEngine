#include "stdafx.h"
#include "application.h"
#include "vertex.h"
#include "uniform.h"
#include "tools/timer.h"
#include <algorithm> 
#include <fstream>
#include <set>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const int MAX_FRAMES_IN_FLIGHT = 2;

//======================================================================================================================
VkResult CreateDebugUtilsMessengerEXT(VkInstance								_instance,
									  const VkDebugUtilsMessengerCreateInfoEXT*	_pCreateInfo,
									  const VkAllocationCallbacks*				_pAllocator,
									  VkDebugUtilsMessengerEXT*					_pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(_instance, _pCreateInfo, _pAllocator, _pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
//======================================================================================================================
void DestroyDebugUtilsMessengerEXT(VkInstance					_instance,
								   VkDebugUtilsMessengerEXT		_debugMessenger,
								   const VkAllocationCallbacks*	_pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(_instance,
																			"vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(_instance, _debugMessenger, _pAllocator);
	}
}
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

	InitVulkan();

	return true;
}
//======================================================================================================================
void Application::Run()
{
	MainLoop();
	Cleanup();
}
//======================================================================================================================
void Application::InitVulkan()
{
	CreateInstance();
	SetupDebugMessenger();
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
	CreateTextureImage();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateUniformBuffers();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffer();
	CreateSyncObjects();
}
//======================================================================================================================
void Application::CreateInstance()
{
	if (enableValidationLayers && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName	= "Hello Triangle";
	appInfo.applicationVersion	= VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName			= "No Engine";
	appInfo.engineVersion		= VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion			= VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType			= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo	= &appInfo;

	auto extensions						= GetRequiredExtensions_();
	createInfo.enabledExtensionCount	= static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames	= extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo_(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount	= 0;
		createInfo.pNext				= nullptr;
	}

	if(vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}
//======================================================================================================================
void Application::CreateSurface()
{
	if (glfwCreateWindowSurface(instance_, window_.get()->GetWindow(), nullptr, &surface_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}
//======================================================================================================================
void Application::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
	for (const auto& device : devices)
	{
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
	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice_);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

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

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers)
	{
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &logicalDevice_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(logicalDevice_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
	vkGetDeviceQueue(logicalDevice_, indices.presentFamily.value(), 0, &presentQueue_);
}
//======================================================================================================================
void Application::CreateSwapChain()
{
	SwapChainSupportDetails	swapChainSupport	= QuerySwapChainSupport(physicalDevice_);

	VkSurfaceFormatKHR		surfaceFormat		= ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR		presentMode			= ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D				extent				= ChooseSwapExtent(swapChainSupport.capabilities);

	uint32_t				imageCount			= swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType			= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface			= surface_;
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

	createInfo.preTransform		= swapChainSupport.capabilities.currentTransform;
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
void Application::CreateImageViews()
{
	swapChainImageViews_.resize(swapChainImages_.size());

	for (size_t i = 0; i < swapChainImages_.size(); ++i)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType							= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image							= swapChainImages_[i];
		createInfo.viewType							= VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format							= swapChainImageFormat_;
		createInfo.components.r						= VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g						= VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b						= VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a						= VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel	= 0;
		createInfo.subresourceRange.levelCount		= 1;
		createInfo.subresourceRange.baseArrayLayer	= 0;
		createInfo.subresourceRange.layerCount		= 1;

		if (vkCreateImageView(logicalDevice_, &createInfo, nullptr, &swapChainImageViews_[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image views!");
		}
	}
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

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount		= 1;
	layoutInfo.pBindings		= &uboLayoutBinding;

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
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice_);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex	= queueFamilyIndices.graphicsFamily.value();
	if (vkCreateCommandPool(logicalDevice_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}
//======================================================================================================================
void Application::CreateTextureImage()
{
	
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
	CopyBuffer(stagingBuffer.GetBuffer(), vertexBuffer_.get()->GetBuffer(), vertexBuffer_.get()->GetSize());

	vkDestroyBuffer(logicalDevice_, stagingBuffer.GetBuffer(), nullptr);
	vkFreeMemory(logicalDevice_, stagingBuffer.GetBufferMemory(), nullptr);
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
	CopyBuffer(stagingBuffer.GetBuffer(), indexBuffer_.get()->GetBuffer(), indexBuffer_.get()->GetSize());

	vkDestroyBuffer(logicalDevice_, stagingBuffer.GetBuffer(), nullptr);
	vkFreeMemory(logicalDevice_, stagingBuffer.GetBufferMemory(), nullptr);
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
	VkDescriptorPoolSize poolSize{};
	poolSize.type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount	= static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount	= 1;
	poolInfo.pPoolSizes		= &poolSize;
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

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSets_[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(logicalDevice_, 1, &descriptorWrite, 0, nullptr);
	}
}
//======================================================================================================================
void Application::CreateCommandBuffer()
{
	commandBuffers_.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool			= commandPool_;
	allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount	= (uint32_t) commandBuffers_.size();

	if (vkAllocateCommandBuffers(logicalDevice_, &allocInfo, commandBuffers_.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
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
//void Application::CreateBuffer(VkDeviceSize				_size,
//							   VkBufferUsageFlags		_usage,
//							   VkMemoryPropertyFlags	_properties,
//							   VkBuffer&				_buffer,
//							   VkDeviceMemory&			_bufferMemory)
//{
//	VkBufferCreateInfo bufferInfo{};
//	bufferInfo.sType		= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	bufferInfo.size			= _size;
//	bufferInfo.usage		= _usage;
//	bufferInfo.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;
//
//	if (vkCreateBuffer(logicalDevice_, &bufferInfo, nullptr, &_buffer) != VK_SUCCESS)
//	{
//		throw std::runtime_error("failed to create buffer!");
//	}
//
//	VkMemoryRequirements memRequirements;
//	vkGetBufferMemoryRequirements(logicalDevice_, _buffer, &memRequirements);
//
//	VkMemoryAllocateInfo allocInfo{};
//	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//	allocInfo.allocationSize = memRequirements.size;
//	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, _properties);
//
//	if (vkAllocateMemory(logicalDevice_, &allocInfo, nullptr, &_bufferMemory) != VK_SUCCESS)
//	{
//		throw std::runtime_error("failed to allocate buffer memory!");
//	}
//
//	vkBindBufferMemory(logicalDevice_, _buffer, _bufferMemory, 0);
//}
//======================================================================================================================
void Application::CopyBuffer(VkBuffer		_srcBuffer,
							 VkBuffer		_dstBuffer,
							 VkDeviceSize	_size)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool			= commandPool_;
	allocInfo.commandBufferCount	= 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(logicalDevice_, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.size = _size;
	vkCmdCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue_);

	vkFreeCommandBuffers(logicalDevice_, commandPool_, 1, &commandBuffer);
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
	
	vkResetCommandBuffer(commandBuffers_[currentFrame_], /*VkCommandBufferResetFlagBits*/ 0);
	RecordCommandBuffer(commandBuffers_[currentFrame_], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[]		= {imageAvailableSemaphores_[currentFrame_]};
	VkPipelineStageFlags waitStages[]	= {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount		= 1;
	submitInfo.pWaitSemaphores			= waitSemaphores;
	submitInfo.pWaitDstStageMask		= waitStages;
	submitInfo.commandBufferCount		= 1;
	submitInfo.pCommandBuffers			= &commandBuffers_[currentFrame_];

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

	vkDestroyCommandPool(logicalDevice_, commandPool_, nullptr);

	vkDestroyDevice(logicalDevice_, nullptr);

	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance_, debugMessenger_, nullptr);
	}

	vkDestroySurfaceKHR(instance_, surface_, nullptr);
	vkDestroyInstance(instance_, nullptr);
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
bool Application::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}
//======================================================================================================================
void Application::SetupDebugMessenger()
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo_(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debugMessenger_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
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
		vkGetPhysicalDeviceSurfaceSupportKHR(_device, i, surface_, &presentSupport);

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
SwapChainSupportDetails Application::QuerySwapChainSupport(VkPhysicalDevice _device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, surface_, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(_device, surface_, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(_device, surface_, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(_device, surface_, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(_device, surface_, &presentModeCount, details.presentModes.data());
	}
	return details;
}
//======================================================================================================================
std::vector<const char*> Application::GetRequiredExtensions_()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}
//======================================================================================================================
void Application::PopulateDebugMessengerCreateInfo_(VkDebugUtilsMessengerCreateInfoEXT& _createInfo)
{
	_createInfo					= {};
	_createInfo.sType			= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	_createInfo.messageSeverity	= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	_createInfo.messageType		= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	_createInfo.pfnUserCallback	= DebugCallback_;
}
//======================================================================================================================
VKAPI_ATTR VkBool32 VKAPI_CALL Application::DebugCallback_(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}
////======================================================================================================================
//void Application::FramebufferResizeCallback(GLFWwindow*	_window,
//											int			_width,
//											int			_height)
//{
//	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(_window));
//	app->framebufferResized_ = true;
//}
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
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
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
	QueueFamilyIndices indices	= FindQueueFamilies(_device);
	bool extensionsSupported	= CheckDeviceExtensionSupport_(_device);

	bool swapChainAdequate		= false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(_device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	return indices.isComplete() && extensionsSupported && swapChainAdequate;
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