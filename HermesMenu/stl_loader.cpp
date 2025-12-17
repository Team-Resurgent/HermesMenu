#include "stl_loader.h"
#include "stringUtility.h"
#include <string.h>
#include <stdlib.h>

utils::dataContainer* stl_loader::loadFromBinary(const unsigned char* data, uint32_t size)
{
	if (data == NULL || size == 0)
	{
		return NULL;
	}

	// Binary STL format:
	// - 80-byte header (ignored)
	// - 4-byte unsigned integer (number of triangles)
	// - For each triangle:
	//   - 12 bytes: normal vector (3 floats)
	//   - 12 bytes: vertex 1 (3 floats)
	//   - 12 bytes: vertex 2 (3 floats)
	//   - 12 bytes: vertex 3 (3 floats)
	//   - 2 bytes: attribute byte count (ignored)

	const uint32_t HEADER_SIZE = 80;
	const uint32_t TRIANGLE_COUNT_SIZE = 4;
	const uint32_t TRIANGLE_SIZE = 50; // 12 + 12 + 12 + 12 + 2

	if (size < HEADER_SIZE + TRIANGLE_COUNT_SIZE)
	{
		return NULL;
	}

	// Skip 80-byte header
	const unsigned char* readPos = data + HEADER_SIZE;

	// Read triangle count (little-endian)
	uint32_t triangleCount = 0;
	triangleCount |= ((uint32_t)readPos[0]);
	triangleCount |= ((uint32_t)readPos[1]) << 8;
	triangleCount |= ((uint32_t)readPos[2]) << 16;
	triangleCount |= ((uint32_t)readPos[3]) << 24;
	readPos += TRIANGLE_COUNT_SIZE;

	// Verify we have enough data
	uint32_t expectedSize = HEADER_SIZE + TRIANGLE_COUNT_SIZE + (triangleCount * TRIANGLE_SIZE);
	if (size < expectedSize || triangleCount == 0)
	{
		return NULL;
	}

	// Allocate vertex buffer (3 vertices per triangle)
	utils::dataContainer* vertices = new utils::dataContainer();
	vertices->size = triangleCount * 3 * sizeof(meshUtility::vertex);
	vertices->data = (char*)malloc(vertices->size);
	
	if (vertices->data == NULL)
	{
		delete vertices;
		return NULL;
	}

	meshUtility::vertex* vertexArray = (meshUtility::vertex*)vertices->data;
	uint32_t vertexIndex = 0;

	// Parse each triangle
	for (uint32_t i = 0; i < triangleCount; i++)
	{
		// Skip normal vector (12 bytes)
		readPos += 12;

		// Read vertex 1
		float v1x, v1y, v1z;
		memcpy(&v1x, readPos, sizeof(float));
		memcpy(&v1y, readPos + 4, sizeof(float));
		memcpy(&v1z, readPos + 8, sizeof(float));
		readPos += 12;

		// Read vertex 2
		float v2x, v2y, v2z;
		memcpy(&v2x, readPos, sizeof(float));
		memcpy(&v2y, readPos + 4, sizeof(float));
		memcpy(&v2z, readPos + 8, sizeof(float));
		readPos += 12;

		// Read vertex 3
		float v3x, v3y, v3z;
		memcpy(&v3x, readPos, sizeof(float));
		memcpy(&v3y, readPos + 4, sizeof(float));
		memcpy(&v3z, readPos + 8, sizeof(float));
		readPos += 12;

		// Skip attribute byte count (2 bytes)
		readPos += 2;

		// Create vertices (STL doesn't have UV coordinates, so set to 0,0)
		vertexArray[vertexIndex++] = meshUtility::vertex(v1x, v1y, v1z, 0.0f, 0.0f);
		vertexArray[vertexIndex++] = meshUtility::vertex(v2x, v2y, v2z, 0.0f, 0.0f);
		vertexArray[vertexIndex++] = meshUtility::vertex(v3x, v3y, v3z, 0.0f, 0.0f);
	}

	return vertices;
}
