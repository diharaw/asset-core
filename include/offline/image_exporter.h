#pragma once

#include <offline/image_importer.h>
#include <cmft/image.h>
#include <cmft/cubemapfilter.h>
#include <ostream>
#include <fstream>

#define READ_AND_OFFSET(stream, dest, size, offset) stream.read((char*)dest, size); offset += size; stream.seekg(offset);

#define RADIANCE_MAP_SIZE 256
#define CMFT_GLOSS_SCALE 10
#define CMFT_GLOSS_BIAS 3
#define RADIANCE_MAP_MIP_LEVELS 7

namespace ast
{
	enum AssetType
	{
		ASSET_IMAGE = 0,
		ASSET_AUDIO = 1,
		ASSET_MESH = 2,
		ASSET_VIDEO = 3
	};

	struct FileHeader
	{
		uint32_t magic;
		uint8_t  version;
		uint8_t  type;
	};

	enum CompressionType
	{
		COMPRESSION_NONE = 0,
		COMPRESSION_BC1 = 1,
		COMPRESSION_BC1a = 2,
		COMPRESSION_BC2 = 3,
		COMPRESSION_BC3 = 4,
		COMPRESSION_BC3n = 5,
		COMPRESSION_BC4 = 6,
		COMPRESSION_BC5 = 7,
		COMPRESSION_BC6 = 8,
		COMPRESSION_BC7 = 9,
		COMPRESSION_ETC1 = 10,
		COMPRESSION_ETC2 = 11,
		COMPRESSION_PVR = 12
	};

	enum PixelType
	{
		PIXEL_TYPE_DEFAULT = 0,
		PIXEL_TYPE_UNORM8 = 8,
		PIXEL_TYPE_FLOAT16 = 16,
		PIXEL_TYPE_FLOAT32 = 32
	};

	struct Header
	{
		uint8_t  compression;
		uint8_t  channelSize;
		uint8_t  numChannels;
		uint16_t numArraySlices;
		uint8_t  numMipSlices;
	};

	struct MipSliceHeader
	{
		uint16_t width;
		uint16_t height;
		int size;
	};

	struct TRMOutputHandler : public nvtt::OutputHandler
	{
		std::fstream* stream;
		long offset;
		int mip_levels = 0;
		int mip_height;
		int compression_type;

		virtual void beginImage(int size, int width, int height, int depth, int face, int miplevel) override
		{
			std::cout << "Beginning Image: Size = " << size << ", Mip = " << miplevel << ", Width = " << width << ", Height = " << height << std::endl;

			MipSliceHeader mip0Header;

			mip0Header.width = width;
			mip0Header.height = height;
			mip0Header.size = size;

			stream->write((char*)&mip0Header, sizeof(MipSliceHeader));
			offset += sizeof(MipSliceHeader);
			stream->seekp(offset);

			mip_height = height;
			mip_levels++;
		}

		// Output data. Compressed data is output as soon as it's generated to minimize memory allocations.
		virtual bool writeData(const void * data, int size) override
		{
			stream->write((char*)data, size);
			offset += size;
			stream->seekp(offset);
			return true;
		}

		// Indicate the end of the compressed image. (New in NVTT 2.1)
		virtual void endImage() override
		{
			std::cout << "Ending Image.." << std::endl;
		}
	};

	

	const char* extension(const char* file)
	{
		int length = strlen(file);

		while (length > 0 && file[length] != '.')
		{
			length--;
		}

		return &file[length + 1];
	}

	const int kChannels[] = { 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4 };

	const nvtt::Format kCompression[] = {
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

    struct ImageExportOptions
    {
        std::string path;
        PixelType pixel_type = PIXEL_TYPE_DEFAULT;
        CompressionType compression = COMPRESSION_NONE;
        bool normal_map = false;
        int output_mips = 0;
    };
    
	template<typename T>
	bool export_image(Image<T>& img,
		const ImageExportOptions& options)
	{
		// Make sure that float images either use no compression or BC6
		if ((std::is_same<T, uint16_t>::value || std::is_same<T, int16_t>::value || std::is_same<T, float>::value) && (options.compression != COMPRESSION_NONE && options.compression != COMPRESSION_BC6))
		{
			std::cout << "ERROR::Float images must use either no compression or BC6!" << std::endl;
			return false;
		}

		FileHeader fh;
		char* magic = (char*)&fh.magic;

		magic[0] = 't';
		magic[1] = 'r';
		magic[2] = 'm';

		fh.version = 1;
		fh.type = ASSET_IMAGE;

		Header imageHeader;

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

			mip_levels++;
		}
		else if (options.output_mips == 0)
			mip_levels = img.mip_slices;

		if (mip_levels == 1)
			generate_mipmaps = false;

		imageHeader.compression = options.compression;
		imageHeader.channelSize = sizeof(T);
		imageHeader.numChannels = img.components;
		imageHeader.numArraySlices = img.array_slices;
		imageHeader.numMipSlices = mip_levels;

		const char* file = options.path.c_str(); // @TODO: Extract name from filename.
		std::fstream f(options.path, std::ios::out | std::ios::binary);

		long offset = 0;

		f.seekp(offset);
		f.write((char*)&fh, sizeof(fh));
		offset += sizeof(fh);
		f.seekp(offset);
		uint16_t len = strlen(file);
		f.write((char*)&len, sizeof(uint16_t));
		offset += sizeof(uint16_t);
		f.seekp(offset);
		f.write((char*)file, strlen(file));
		offset += len;
		f.seekp(offset);
		f.write((char*)&imageHeader, sizeof(Header));
		offset += sizeof(Header);
		f.seekp(offset);

		TRMOutputHandler handler;
		nvtt::CompressionOptions compression_options;
		nvtt::InputOptions input_options;
		nvtt::OutputOptions output_options;
		nvtt::Compressor compressor;

		handler.stream = &f;
		handler.offset = offset;

		compression_options.setFormat(kCompression[options.compression]);

		if (options.compression == COMPRESSION_NONE)
		{
			uint32_t pixel_size = (options.pixel_type == PIXEL_TYPE_DEFAULT) ? 8 * sizeof(T) : options.pixel_type;

			if (std::is_same<T, uint8_t>::value)
				compression_options.setPixelType(nvtt::PixelType_UnsignedNorm);
			else if (std::is_same<T, uint16_t>::value || std::is_same<T, int16_t>::value || std::is_same<T, float>::value)
				compression_options.setPixelType(nvtt::PixelType_Float);

			if (img.components == 4)
				compression_options.setPixelFormat(pixel_size, pixel_size, pixel_size, pixel_size);
			else if (img.components == 3)
				compression_options.setPixelFormat(pixel_size, pixel_size, pixel_size, 0);
			else if (img.components == 2)
				compression_options.setPixelFormat(pixel_size, pixel_size, 0, 0);
			else if (img.components == 1)
				compression_options.setPixelFormat(pixel_size, 0, 0, 0);
		}

		if (options.pixel_type == PIXEL_TYPE_DEFAULT)
		{
			if (std::is_same<T, uint8_t>::value)
				input_options.setFormat(nvtt::InputFormat_BGRA_8UB);
			else if (std::is_same<T, uint16_t>::value || std::is_same<T, int16_t>::value)
				input_options.setFormat(nvtt::InputFormat_RGBA_16F);
			else if (std::is_same<T, float>::value)
				input_options.setFormat(nvtt::InputFormat_RGBA_32F);
		}
		else
		{
			if (options.pixel_type == PIXEL_TYPE_UNORM8)
				input_options.setFormat(nvtt::InputFormat_BGRA_8UB);
			else if (options.pixel_type == PIXEL_TYPE_FLOAT16)
				input_options.setFormat(nvtt::InputFormat_RGBA_16F);
			else if (options.pixel_type == PIXEL_TYPE_FLOAT32)
				input_options.setFormat(nvtt::InputFormat_RGBA_32F);
		}

		input_options.setNormalMap(options.normal_map);
		input_options.setConvertToNormalMap(false);
		input_options.setNormalizeMipmaps(false);

		output_options.setOutputHeader(false);
		output_options.setOutputHandler(&handler);

		for (int i = 0; i < img.array_slices; i++)
		{
			Image<T> temp_img;

			input_options.setTextureLayout(nvtt::TextureType_2D, img.data[i][0].width, img.data[i][0].height);

			// If generate_mipmaps is false or if the full mipchain has to be generated, set the data for the initial mip level.
			if (!generate_mipmaps || (generate_mipmaps && options.output_mips == -1) || (generate_mipmaps && options.output_mips > 1))
			{
				img.to_rgba(temp_img, i, 0);
				input_options.setMipmapGeneration(generate_mipmaps);
				input_options.setMipmapData(temp_img.data[i][0].data, img.data[i][0].width, img.data[i][0].height);
			}
			else if (generate_mipmaps && img.mip_slices > 1)
			{
				input_options.setMipmapGeneration(generate_mipmaps, img.mip_slices);

				for (int mip = 0; mip < img.mip_slices; mip++)
				{
					img.to_rgba(temp_img, i, mip);
					input_options.setMipmapData(temp_img.data[i][mip].data, img.data[i][mip].width, img.data[i][mip].height, 1, 0, mip);
				}
			}
			else
			{
				temp_img.unload();
				std::cout << "ERROR::Image must contain at least one miplevel" << std::endl;
				return false;
			}

			handler.mip_levels = 0;
			compressor.process(input_options, compression_options, output_options);
			temp_img.unload();
		}

		f.close();

		return true;
	}

//    bool export_texture(const char* input,
//        const char* output,
//        CompressionType compression = COMPRESSION_NONE,
//        bool normal_map = false,
//        int output_mips = 0)
//    {
//        std::string ext = extension(input);
//
//        if (ext == "hdr")
//        {
//            Image<float> img;
//
//            if (!import_image(img, input))
//                return false;
//
//            bool result = export_image(img, output, PIXEL_TYPE_DEFAULT, compression, normal_map, output_mips);
//            img.unload();
//
//            return result;
//        }
//        else
//        {
//            Image<uint8_t> img;
//
//            if (!import_image(img, input))
//                return false;
//
//            bool result = export_image(img, output, PIXEL_TYPE_DEFAULT, compression, normal_map, output_mips);
//            img.unload();
//
//            return result;
//        }
//    }

	template<typename T>
	bool cubemap_from_latlong(cmft::Image& dst, Image<T>& src)
	{
		cmft::Image img;
		img.m_width = uint32_t(src.data[0][0].width);
		img.m_height = uint32_t(src.data[0][0].height);
		img.m_dataSize = src.size(0, 0);

		if (src.components < 3)
		{
			std::cout << "ERROR::Image must at least have 3 color channels" << std::endl;
			return false;
		}

		if (src.components == 4)
		{
			if (std::is_same<T, uint8_t>::value)
				img.m_format = cmft::TextureFormat::RGBA8;
			else if (std::is_same<T, uint16_t>::value)
				img.m_format = cmft::TextureFormat::RGBA16F;
			else if (std::is_same<T, float>::value)
				img.m_format = cmft::TextureFormat::RGBA32F;
		}
		else if (src.components == 3)
		{
			if (std::is_same<T, uint8_t>::value)
				img.m_format = cmft::TextureFormat::RGB8;
			else if (std::is_same<T, uint16_t>::value)
				img.m_format = cmft::TextureFormat::RGB16F;
			else if (std::is_same<T, float>::value)
				img.m_format = cmft::TextureFormat::RGB32F;
		}

		img.m_numMips = 1;
		img.m_numFaces = 1;
		img.m_data = (void*)src.data[0][0].data;

		if (!cmft::imageIsLatLong(img))
		{
			std::cout << "ERROR::Image is not in latlong format" << std::endl;
			return false;
		}

		if (!cmft::imageCubemapFromLatLong(dst, img))
		{
			std::cout << "ERROR::Failed to convert Cubemap" << std::endl;
			return false;
		}

		return true;
	}

	template<typename T>
	bool cubemap_from_latlong(Image<T>& src,
		const char* name,
		const char* output,
		PixelType pixel_type = PIXEL_TYPE_DEFAULT,
		CompressionType compression = COMPRESSION_NONE,
		bool mipmap = false,
		bool irradiance = false,
		bool radiance = false)
	{
		cmft::Image cmft_cube;

		if (!cubemap_from_latlong(cmft_cube, src))
		{
			std::cout << "ERROR::Failed to convert Cubemap" << std::endl;
			return false;
		}

		if (irradiance)
		{
			cmft::Image cmft_irradiance_cube;

			if (!cmft::imageIrradianceFilterSh(cmft_irradiance_cube, 128, cmft_cube))
			{
				std::cout << "ERROR::Failed to generate irradiance map!" << std::endl;
				return false;
			}

			Image<T> irradiance_cube;

			uint32_t img_offsets[CUBE_FACE_NUM][MAX_MIP_NUM];
			cmft::imageGetMipOffsets(img_offsets, cmft_irradiance_cube);

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

			std::string out = "irradiance.trm";
            
            ImageExportOptions irradiance_exp_options;
            
            irradiance_exp_options.compression = compression;
            irradiance_exp_options.normal_map = false;
            irradiance_exp_options.output_mips = 0;
            irradiance_exp_options.path = out;
            
			if (!export_image(irradiance_cube, irradiance_exp_options))
			{
				std::cout << "ERROR::Failed to export Cubemap" << std::endl;
				return false;
			}

			cmft::imageUnload(cmft_irradiance_cube);
		}

		if (radiance)
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

			Image<T> radiance_cube;

			uint32_t img_offsets[CUBE_FACE_NUM][MAX_MIP_NUM];
			cmft::imageGetMipOffsets(img_offsets, cmft_radiance_cube);

			radiance_cube.components = src.components;
			radiance_cube.array_slices = 6;
			radiance_cube.mip_slices = 7;

			for (int i = 0; i < 6; i++)
			{
				for (int j = 0; j < 7; j++)
				{
					uint8_t* mip_data = (uint8_t*)cmft_radiance_cube.m_data + img_offsets[i][j];

					radiance_cube.data[i][j].data = (T*)mip_data;
					radiance_cube.data[i][j].height = cmft_radiance_cube.m_height >> j;
					radiance_cube.data[i][j].width = cmft_radiance_cube.m_width >> j;
				}
			}

			std::string out = "radiance.trm";
            
            ImageExportOptions radiance_exp_options;
            
            radiance_exp_options.compression = compression;
            radiance_exp_options.normal_map = false;
            radiance_exp_options.output_mips = 0;
            radiance_exp_options.path = out;

			if (!export_image(radiance_cube, radiance_exp_options))
			{
				std::cout << "ERROR::Failed to export Cubemap" << std::endl;
				return false;
			}

			cmft::imageUnload(cmft_radiance_cube);
		}

		uint32_t img_offsets[CUBE_FACE_NUM][MAX_MIP_NUM];
		cmft::imageGetMipOffsets(img_offsets, cmft_cube);

		Image<T> cubemap;

		cubemap.components = src.components;
		cubemap.array_slices = 6;
		cubemap.mip_slices = 1;

		for (int i = 0; i < 6; i++)
		{
			uint8_t* mip_data = (uint8_t*)cmft_cube.m_data + img_offsets[i][0];

			cubemap.data[i][0].data = (T*)mip_data;
			cubemap.data[i][0].height = cmft_cube.m_height;
			cubemap.data[i][0].width = cmft_cube.m_width;
		}
        
        ImageExportOptions exp_options;
        
        exp_options.compression = compression;
        exp_options.normal_map = false;
        exp_options.output_mips = 0;
        exp_options.pixel_type = pixel_type;
        exp_options.path = output;

		if (!export_image(cubemap, exp_options))
		{
			std::cout << "ERROR::Failed to export Cubemap" << std::endl;
			return false;
		}

		cmft::imageUnload(cmft_cube);
		src.unload();

		return true;
	}

//    bool cubemap_from_latlong(const char* input,
//        const char* output,
//        PixelType pixel_type = PIXEL_TYPE_DEFAULT,
//        CompressionType compression = COMPRESSION_NONE,
//        bool mipmap = false,
//        bool irradiance = false,
//        bool radiance = false)
//    {
//        Image<float> src;
//
//        if (!image_load(src, input))
//        {
//            std::cout << "ERROR::Failed to open Cubemap" << std::endl;
//            return false;
//        }
//
//        return cubemap_from_latlong<float>(src, input, output, pixel_type, compression, mipmap, irradiance, radiance);
//    }
};
