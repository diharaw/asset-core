#include <exporter/mesh_exporter.h>
#include <exporter/material_exporter.h>
#include <common/filesystem.h>
#include <common/header.h>
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>

#define WRITE_AND_OFFSET(stream, dest, size, offset) \
    stream.write((char*)dest, size);                 \
    offset += size;                                  \
    stream.seekg(offset);

namespace ast
{
bool export_mesh(const MeshImportResult& import_result, const MeshExportOption& options)
{
    auto start = std::chrono::high_resolution_clock::now();

    std::filesystem::path output_root_folder_path_absolute = std::filesystem::path(options.output_root_folder_path);

    // Check if output root folder path is absolute
    if (!output_root_folder_path_absolute.is_absolute())
    {
        // Create absolute path
        std::string absolute_output_path = std::filesystem::current_path().string() + "/" + options.output_root_folder_path;
        output_root_folder_path_absolute = std::filesystem::path(absolute_output_path);
    }

    if (!filesystem::does_directory_exist(output_root_folder_path_absolute.string()))
        filesystem::create_directory(output_root_folder_path_absolute.string());

    std::string mesh_path = output_root_folder_path_absolute.string() + "/mesh";

    if (!filesystem::create_directory(mesh_path))
    {
        std::cout << "Invalid Mesh path!" << std::endl;
        return false;
    }

    std::string material_path = output_root_folder_path_absolute.string() + "/material";

    if (!filesystem::create_directory(material_path))
    {
        std::cout << "Invalid Material path!" << std::endl;
        return false;
    }

    std::string texture_path = output_root_folder_path_absolute.string() + "/texture";

    if (!filesystem::create_directory(texture_path))
    {
        std::cout << "Invalid Texture path!" << std::endl;
        return false;
    }

    std::string output_path = output_root_folder_path_absolute.string();
    output_path += "/mesh/";
    output_path += import_result.name;
    output_path += ".ast";

    std::fstream f(output_path, std::ios::out | std::ios::binary);

    if (f.is_open())
    {
        BINFileHeader fh;
        char*         magic = (char*)&fh.magic;

        magic[0] = 'a';
        magic[1] = 's';
        magic[2] = 't';

        fh.version = AST_VERSION;
        fh.type    = ASSET_MESH;

        BINMeshFileHeader header;

        // Copy Name
        strcpy(&header.name[0], import_result.name.c_str());
        header.name[import_result.name.size()] = '\0';

        header.index_count           = import_result.indices.size();
        header.vertex_count          = import_result.vertices.size();
        header.skeletal_vertex_count = import_result.skeletal_vertices.size();
        header.material_count        = import_result.materials.size();
        header.mesh_count            = import_result.submeshes.size();
        header.max_extents           = import_result.max_extents;
        header.min_extents           = import_result.min_extents;

        size_t offset = 0;

        // Write mesh header
        WRITE_AND_OFFSET(f, (char*)&fh, sizeof(BINFileHeader), offset);

        // Write mesh header
        WRITE_AND_OFFSET(f, (char*)&header, sizeof(BINMeshFileHeader), offset);

        // Write vertices
        if (import_result.vertices.size() > 0)
        {
            WRITE_AND_OFFSET(f, (char*)&import_result.vertices[0], sizeof(Vertex) * import_result.vertices.size(), offset);
        }

        // Write skeletal vertices
        if (import_result.skeletal_vertices.size() > 0)
        {
            WRITE_AND_OFFSET(f, (char*)&import_result.skeletal_vertices[0], sizeof(SkeletalVertex) * import_result.skeletal_vertices.size(), offset);
        }

        // Write indices
        if (import_result.indices.size() > 0)
        {
            WRITE_AND_OFFSET(f, (char*)&import_result.indices[0], sizeof(uint32_t) * import_result.indices.size(), offset);
        }

        // Write mesh headers
        if (import_result.submeshes.size() > 0)
        {
            WRITE_AND_OFFSET(f, (char*)&import_result.submeshes[0], sizeof(SubMesh) * import_result.submeshes.size(), offset);
        }

        // Export materials
        std::vector<BINMeshMaterialJson> mats;

        for (int i = 0; i < import_result.materials.size(); i++)
        {
            auto material = import_result.materials[i].get();

            MaterialExportOptions mat_exp_options;

            mat_exp_options.output_root_folder_path_absolute = output_root_folder_path_absolute.string();
            mat_exp_options.use_compression                  = options.use_compression;
            mat_exp_options.normal_map_flip_green            = options.normal_map_flip_green;

            if (export_material(*material, mat_exp_options))
            {
                std::string mat_out_path = "../material/" + material->name + ".json";

                BINMeshMaterialJson mat;

                strcpy(&mat.material[0], mat_out_path.c_str());
                mat.material[mat_out_path.size()] = '\0';

                mats.push_back(mat);
            }
        }

        // Write material paths
        if (mats.size() > 0)
        {
            WRITE_AND_OFFSET(f, (char*)&mats[0], sizeof(BINMeshMaterialJson) * mats.size(), offset);
        }

        f.close();

        if (options.output_metadata)
        {
            nlohmann::json doc;

            doc["name"]           = import_result.name;
            doc["vertex_count"]   = import_result.vertices.size();
            doc["index_count"]    = import_result.indices.size();
            doc["submesh_count"]  = import_result.submeshes.size();
            doc["material_count"] = import_result.materials.size();

            auto submesh_array = doc.array();

            for (auto& submesh_desc : import_result.submeshes)
            {
                nlohmann::json submesh;

                submesh["name"]           = submesh_desc.name;
                submesh["material_index"] = submesh_desc.material_index;
                submesh["index_count"]    = submesh_desc.index_count;
                submesh["vertex_count"]   = submesh_desc.vertex_count;
                submesh["base_vertex"]    = submesh_desc.base_vertex;
                submesh["base_index"]     = submesh_desc.base_index;

                auto min_array = doc.array();
                min_array.push_back(submesh_desc.min_extents[0]);
                min_array.push_back(submesh_desc.min_extents[1]);
                min_array.push_back(submesh_desc.min_extents[2]);

                submesh["min_extents"] = min_array;

                auto max_array = doc.array();
                max_array.push_back(submesh_desc.max_extents[0]);
                max_array.push_back(submesh_desc.max_extents[1]);
                max_array.push_back(submesh_desc.max_extents[2]);

                submesh["max_extents"] = max_array;

                submesh_array.push_back(submesh);
            }

            doc["submeshes"] = submesh_array;

            auto material_array = doc.array();

            for (int mat_id = 0; mat_id < import_result.materials.size(); mat_id++)
            {
                nlohmann::json material;

                material["index"] = mat_id;
                material["path"]  = "../material/" + import_result.materials[mat_id]->name + ".json";

                material_array.push_back(material);
            }

            doc["materials"] = material_array;

            std::string output_path = output_root_folder_path_absolute.string() + "/" + import_result.name + "_metadata.json";

            std::string output_str = doc.dump(4);

            std::fstream f(output_path, std::ios::out);

            if (f.is_open())
            {
                f.write(output_str.c_str(), output_str.size());
                f.close();
            }
            else
                std::cout << "Failed to write Metadata JSON!" << std::endl;
        }

        auto                          finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> time   = finish - start;

        printf("Successfully exported mesh(%s) in %f seconds\n\n", import_result.name.c_str(), time.count());

        return true;
    }
    else
        std::cout << "Failed to write Mesh!" << std::endl;

    return false;
}
} // namespace ast
