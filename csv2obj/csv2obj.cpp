﻿#include <istream>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <array>

void printHelp()
{
	std::cout << "Using: csv2obj input.csv" << std::endl;
}

using FloatVector = std::array<float, 3>;

struct ObjFile
{
	std::vector<FloatVector> vertices;
	std::vector<FloatVector> texCoords;
};

void cleanAndSplit(char buffer[], std::vector<std::string>& values)
{
	char* end = std::remove(buffer, buffer + strlen(buffer), ',');
	*end = 0;

	std::istringstream iss(buffer);
	values = std::vector<std::string>(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
}

uint64_t gl_Position_x = std::hash<std::string>().operator()("in_POSITION0.x");
uint64_t gl_Position_y = std::hash<std::string>().operator()("in_POSITION0.y");
uint64_t gl_Position_z = std::hash<std::string>().operator()("in_POSITION0.z");
uint64_t v_texcoord_x = std::hash<std::string>().operator()("in_TEXCOORD0.x");
uint64_t v_texcoord_y = std::hash<std::string>().operator()("in_TEXCOORD0.y");

void distributeValues(const std::vector<uint64_t>& header, const std::vector<std::string>& values, ObjFile& objFile)
{
	objFile.vertices.emplace_back();
	objFile.texCoords.emplace_back();
	FloatVector& vertex = objFile.vertices.back();
	FloatVector& texCoord = objFile.texCoords.back();

	size_t i = 0;
	assert(header.size() == values.size());
	for (const uint64_t& h : header)
	{
		if (h == gl_Position_x)
		{
			vertex[0] = std::stof(values[i]);
		}
		else if (h == gl_Position_y)
		{
			vertex[1] = std::stof(values[i]);
		}
		else if (h == gl_Position_z)
		{
			vertex[2] = std::stof(values[i]);
		}
		else if (h == v_texcoord_x)
		{
			texCoord[0] = std::stof(values[i]);
		}
		else if (h == v_texcoord_y)
		{
			texCoord[1] = std::stof(values[i]);
		}
		++i;
	}
}

void writeObj(const ObjFile& obj, const char* outFileName)
{
	std::ofstream fOut(outFileName);
	for (const auto& v : obj.vertices)
		fOut << "v " << v[0] << " " << v[1] << " " << v[2] << std::endl;
	for (const auto& v : obj.texCoords)
		fOut << "vt " << v[0] << " " << v[1] << std::endl;
	fOut << "g default" << std::endl;

	size_t trianglesCount = obj.vertices.size() / 3;
	for (size_t i = 0; i < trianglesCount; ++i)
	{
		fOut << "f " << 3 * i + 1 << "/" << 3 * i + 1 << " ";
		fOut << 3 * i + 2 << "/" << 3 * i + 2 << " ";
		fOut << 3 * i + 3 << "/" << 3 * i + 3 << std::endl;
	}

	fOut.close();
}

int main(int argc, const char* argv[])
{
	if (argc != 2)
	{
		printHelp();
		return 1;
	}

	size_t fileNameSize = strlen(argv[1]);
	if (strcmp(argv[1] + fileNameSize - 4, ".csv"))
	{
		printHelp();
		return 2;
	}

	ObjFile output;

	char outputFileName[2048] = {};
	snprintf(outputFileName, sizeof(outputFileName), "%s", argv[1]);
	snprintf(outputFileName + fileNameSize - 4, 5, ".obj");

	char buffer[2048] = {};
	const char* inputFileName = argv[1];
	std::ifstream fIn(inputFileName);
	fIn.getline(buffer, sizeof(buffer));

	std::vector<std::string> headerValues;
	cleanAndSplit(buffer, headerValues);
	std::vector<uint64_t> hashes;
	for (const std::string& h : headerValues)
		hashes.emplace_back(std::hash<std::string>().operator()(h));

	while (!fIn.eof())
	{
		fIn.getline(buffer, sizeof(buffer));
		if (strlen(buffer) == 0)
			break;

		std::vector<std::string> values;
		cleanAndSplit(buffer, values);
		distributeValues(hashes, values, output);
	}

	fIn.close();

	writeObj(output, outputFileName);

	return 0;
}
