#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <vector>

int main() {
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app.pApplicationName = "gpu-api-validation";
    app.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo create{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create.pApplicationInfo = &app;

    VkInstance instance{};
    if (vkCreateInstance(&create, nullptr, &instance) != VK_SUCCESS) {
        std::cerr << "failed to create Vulkan instance\n";
        return 1;
    }

    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (!count) {
        std::cerr << "no Vulkan physical devices\n";
        vkDestroyInstance(instance, nullptr);
        return 1;
    }

    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, devices.data());
    for (uint32_t i = 0; i < count; ++i) {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(devices[i], &props);
        std::cout << "Vulkan device[" << i << "]: " << props.deviceName
                  << " api=" << VK_VERSION_MAJOR(props.apiVersion) << "."
                  << VK_VERSION_MINOR(props.apiVersion) << "."
                  << VK_VERSION_PATCH(props.apiVersion) << "\n";
    }

    vkDestroyInstance(instance, nullptr);
    return 0;
}
