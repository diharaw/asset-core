#include <offline/image_exporter.h>
#include <common/header.h>
#include <cmft/image.h>
#include <cmft/cubemapfilter.h>
#include <nvtt/nvtt.h>
#include <nvimage/Image.h>
#include <nvimage/DirectDrawSurface.h>
#include <thread>

#define RADIANCE_MAP_SIZE 256
#define CMFT_GLOSS_SCALE 10
#define CMFT_GLOSS_BIAS 3
#define RADIANCE_MAP_MIP_LEVELS 7

#define WRITE_AND_OFFSET(stream, dest, size, offset) stream.write((char*)dest, size); offset += size; stream.seekg(offset);

namespace ast
{
    const nvtt::Format kCompression[] = 
	{
        nvtt::Format_RGB,
        nvtt::Format_BC1,
        nvtt::Format_BC1a,
        nvtt::Format_BC2,
        nvtt::Format_BC3,
        nvtt::Format_BC3n,
        nvtt::Format_BC4,
        nvtt::Format_BC5,
        nvtt::Format_BC6,
        nvtt::Format_BC7
    };
    
    struct NVTTOutputHandler : public nvtt::OutputHandler
    {
        std::fstream* stream;
        long offset;
        int mip_levels = 0;
        int mip_height;
        int compression_type;
        
        virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) override
        {
#ifdef _DEBUG
            std::cout << "Beginning Image: Size = " << size << ", Mip = " << miplevel << ", Width = " << width << ", Height = " << height << std::endl;
#endif

            BINMipSliceHeader mip0Header;
            
            mip0Header.width = width;
            mip0Header.height = height;
            mip0Header.size = size;
            
            stream->write((char*)&mip0Header, sizeof(BINMipSliceHeader));
            offset += sizeof(BINMipSliceHeader);
            stream->seekp(offset);
            
            mip_height = height;
            mip_levels++;
        }
        
        virtual bool writeData(const void * data, int size) override
        {
            stream->write((char*)data, size);
            offset += size;
            stream->seekp(offset);
            return true;
        }
        
        virtual void endImage() override
        {
#ifdef _DEBUG
            std::cout << "Ending Image.." << std::endl;
#endif
        }
    };

#define FLIP_GREEN(type, num_components, dst_data)													\
Image::Pixel<type, num_components>* dst = (Image::Pixel<type, num_components>*)dst_data.data;		\
for (int y = 0; y < dst_data.height; y++)															\
{																									\
	for (int x = 0; x < dst_data.width; x++)														\
	{																								\
		type value = dst[y * dst_data.width + x].c[1];												\
		dst[y * dst_data.width + x].c[1] = std::numeric_limits<type>::max() - value;				\
	}																								\
}

    bool export_image(Image& img, const ImageExportOptions& options)
    {
        // Make sure that float images either use no compression or BC6
        if ((img.type == PIXEL_TYPE_FLOAT16 || img.type == PIXEL_TYPE_FLOAT32) && (options.compression != COMPRESSION_NONE && options.compression != COMPRESSION_BC6))
        {
            std::cout << "ERROR::Float images must use either no compression or BC6!" << std::endl;
            return false;
        }

		if (options.flip_green)
		{
			for (uint32_t layer = 0; layer < img.array_slices; layer++)
			{
				for (uint32_t mip = 0; mip < img.mip_slices; mip++)
				{
					Image::Data& data = img.data[layer][mip];

					if (img.type == PIXEL_TYPE_UNORM8)
					{
						if (img.components == 4)
						{
							FLIP_GREEN(uint8_t, 4, data)
						}
						else
						{
							FLIP_GREEN(uint8_t, 3, data)
						}
					}
					else if (img.type == PIXEL_TYPE_FLOAT16)
					{
						if (img.components == 4)
						{
							FLIP_GREEN(int16_t, 4, data)
						}
						else
						{
							FLIP_GREEN(int16_t, 3, data)
						}
					}
					else if (img.type == PIXEL_TYPE_FLOAT32)
					{
						if (img.components == 4)
						{
							FLIP_GREEN(float, 4, data)
						}
						else
						{
							FLIP_GREEN(float, 3, data)
						}
					}
				}
			}
		}
        
        BINFileHeader fh;
        char* magic = (char*)&fh.magic;
        
        magic[0] = 'a';
        magic[1] = 's';
        magic[2] = 't';
        
        fh.version = AST_VERSION;
        fh.type = ASSET_IMAGE;
        
        BINImageHeader image_header;
        
        int x = img.data[0][0].width;
        int y = img.data[0][0].height;
        int mX = x;
        int mY = y;
        
        // output_mips  =  -1 = a full mipchain will be generated.
        // output_mips  =   0 = the existing mips will be used. No new mips will be generated
        // output_mips  >=  1 = N number of miplevels will be generated.
        
        int32_t mip_levels = options.output_mips;
        bool generate_mipmaps = true;
        
        if (mip_levels < -1)
        {
            std::cout << "WARNING::mipmaps_to_generate must be greater than or equal to -1. Generating full mipchain..." << std::endl;
            mip_levels = -1;
        }
        
        if (options.output_mips == -1)
        {
            mip_levels = 0;
            
            while (mX > 1 && mY > 1)
            {
                mX = x >> mip_levels;
                mY = y >> mip_levels;
                
                mip_levels++;
            }
        }
        else if (options.output_mips == 0)
            mip_levels = img.mip_slices;
        
        if (mip_levels == 1)
            generate_mipmaps = false;
        
        image_header.compression = options.compression;
        image_header.channel_size = img.type;
        image_header.num_channels = img.components;
        image_header.num_array_slices = img.array_slices;
        image_header.num_mip_slices = mip_levels;
        
        std::string filename = img.name;
        std::string path;

		if (options.path.size() > 0)
		{
			path = options.path;
			path += "/";
		}
       
        path += filename;
        path += ".ast";
        
        std::fstream f(path, std::ios::out | std::ios::binary);

		if (!f.is_open())
			std::cout << "Failed to open file: " << path << std::endl;

        long offset = 0;
        f.seekp(offset);
        
        WRITE_AND_OFFSET(f, &fh, sizeof(fh), offset);
        
        uint16_t len = filename.size();
        WRITE_AND_OFFSET(f, &len, sizeof(uint16_t), offset);
        
        WRITE_AND_OFFSET(f, filename.c_str(), len, offset);
        
        WRITE_AND_OFFSET(f, &image_header, sizeof(BINImageHeader), offset);
        
        if (options.output_mips == 0 && options.compression == COMPRESSION_NONE)
        {
            for (uint32_t i = 0; i < img.array_slices; i++)
            {
                for (uint32_t j = 0; j < img.mip_slices; j++)
                {
                    BINMipSliceHeader mip_header;
                    
                    mip_header.width = img.data[i][j].width;
                    mip_header.height = img.data[i][j].height;
                    mip_header.size = mip_header.width * mip_header.height * img.type * img.components;
                    
                    WRITE_AND_OFFSET(f, &mip_header, sizeof(BINMipSliceHeader), offset);
                    
                    WRITE_AND_OFFSET(f, img.data[i][j].data, mip_header.size, offset);
                }
            }
        }
        else
        {
            NVTTOutputHandler handler;
            nvtt::CompressionOptions compression_options;
            nvtt::InputOptions input_options;
            nvtt::OutputOptions output_options;
            nvtt::Compressor compressor;
            
            handler.stream = &f;
            handler.offset = offset;
            
            compression_options.setFormat(kCompression[options.compression]);
            
            if (options.compression == COMPRESSION_NONE)
            {
                uint32_t pixel_size = 8 * options.pixel_type;
                
                if (img.type == PIXEL_TYPE_UNORM8)
                    compression_options.setPixelType(nvtt::PixelType_UnsignedNorm);
                else if (img.type == PIXEL_TYPE_FLOAT16 || img.type == PIXEL_TYPE_FLOAT32)
                    compression_options.setPixelType(nvtt::PixelType_Float);
                
                if (img.components == 4)
                    compression_options.setPixelFormat(pixel_size, pixel_size, pixel_size, pixel_size);
                else if (img.components == 3)
                    compression_options.setPixelFormat(pixel_size, pixel_size, pixel_size, 0);
                else if (img.components == 2)
                    compression_options.setPixelFormat(pixel_size, pixel_size, 0, 0);
                else if (img.components == 1)
                    compression_options.setPixelFormat(0, 0, pixel_size, 0);
            }
            
            if (options.pixel_type == PIXEL_TYPE_UNORM8)
                input_options.setFormat(nvtt::InputFormat_BGRA_8UB);
            else if (options.pixel_type == PIXEL_TYPE_FLOAT16)
                input_options.setFormat(nvtt::InputFormat_RGBA_16F);
            else if (options.pixel_type == PIXEL_TYPE_FLOAT32)
                input_options.setFormat(nvtt::InputFormat_RGBA_32F);
            
            input_options.setNormalMap(options.normal_map);
            input_options.setConvertToNormalMap(false);
            input_options.setNormalizeMipmaps(false);
            
            output_options.setOutputHeader(false);
            output_options.setOutputHandler(&handler);
            
            for (int i = 0; i < img.array_slices; i++)
            {
                Image temp_img;
                Image* current_img = &temp_img;
                
                input_options.setTextureLayout(nvtt::TextureType_2D, img.data[i][0].width, img.data[i][0].height);
                
                // If generate_mipmaps is false or if the full mipchain has to be generated, set the data for the initial mip level.
                if (!generate_mipmaps || (generate_mipmaps && options.output_mips == -1) || (generate_mipmaps && options.output_mips > 1))
                {
					if (img.components == 4)
					{
						if (options.compression == COMPRESSION_NONE)
							img.argb_to_rgba(i, 0);
						else
							img.to_bgra(i, 0);

						current_img = &img;
					}
                    else
                        img.to_rgba(temp_img, i, 0);
                    
                    input_options.setMipmapGeneration(generate_mipmaps);
                    input_options.setMipmapData(current_img->data[i][0].data, img.data[i][0].width, img.data[i][0].height);
                }
                else if (generate_mipmaps && img.mip_slices > 1)
                {
                    input_options.setMipmapGeneration(generate_mipmaps, img.mip_slices);
                    
                    for (int mip = 0; mip < img.mip_slices; mip++)
                    {
                        if (img.components == 4)
						{
							if (options.compression == COMPRESSION_NONE)
								img.argb_to_rgba(i, 0);
							else
								img.to_bgra(i, 0);

							current_img = &img;
						}
                        else
                            img.to_rgba(temp_img, i, 0);
                        
                        input_options.setMipmapData(current_img->data[i][mip].data, img.data[i][mip].width, img.data[i][mip].height, 1, 0, mip);
                    }
                }
                else
                {
                    temp_img.deallocate();
                    std::cout << "ERROR::Image must contain at least one miplevel" << std::endl;
                    return false;
                }
                
                handler.mip_levels = 0;
                compressor.process(input_options, compression_options, output_options);
                
                temp_img.deallocate();
            }
        }
        
        f.close();
        
        return true;
    }
    
    bool cubemap_from_latlong(cmft::Image& dst, Image& src)
    {
        cmft::Image cmft_img;
        cmft_img.m_width = uint32_t(src.data[0][0].width);
        cmft_img.m_height = uint32_t(src.data[0][0].height);
        cmft_img.m_dataSize = src.size(0, 0);
        
        if (src.components < 3)
        {
            std::cout << "ERROR::Image must at least have 3 color channels" << std::endl;
            return false;
        }
        
        if (src.components == 4)
        {
            if (src.type == PIXEL_TYPE_UNORM8)
                cmft_img.m_format = cmft::TextureFormat::RGBA8;
            else if (src.type == PIXEL_TYPE_FLOAT16)
                cmft_img.m_format = cmft::TextureFormat::RGBA16F;
            else if (src.type == PIXEL_TYPE_FLOAT32)
                cmft_img.m_format = cmft::TextureFormat::RGBA32F;
        }
        else if (src.components == 3)
        {
            if (src.type == PIXEL_TYPE_UNORM8)
                cmft_img.m_format = cmft::TextureFormat::RGB8;
            else if (src.type == PIXEL_TYPE_FLOAT16)
                cmft_img.m_format = cmft::TextureFormat::RGB16F;
            else if (src.type == PIXEL_TYPE_FLOAT32)
                cmft_img.m_format = cmft::TextureFormat::RGB32F;
        }
        
        cmft_img.m_numMips = 1;
        cmft_img.m_numFaces = 1;
        cmft_img.m_data = (void*)src.data[0][0].data;
        
        if (!cmft::imageIsLatLong(cmft_img))
        {
            std::cout << "ERROR::Image is not in latlong format" << std::endl;
            return false;
        }
        
        if (!cmft::imageCubemapFromLatLong(dst, cmft_img))
        {
            std::cout << "ERROR::Failed to convert Cubemap" << std::endl;
            return false;
        }
        
        return true;
    }
    
    bool cubemap_from_latlong(Image& src, const CubemapImageExportOptions& options)
    {
        cmft::Image cmft_cube;
        
        if (!cubemap_from_latlong(cmft_cube, src))
        {
            std::cout << "ERROR::Failed to convert Cubemap" << std::endl;
            return false;
        }
        
        if (options.irradiance)
        {
            cmft::Image cmft_irradiance_cube;
            
            if (!cmft::imageIrradianceFilterSh(cmft_irradiance_cube, 128, cmft_cube))
            {
                std::cout << "ERROR::Failed to generate irradiance map!" << std::endl;
                return false;
            }
            
            Image irradiance_cube;
            
            uint32_t img_offsets[CUBE_FACE_NUM][MAX_MIP_NUM];
            cmft::imageGetMipOffsets(img_offsets, cmft_irradiance_cube);
            
            irradiance_cube.name = src.name;
            irradiance_cube.name += "_irradiance";
            
            irradiance_cube.components = src.components;
            irradiance_cube.array_slices = 6;
            irradiance_cube.mip_slices = 1;
            
            for (int i = 0; i < 6; i++)
            {
                uint8_t* mip_data = (uint8_t*)cmft_irradiance_cube.m_data + img_offsets[i][0];
                
                irradiance_cube.data[i][0].data = (float*)mip_data;
                irradiance_cube.data[i][0].height = cmft_irradiance_cube.m_height;
                irradiance_cube.data[i][0].width = cmft_irradiance_cube.m_width;
            }
            
            ImageExportOptions irradiance_exp_options;
            
            irradiance_exp_options.compression = options.compression;
            irradiance_exp_options.normal_map = false;
            irradiance_exp_options.output_mips = 0;
            irradiance_exp_options.path = options.path;
            
            if (!export_image(irradiance_cube, irradiance_exp_options))
            {
                std::cout << "ERROR::Failed to export Cubemap" << std::endl;
                return false;
            }
            
            cmft::imageUnload(cmft_irradiance_cube);
        }
        
        if (options.radiance)
        {
            cmft::Image cmft_radiance_cube;
            
            int threads = std::thread::hardware_concurrency();
            
            std::cout << "Using " << threads << " threads to generate radiance map" << std::endl;
            
            if (!cmft::imageRadianceFilter(cmft_radiance_cube,
                                           RADIANCE_MAP_SIZE,
                                           cmft::LightingModel::BlinnBrdf,
                                           true,
                                           RADIANCE_MAP_MIP_LEVELS,
                                           CMFT_GLOSS_SCALE,
                                           CMFT_GLOSS_BIAS,
                                           cmft_cube,
                                           cmft::EdgeFixup::None,
                                           threads))
            {
                std::cout << "ERROR::Failed to generate radiance map!" << std::endl;
                return false;
            }
            
            Image radiance_cube;
            
            uint32_t img_offsets[CUBE_FACE_NUM][MAX_MIP_NUM];
            cmft::imageGetMipOffsets(img_offsets, cmft_radiance_cube);
            
            radiance_cube.name = src.name;
            radiance_cube.name += "_radiance";
            
            radiance_cube.components = src.components;
            radiance_cube.array_slices = 6;
            radiance_cube.mip_slices = 7;
            
            for (int i = 0; i < 6; i++)
            {
                for (int j = 0; j < 7; j++)
                {
                    uint8_t* mip_data = (uint8_t*)cmft_radiance_cube.m_data + img_offsets[i][j];
                    
                    radiance_cube.data[i][j].data = (void*)mip_data;
                    radiance_cube.data[i][j].height = cmft_radiance_cube.m_height >> j;
                    radiance_cube.data[i][j].width = cmft_radiance_cube.m_width >> j;
                }
            }
            
            ImageExportOptions radiance_exp_options;
            
            radiance_exp_options.compression = options.compression;
            radiance_exp_options.normal_map = false;
            radiance_exp_options.output_mips = 0;
            radiance_exp_options.path = options.path;
            
            if (!export_image(radiance_cube, radiance_exp_options))
            {
                std::cout << "ERROR::Failed to export Cubemap" << std::endl;
                return false;
            }
            
            cmft::imageUnload(cmft_radiance_cube);
        }
        
        uint32_t img_offsets[CUBE_FACE_NUM][MAX_MIP_NUM];
        cmft::imageGetMipOffsets(img_offsets, cmft_cube);
        
        Image cubemap;
        
        cubemap.name = src.name;
        cubemap.components = src.components;
        cubemap.array_slices = 6;
        cubemap.mip_slices = 1;
        
        for (int i = 0; i < 6; i++)
        {
            uint8_t* mip_data = (uint8_t*)cmft_cube.m_data + img_offsets[i][0];
            
            cubemap.data[i][0].data = (void*)mip_data;
            cubemap.data[i][0].height = cmft_cube.m_height;
            cubemap.data[i][0].width = cmft_cube.m_width;
        }
        
        ImageExportOptions exp_options;
        
        exp_options.compression = options.compression;
        exp_options.normal_map = false;
        exp_options.output_mips = options.output_mips;
        exp_options.pixel_type = options.pixel_type;
        exp_options.path = options.path;
        
        if (!export_image(cubemap, exp_options))
        {
            std::cout << "ERROR::Failed to export Cubemap" << std::endl;
            return false;
        }
        
        cmft::imageUnload(cmft_cube);
        src.deallocate();
        
        return true;
    }
    
    bool cubemap_from_latlong(const std::string& input, const CubemapImageExportOptions& options)
    {
        Image src;
        
        if (!import_image(src, input))
        {
            std::cout << "ERROR::Failed to open Cubemap" << std::endl;
            return false;
        }
        
        return cubemap_from_latlong(src, options);
    }
}
