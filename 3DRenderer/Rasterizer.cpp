#pragma once
#include "Rasterizer.hpp"

Rasterizer::Rasterizer(int newWindowWidth, int newWindowHeight, int fov, float cNear, float cFar) :
	wWidth(newWindowWidth), wHeight(newWindowHeight)
{
	float aspectRatio = wWidth / wHeight;

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			pMat[i][j] = 0;
		}
	}

	pMat[0][0] = aspectRatio * 1.0 / tan(fov * 0.5f / 180.0f * 3.14159f);
	pMat[1][1] = 1.0 / tan(fov * 0.5f / 180.0f * 3.14159f);
	pMat[2][2] = cFar / (cFar - cNear);
	pMat[2][3] = 1;
	pMat[3][2] = (-cFar * cNear) / (cFar - cNear);
	pMat[3][3] = 0;

}

void Rasterizer::projectTriangle(Triangle& tri) const
{
	//Perspective divide
	func::vecXmatrixProjective(tri.transVerts[0], pMat, tri.projVerts[0]);
	func::vecXmatrixProjective(tri.transVerts[1], pMat, tri.projVerts[1]);
	func::vecXmatrixProjective(tri.transVerts[2], pMat, tri.projVerts[2]);

	//Normalize to NDC space
	for (int i = 0; i < 3; ++i)
	{
		tri.projVerts[i].x = floor((tri.projVerts[i].x + 1) / 2 * wWidth);
		tri.projVerts[i].y = floor((1 - tri.projVerts[i].y) / 2 * wHeight);
	}
}

void Rasterizer::calculateBoundingBox(Triangle& tri) const
{
	tri.bLeft = std::max(0.0f, std::min({ tri.projVerts[0].x, tri.projVerts[1].x, tri.projVerts[2].x, (float)wWidth - 1 }));
	tri.bTop = std::max(0.0f, std::min({ tri.projVerts[0].y, tri.projVerts[1].y, tri.projVerts[2].y, (float)wHeight - 1 }));
	tri.bRight = std::min((float)wWidth - 1, std::max({ tri.projVerts[0].x, tri.projVerts[1].x, tri.projVerts[2].x, 0.0f }));
	tri.bBot = std::min((float)wHeight - 1, std::max({ tri.projVerts[0].y, tri.projVerts[1].y, tri.projVerts[2].y, 0.0f }));
}

void Rasterizer::calculateVertexData(Triangle& tri) const
{
	tri.vertexDepth[0] = 1 / tri.transVerts[0].z;
	tri.vertexDepth[1] = 1 / tri.transVerts[1].z;
	tri.vertexDepth[2] = 1 / tri.transVerts[2].z;

	tri.tx[0] = tri.tCoords[0].x / tri.transVerts[0].z; tri.tx[1] = tri.tCoords[1].x / tri.transVerts[1].z; tri.tx[2] = tri.tCoords[2].x / tri.transVerts[2].z;
	tri.ty[0] = tri.tCoords[0].y / tri.transVerts[0].z; tri.ty[1] = tri.tCoords[1].y / tri.transVerts[1].z; tri.ty[2] = tri.tCoords[2].y / tri.transVerts[2].z;
	tri.area = func::edgeFunc(vec2(tri.projVerts[0].x, tri.projVerts[0].y), tri.projVerts[1], tri.projVerts[2]);
}

void Rasterizer::rasterTile(const Tile& tile, std::vector<std::vector<float>>& zBuffer, sf::Uint8*& pixelBuffer) const
{
	int count = 0;
	//Using AVX2 instructions
	__m256 factor = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);
	for (auto& triangle : tile.trianglesToRender)
	{


		//We clamp the dimensions of the triangle's bounding box to the dimensions of the tile. 
		//This ensures we don't check pixels outside the tile.
		int bLeft = std::max(tile.bLeft, std::min(triangle->bLeft, tile.bRight));
		int bTop = std::max(tile.bTop, std::min(triangle->bTop, tile.bBot));
		int bRight = std::min(tile.bRight, std::max(triangle->bRight, tile.bLeft));
		int bBot = std::min(tile.bBot, std::max(triangle->bBot, tile.bTop));

		//Instead of calculating the edge function per pixel, we can decrease the number of calculations
		//by adding increments to the original edge function results. We also vectorize this using avx2
		//For more information on how this works, see https://fgiesen.wordpress.com/2013/02/10/optimizing-the-basic-rasterizer/
		float a0 = (triangle->projVerts[2].y - triangle->projVerts[1].y), b0 = (triangle->projVerts[1].x - triangle->projVerts[2].x);
		float a1 = (triangle->projVerts[0].y - triangle->projVerts[2].y), b1 = (triangle->projVerts[2].x - triangle->projVerts[0].x);
		float a2 = (triangle->projVerts[1].y - triangle->projVerts[0].y), b2 = (triangle->projVerts[0].x - triangle->projVerts[1].x);

		__m256 A0s = _mm256_mul_ps(_mm256_set1_ps(a0), factor);
		__m256 A1s = _mm256_mul_ps(_mm256_set1_ps(a1), factor);
		__m256 A2s = _mm256_mul_ps(_mm256_set1_ps(a2), factor);
		float incA0 = a0 * 8;
		float incA1 = a1 * 8;
		float incA2 = a2 * 8;

		vec2 pixel(std::max(bLeft, 0), std::max(0, bTop));
		float w0r = func::edgeFunc(pixel, triangle->projVerts[1], triangle->projVerts[2]);
		float w1r = func::edgeFunc(pixel, triangle->projVerts[2], triangle->projVerts[0]);
		float w2r = func::edgeFunc(pixel, triangle->projVerts[0], triangle->projVerts[1]);

		//Loop over each row in triangle's bounding box
		for (int i = bTop; i < bBot; i += 1)
		{
			__m256 w0 = _mm256_add_ps(A0s, _mm256_set1_ps(w0r));
			__m256 w1 = _mm256_add_ps(A1s, _mm256_set1_ps(w1r));
			__m256 w2 = _mm256_add_ps(A2s, _mm256_set1_ps(w2r));

			float* w0Results = (float*)&w0;
			float* w1Results = (float*)&w1;
			float* w2Results = (float*)&w2;

			//Each pixel
			for (int j = bLeft; j < bRight; j += 8)
			{
				//AVX2 vectorizes everything 8 32 bit components, so we calculate some of the data 8 pixels at a time
				for (int k = 0; k < 8 && k + j < bRight; ++k)
				{
					if ((w0Results[k] <= 0 && w1Results[k] <= 0 && w2Results[k] <= 0) || (w0Results[k] >= 0 && w1Results[k] >= 0 && w2Results[k] >= 0))
					{
						float barycentricCoords0 = w0Results[k] / (triangle->area);
						float barycentricCoords1 = w1Results[k] / (triangle->area);
						float barycentricCoords2 = w2Results[k] / (triangle->area);

						//We find our depth at each pixel by interpolating w/ barycentric coordinates & vertex depth
						float z = barycentricCoords0 * triangle->vertexDepth[0] +
							barycentricCoords1 * triangle->vertexDepth[1] +
							barycentricCoords2 * triangle->vertexDepth[2];

						vec2 texel;

						float inverseZ = 1 / z;
						int zindex = k + j;
						if (inverseZ < zBuffer[zindex][i])
						{
							int index = (zindex + i * wWidth) * 4;

							//calculating pixel coordinates
							float u = (triangle->tx[0] * barycentricCoords0) +
								(triangle->tx[1] * barycentricCoords1) +
								(triangle->tx[2] * barycentricCoords2);

							float v = (triangle->ty[0] * barycentricCoords0) +
								(triangle->ty[1] * barycentricCoords1) +
								(triangle->ty[2] * barycentricCoords2);


							texel.x = u / z;
							texel.y = v / z;


							if (texel.x < 0 || texel.x > 0)
								texel.x = texel.x - floor(texel.x);
							if (texel.y < 0 || texel.y > 0)
								texel.y = texel.y - floor(texel.y);

							texel.x *= (triangle->tWidth - 1);
							texel.y *= (triangle->tHeight - 1);


							int index2 = (floor(texel.x) + floor(texel.y) * triangle->tWidth) * 4;

							//index2 + 3 holds the "A" components of RGBA. For now, we only render if this value is set to >0, 
							//meaning there is some transparency. (note we render everything partially transparent as fully transparent)
							if (triangle->triangleTexture[index2 + 3] > 0)
							{
								zBuffer[zindex][i] = inverseZ;
								pixelBuffer[index] = triangle->triangleTexture[index2];
								pixelBuffer[index + 1] = triangle->triangleTexture[index2 + 1];
								pixelBuffer[index + 2] = triangle->triangleTexture[index2 + 2];
								pixelBuffer[index + 3] = triangle->triangleTexture[index2 + 3];
							}
						}
					}
				}
				w0 = _mm256_add_ps(w0, _mm256_set1_ps(incA0));
				w1 = _mm256_add_ps(w1, _mm256_set1_ps(incA1));
				w2 = _mm256_add_ps(w2, _mm256_set1_ps(incA2));
			}
			w0r += b0;
			w1r += b1;
			w2r += b2;
		}
	}

}