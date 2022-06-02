#include "func.hpp"
#include <iomanip>

sf::Vector3f func::getIntersection(sf::Vector3f plane_point, sf::Vector3f plane_normal, sf::Vector3f p0, sf::Vector3f p1, float& t)
{
	plane_normal = func::norm3f(plane_normal);
	float dist = -func::dotpro(plane_normal, plane_point);
	float ad = func::dotpro(p0, plane_normal);
	float bd = func::dotpro(p1, plane_normal);
	t = (-dist - ad) / (bd - ad);
	sf::Vector3f line = p1 - p0;
	sf::Vector3f lineIntersect;
	lineIntersect.x = line.x * t; lineIntersect.y = line.y * t; lineIntersect.z = line.z * t;
	return (p0 + lineIntersect);

}

void func::vecXmatrix(const sf::Vector3f vec, const float matrix[4][4], sf::Vector3f& result)
{
	/*result.x = vec.x * matrix[0][0] + vec.y * matrix[1][0] + vec.z * matrix[2][0] + matrix[3][0];
	result.y = vec.x * matrix[0][1] + vec.y * matrix[1][1] + vec.z * matrix[2][1] + matrix[3][1];
	result.z = vec.x * matrix[0][2] + vec.y * matrix[1][2] + vec.z * matrix[2][2] + matrix[3][2];
	float w = vec.x * matrix[0][3] + vec.y * matrix[1][3] + vec.z * matrix[2][3] + matrix[3][3];

	if (w != 1)
	{
		result.x /= w;
		result.y /= w;
		result.z /= w;
	}
	*/
	__m256 multiplyXandYs = _mm256_mul_ps(_mm256_setr_ps(vec.x, vec.x, vec.x, vec.x, vec.y, vec.y, vec.y, vec.y), 
							   _mm256_setr_ps(matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3], matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3]));

	__m128 multiplyZs = _mm_mul_ps(_mm_setr_ps(vec.z, vec.z, vec.z, vec.z), _mm_setr_ps(matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3]));

	__m256 snd = _mm256_castps128_ps256(multiplyZs);
	snd = _mm256_insertf128_ps(snd, _mm_setr_ps(matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]), 1);
	__m256 add = _mm256_add_ps(multiplyXandYs, snd);

	__m128 sum = _mm_add_ps(_mm256_extractf128_ps(add, 0), _mm256_extractf128_ps(add, 1));
	float* sumArray = (float*)&sum;
	result.x = sumArray[0];
	result.y = sumArray[1];
	result.z = sumArray[2];
	float w = sumArray[3];
	if (w != 1)
	{
		result.x /= w;
		result.y /= w;
		result.z /= w;
	}
	
	

	

}

float func::edge_f(const sf::Vector2f& pixel, const sf::Vector3f& v0, const sf::Vector3f& v1)
{

	//std::cout << "pixel.x: " << pixel.x << ", " << pixel.y << std::endl;
	//func::print(v0);
	//func::print(v1);

	//int temp = ((v1.y - v0.y) * pixel.x) + ((v0.x - v1.x) * pixel.y) + ((v0.y * v1.x) - (v0.x * v1.y));
	//int temp = ((v0.y - v1.y) * pixel.x) + ((v1.x - v0.x) * pixel.y) + ((v0.x * v1.y) - (v0.y * v1.x));
	//std::cout << "NEW: " << temp << std::endl;

	//OLD
	//int temp = ((pixel.x - v0.x) * (v1.y - v0.y)) - ((pixel.y - v0.y) * (v1.x - v0.x));
	double temp = (pixel.x - v0.x) * (v1.y - v0.y) - (pixel.y - v0.y) * (v1.x - v0.x);
	return temp;
}

void func::matrixXmatrix (const float left_mat[4][4], const float right_mat[4][4], float (&res)[4][4])
{
	for (uint8_t i = 0; i < 4; ++i) {
		for (uint8_t j = 0; j < 4; ++j) {
			res[i][j] = left_mat[i][0] * right_mat[0][j] +
				left_mat[i][1] * right_mat[1][j] +
				left_mat[i][2] * right_mat[2][j] +
				left_mat[i][3] * right_mat[3][j];
		}
	}
	return;
}

sf::Vector3f func::norm3f(const sf::Vector3f& in)
{
	float mag = sqrt(dotpro(in, in));
	return sf::Vector3f(in.x / mag, in.y / mag, in.z / mag);
}

sf::Vector3f func::crossv(const sf::Vector3f l, const sf::Vector3f r)
{
	return sf::Vector3f((l.y * r.z) - (l.z * r.y),
						(l.z * r.x) - (l.x * r.z),
						(l.x * r.y) - (l.y * r.x));
}

float func::dotpro(const sf::Vector3f l, const sf::Vector3f r)
{
	return (l.x * r.x) + (l.y * r.y) + (l.z * r.z);
}

sf::Vector3f func::vec3XScalar(const sf::Vector3f l, float r)
{
	return sf::Vector3f(l.x * r, l.y * r, l.z * r);
}

sf::Vector2f func::vec2XScalar(const sf::Vector2f l, float r)
{
	return sf::Vector2f(l.x * r, l.y * r);
}

void func::print(const sf::Vector3f& i)
{
	
	std::cout << std::setprecision(50) << "x: " << i.x << " y: " << i.y << " z: " << i.z << std::endl;
}