#pragma once
#include "Rasterizer.hpp"



Rasterizer::Rasterizer(int _w_width, int _w_height, float _c_near, float _c_far, int _fov) :
	w_width(_w_width), w_height(_w_height), c_near(_c_near), c_far(_c_far), fov(_fov)
{
	const float pi = 3.141592653;
	aspect_ratio = w_width / w_height;

	//canvas size and dimensions
	nWidth = tan((fov / 2) * pi / 180) * c_near * 2;
	nHeight = nWidth;
	fWidth = tan((fov / 2) * pi / 180) * c_far * 2;
	fHeight = fWidth;

	c_top = nHeight / 2;
	c_right = nWidth / 2;
	c_bot = -c_top;
	c_left = -c_right;

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			p_mat[i][j] = 0;
		}
	}

	//projection matrix
	//p_mat[0][0] = aspect_ratio * 1.0 / tan(fov * 0.5f / 180.0f * 3.14159f);
	//p_mat[1][1] = 1.0 / tan(fov * 0.5f / 180.0f * 3.14159f);
	//p_mat[2][2] = -c_far / (c_far - c_near);
	//p_mat[2][3] = 1;
	//p_mat[3][2] = -c_far * c_near / (c_far - c_near);
	//p_mat[3][3] = 0;

	p_mat[0][0] = (2 * c_near) / (c_right - c_left);
	p_mat[1][1] = (2 * c_near) / (c_top - c_bot);
	p_mat[2][0] = (c_right + c_left) / (c_right - c_left);
	p_mat[2][1] = (c_top + c_bot) / (c_top - c_bot);
	p_mat[2][2] = -(c_far + c_near) / (c_far - c_near);
	p_mat[2][3] = 1;
	p_mat[3][2] = -(2 * c_far * c_near) / (c_far - c_near);
	p_mat[3][3] = 0;

}

bool Rasterizer::project_triangle(Triangle& tri, float mat[4][4])
{
	func::vecXmatrix(tri.transVerts[0], mat, tri.projVerts[0], true);
	func::vecXmatrix(tri.transVerts[1], mat, tri.projVerts[1], true);
	func::vecXmatrix(tri.transVerts[2], mat, tri.projVerts[2], true);


	//if (tri.projVerts[0].x > tri.projVerts[0].w && tri.projVerts[1].x > tri.projVerts[1].w && tri.projVerts[2].x > tri.projVerts[2].w)
	//	return false;
	//if (tri.projVerts[0].x < -tri.projVerts[0].w && tri.projVerts[1].x < -tri.projVerts[1].w && tri.projVerts[2].x < -tri.projVerts[2].w)
	//	return false;
	//if (tri.projVerts[0].y > tri.projVerts[0].w && tri.projVerts[1].y > tri.projVerts[1].w && tri.projVerts[2].y > tri.projVerts[2].w)
	//	return false;
	//if (tri.projVerts[0].y < -tri.projVerts[0].w && tri.projVerts[1].y < -tri.projVerts[1].w && tri.projVerts[2].y < -tri.projVerts[2].w)
	//	return false;



	for (int i = 0; i < 3; ++i)
	{
		tri.projVerts[i].x = floor((tri.projVerts[i].x + 1) / 2 * w_width);
		tri.projVerts[i].y = floor((1 - tri.projVerts[i].y) / 2 * w_height);
	}
	tri.bLeft = std::min({ tri.projVerts[0].x, tri.projVerts[1].x, tri.projVerts[2].x });
	tri.bTop = std::min({ tri.projVerts[0].y, tri.projVerts[1].y, tri.projVerts[2].y });
	tri.bRight = std::max({ tri.projVerts[0].x, tri.projVerts[1].x, tri.projVerts[2].x });
	tri.bBot = std::max({ tri.projVerts[0].y, tri.projVerts[1].y, tri.projVerts[2].y });
	//out.push_back(&tri);
	return true;
}


int Rasterizer::clip_triangle_near(Triangle& tri, std::vector<Triangle*>& outputTris, std::vector<Triangle*>& trisToDelete) const
{
	if (tri.transVerts[0].z >= c_near && tri.transVerts[1].z >= c_near && tri.transVerts[2].z >= c_near)
	{
		outputTris.push_back(&tri);
		return 1;
	}
	else if (tri.transVerts[0].z < c_near && tri.transVerts[1].z < c_near && tri.transVerts[2].z < c_near)
	{
		return 0;
	}

	std::vector<vec4> clippedVertices;
	std::vector<vec4> safeVertices;
	std::vector<vec2> clippedTVertices;
	std::vector<vec2> safeTVertices;

	for (int i = 0; i < 3; ++i)
	{
		if (tri.transVerts[i].z < c_near)
		{
			clippedVertices.push_back(tri.transVerts[i]);
			clippedTVertices.push_back(tri.tCoords[i]);
		}
		else
		{
			safeVertices.push_back(tri.transVerts[i]);
			safeTVertices.push_back(tri.tCoords[i]);
		}
	}

	// only need to create one new triangle
	if (clippedVertices.size() == 2)
	{
		float t;
		vec4 one = func::getIntersection(vec4(0, 0, c_near), vec4(0, 0, 1), safeVertices[0], clippedVertices[0], t);
		vec2 newT1;
		newT1.x = safeTVertices[0].x + (t * (clippedTVertices[0].x - safeTVertices[0].x));
		newT1.y = safeTVertices[0].y + (t * (clippedTVertices[0].y - safeTVertices[0].y));

		vec4 two = func::getIntersection(vec4(0, 0, c_near), vec4(0, 0, 1), safeVertices[0], clippedVertices[1], t);
		vec2 newT2;
		newT2.x = safeTVertices[0].x + (t * (clippedTVertices[1].x - safeTVertices[0].x));
		newT2.y = safeTVertices[0].y + (t * (clippedTVertices[1].y - safeTVertices[0].y));

		Triangle *clippedTri = new Triangle(tri.verts[0], tri.verts[1], tri.verts[2], newT1, newT2, safeTVertices[0], tri.associatedMtl);
		clippedTri->transVerts[0] = one;
		clippedTri->transVerts[1] = two;
		clippedTri->transVerts[2] = safeVertices[0];
		outputTris.push_back(clippedTri);
		trisToDelete.push_back(clippedTri);
		return 1;
	}

	else if (clippedVertices.size() == 1)
	{
		float t;
		vec4 one = func::getIntersection(vec4(0, 0, c_near), vec4(0, 0, 1), safeVertices[0], clippedVertices[0], t);
		vec2 newT1;
		newT1.x = safeTVertices[0].x + (t * (clippedTVertices[0].x - safeTVertices[0].x));
		newT1.y = safeTVertices[0].y + (t * (clippedTVertices[0].y - safeTVertices[0].y));

		vec4 two = func::getIntersection(vec4(0, 0, c_near), vec4(0, 0, 1), safeVertices[1], clippedVertices[0], t);
		vec2 newT2;
		newT2.x = safeTVertices[1].x + (t * (clippedTVertices[0].x - safeTVertices[1].x));
		newT2.y = safeTVertices[1].y + (t * (clippedTVertices[0].y - safeTVertices[1].y));

		Triangle *clippedTri = new Triangle(tri.verts[0], tri.verts[1], tri.verts[2], newT1, newT2, safeTVertices[1], tri.associatedMtl);
		clippedTri->transVerts[0] = one;
		clippedTri->transVerts[1] = two;
		clippedTri->transVerts[2] = safeVertices[1];
		Triangle *clippedTri2 = new Triangle(tri.verts[0], tri.verts[1], tri.verts[2], newT1, safeTVertices[0], safeTVertices[1], tri.associatedMtl);
		clippedTri2->transVerts[0] = one;
		clippedTri2->transVerts[1] = safeVertices[0];
		clippedTri2->transVerts[2] = safeVertices[1];
		outputTris.push_back(clippedTri);
		outputTris.push_back(clippedTri2);
		trisToDelete.push_back(clippedTri);
		trisToDelete.push_back(clippedTri2);

		return 2;
	}
	return 0;


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
							int index = (zindex + i * w_width) * 4;

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
