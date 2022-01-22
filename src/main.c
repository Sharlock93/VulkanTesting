#include <stdio.h>
#include <vulkan/vulkan.h>


int main(int argc, char **argv) {

	unsigned int c = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &c, NULL);
	printf("%d", c);


	return 0;
}
