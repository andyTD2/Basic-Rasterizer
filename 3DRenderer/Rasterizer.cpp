#pragma once
#include "Rasterizer.hpp"



Rasterizer::Rasterizer(int newWindowWidth, int newWindowHeight, int fov, float cNear, float cFar)
{
	wWidth = newWindowWidth;
	wHeight = newWindowHeight;
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

bool Rasterizer::project_triangle(Triangle& tri)
{
	func::vecXmatrix(tri.transVerts[0], pMat, tri.projVerts[0], true);
	func::vecXmatrix(tri.transVerts[1], pMat, tri.projVerts[1], true);
	func::vecXmatrix(tri.transVerts[2], pMat, tri.projVerts[2], true);


	for (int i = 0; i < 3; ++i)
	{
		tri.projVerts[i].x = floor((tri.projVerts[i].x + 1) / 2 * wWidth);
		tri.projVerts[i].y = floor((1 - tri.projVerts[i].y) / 2 * wHeight);
	}

	return true;
}

void Rasterizer::calculateBoundingBox(Triangle& tri)
{
	tri.bLeft = std::max(0.0f, std::min({ tri.projVerts[0].x, tri.projVerts[1].x, tri.projVerts[2].x, (float)wWidth - 1 }));
	tri.bTop = std::max(0.0f, std::min({ tri.projVerts[0].y, tri.projVerts[1].y, tri.projVerts[2].y, (float)wHeight - 1 }));
	tri.bRight = std::min((float)wWidth - 1, std::max({ tri.projVerts[0].x, tri.projVerts[1].x, tri.projVerts[2].x, 0.0f }));
	tri.bBot = std::min((float)wHeight - 1, std::max({ tri.projVerts[0].y, tri.projVerts[1].y, tri.projVerts[2].y, 0.0f }));
}

void Rasterizer::calculateVertexData(Triangle& tri)
{
	tri.vertexDepth[0] = 1 / tri.transVerts[0].z;
	tri.vertexDepth[1] = 1 / tri.transVerts[1].z;
	tri.vertexDepth[2] = 1 / tri.transVerts[2].z;


	tri.tx[0] = tri.tCoords[0].x / tri.transVerts[0].z; tri.tx[1] = tri.tCoords[1].x / tri.transVerts[1].z; tri.tx[2] = tri.tCoords[2].x / tri.transVerts[2].z;
	tri.ty[0] = tri.tCoords[0].y / tri.transVerts[0].z; tri.ty[1] = tri.tCoords[1].y / tri.transVerts[1].z; tri.ty[2] = tri.tCoords[2].y / tri.transVerts[2].z;
	tri.area = func::edge_f(vec2(tri.projVerts[0].x, tri.projVerts[0].y), tri.projVerts[1], tri.projVerts[2]);
}

void Rasterizer::rasterTile(Tile& tile, std::vector<std::vector<float>>& z_buffer, sf::Uint8*& buffer)
{
	__m256 factor = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);
	for (auto& triangle : tile.trianglesToRender)
	{
		int bLeft = std::max(tile.bLeft, std::min(triangle->bLeft, tile.bRight));
		int bTop = std::max(tile.bTop, std::min(triangle->bTop, tile.bBot));
		int bRight = std::min(tile.bRight, std::max(triangle->bRight, tile.bLeft));
		int bBot = std::min(tile.bBot, std::max(triangle->bBot, tile.bTop));

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
		float w0r = func::edge_f(pixel, triangle->projVerts[1], triangle->projVerts[2]);
		float w1r = func::edge_f(pixel, triangle->projVerts[2], triangle->projVerts[0]);
		float w2r = func::edge_f(pixel, triangle->projVerts[0], triangle->projVerts[1]);

		for (int i = bTop; i < bBot; i += 1)
		{
			__m256 w0 = _mm256_add_ps(A0s, _mm256_set1_ps(w0r));
			__m256 w1 = _mm256_add_ps(A1s, _mm256_set1_ps(w1r));
			__m256 w2 = _mm256_add_ps(A2s, _mm256_set1_ps(w2r));

			float* one = (float*)&w0;
			float* two = (float*)&w1;
			float* three = (float*)&w2;
			for (int j = bLeft; j < bRight; j += 8)
			{
				for (int k = 0; k < 8 && k + j < bRight; ++k)
				{
					if ((one[k] <= 0 && two[k] <= 0 && three[k] <= 0) || (one[k] >= 0 && two[k] >= 0 && three[k] >= 0))
					{
						float b0t = one[k] / (triangle->area);
						float b1t = two[k] / (triangle->area);
						float b2t = three[k] / (triangle->area);

						float z = b0t * triangle->vertexDepth[0] +
							b1t * triangle->vertexDepth[1] +
							b2t * triangle->vertexDepth[2];

						vec2 texel;

						float inv_z = 1 / z;
						int zindex = k + j;
						if (inv_z < z_buffer[zindex][i])
						{
							int index = (zindex + i * wWidth) * 4;

							float u = (triangle->tx[0] * b0t) +
								(triangle->tx[1] * b1t) +
								(triangle->tx[2] * b2t);

							float v = (triangle->ty[0] * b0t) +
								(triangle->ty[1] * b1t) +
								(triangle->ty[2] * b2t);


							texel.x = u / z;
							texel.y = v / z;


							if (texel.x < 0 || texel.x > 0)
								texel.x = texel.x - floor(texel.x);
							if (texel.y < 0 || texel.y > 0)
								texel.y = texel.y - floor(texel.y);

							texel.x *= (triangle->tWidth - 1);
							texel.y *= (triangle->tHeight - 1);



							int index2 = (floor(texel.x) + floor(texel.y) * triangle->tWidth) * 4;
							if (triangle->triangleTexture[index2 + 3] > 0)
							{
								z_buffer[zindex][i] = inv_z;
								buffer[index] = triangle->triangleTexture[index2];
								buffer[index + 1] = triangle->triangleTexture[index2 + 1];
								buffer[index + 2] = triangle->triangleTexture[index2 + 2];
								buffer[index + 3] = triangle->triangleTexture[index2 + 3];
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