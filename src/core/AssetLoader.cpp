#include "AssetLoader.hpp"
#include <iostream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace
{}

namespace rex::AssetLoader
{

	std::vector<char> readFile(const std::string &fileName)
	{
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		std::vector<char> buffer(file.tellg());

		file.seekg(0, std::ios::beg);
		file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

		file.close();

		return buffer;
	}

	ImageData loadImage(const std::string &path)
	{
	    int            texWidth, texHeight, texChannels;
	    stbi_uc       *pixels    = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	    if (!pixels)
		    throw std::runtime_error("failed to load texture image!");
		
	    ImageData img{
	        .width  = static_cast<uint32_t>(texWidth),
	        .height = static_cast<uint32_t>(texHeight),
	    };
	    img.pixels.assign(pixels, pixels + static_cast<size_t>(texWidth) * texHeight * 4);
	    
	    stbi_image_free(pixels);

	    return img;
	}

}