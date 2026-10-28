#pragma once

/*
namespace AssetLoader

        loadOneTextureIntoAtlas(...)
                - ported from main

        loadModel(...)
                - load textures of the model using above function
                - load the vertices and indices in the master_vertex_buffer and master_index_buffer
                - setup gameobject submesh vector

        loadSceneFromFile(string filepath)
                - parse the file and load all the objects into memory
                - setup gameObjects

*/

#include <vector>
#include <string>

namespace rex::AssetLoader
{
        struct ImageData
        {
	        uint32_t             width  = 0;
	        uint32_t             height = 0;
	        std::vector<uint8_t> pixels;
        };

        std::vector<char> readFile(const std::string &fileName);
        ImageData         loadImage(const std::string &path);
}