#include <offline/image_exporter.h>
#include <common/filesystem.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <glm.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define BRDF_LUT_SIZE 512

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.

float radical_inverse_vdc(uint32_t bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// ----------------------------------------------------------------------------

glm::vec2 hammersley(uint32_t i, uint32_t N)
{
	return glm::vec2(float(i) / float(N), radical_inverse_vdc(i));
}

// ----------------------------------------------------------------------------

glm::vec3 importance_sample_ggx(glm::vec2 Xi, glm::vec3 N, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates - halfway vector
	glm::vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space H vector to world-space sample vector
	glm::vec3 up = abs(N.z) < 0.999f ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 tangent = normalize(cross(up, N));
	glm::vec3 bitangent = cross(N, tangent);

	glm::vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

// ----------------------------------------------------------------------------

float geometry_schlick_ggx(float NdotV, float roughness)
{
	// note that we use a different k for IBL
	float a = roughness;
	float k = (a * a) / 2.0f;

	float nom = NdotV;
	float denom = NdotV * (1.0f - k) + k;

	return nom / denom;
}

// ----------------------------------------------------------------------------

float geometry_smith(glm::vec3 N, glm::vec3 V, glm::vec3 L, float roughness)
{
	float NdotV = std::max(glm::dot(N, V), 0.0f);
	float NdotL = std::max(glm::dot(N, L), 0.0f);
	float ggx2 = geometry_schlick_ggx(NdotV, roughness);
	float ggx1 = geometry_schlick_ggx(NdotL, roughness);

	return ggx1 * ggx2;
}

// ----------------------------------------------------------------------------

glm::vec2 integrate_brdf(float NdotV, float roughness)
{
	glm::vec3 V;
	V.x = sqrt(1.0f - NdotV * NdotV);
	V.y = 0.0f;
	V.z = NdotV;

	float A = 0.0f;
	float B = 0.0f;

	glm::vec3 N = glm::vec3(0.0f, 0.0f, 1.0f);

	const uint32_t SAMPLE_COUNT = 1024u;

	for (uint32_t i = 0u; i < SAMPLE_COUNT; ++i)
	{
		// generates a sample vector that's biased towards the
		// preferred alignment direction (importance sampling).
		glm::vec2 Xi = hammersley(i, SAMPLE_COUNT);
		glm::vec3 H = importance_sample_ggx(Xi, N, roughness);
		glm::vec3 L = glm::normalize(2.0f * glm::dot(V, H) * H - V);

		float NdotL = std::max(L.z, 0.0f);
		float NdotH = std::max(H.z, 0.0f);
		float VdotH = std::max(dot(V, H), 0.0f);

		if (NdotL > 0.0)
		{
			float G = geometry_smith(N, V, L, roughness);
			float G_Vis = (G * VdotH) / (NdotH * NdotV);
			float Fc = pow(1.0 - VdotH, 5.0);

			A += (1.0 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}

	A /= float(SAMPLE_COUNT);
	B /= float(SAMPLE_COUNT);

	return glm::vec2(A, B);
}

void print_usage()
{
	printf("usage: brdf_lut [outpath]\n\n");
}

int main(int argc, char * argv[])
{
    if (argc == 1)
    {
		print_usage();
        return 1;
    }
	else
	{
		std::string output = argv[1];

		if (output.size() == 0)
		{
			printf("ERROR: Invalid output path: %s\n\n", argv[1]);
			print_usage();

			return 1;
		}

		std::vector<glm::vec3> brdf;
		brdf.resize(BRDF_LUT_SIZE * BRDF_LUT_SIZE);

#ifndef __APPLE__
		#pragma omp parallel for
#endif
		for (int32_t y = 0; y < BRDF_LUT_SIZE; y++)
		{
			for (int32_t x = 0; x < BRDF_LUT_SIZE; x++)
			{
				glm::vec2 tex_coord = glm::vec2(float(x), float(y)) / float(BRDF_LUT_SIZE - 1);

				brdf[(BRDF_LUT_SIZE - 1 - y) * BRDF_LUT_SIZE + x] = glm::vec3(integrate_brdf(tex_coord.x, tex_coord.y), 0.0f);
			}
		}

		if (stbi_write_hdr(output.c_str(), BRDF_LUT_SIZE, BRDF_LUT_SIZE, 3, &brdf[0].x) == 0)
			std::cout << "Failed to output BRDF LUT: " + output << std::endl;
		
		return 0;
	}
}
