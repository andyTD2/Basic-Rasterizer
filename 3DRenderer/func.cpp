#include "func.hpp"
#include <iomanip>

vec4 func::getIntersection(vec4 plane_point, vec4 plane_normal, vec4 p0, vec4 p1, float& t)
{
	plane_normal = func::norm(plane_normal);
	float dist = -func::dotPro(plane_normal, plane_point);
	float ad = func::dotPro(p0, plane_normal);
	float bd = func::dotPro(p1, plane_normal);
	t = (-dist - ad) / (bd - ad);
	vec4 line = p1 - p0;
	vec4 lineIntersect;
	lineIntersect.x = line.x * t; lineIntersect.y = line.y * t; lineIntersect.z = line.z * t;
	return (p0 + lineIntersect);

}

float func::vecXmatrix(const vec4& vec, const float matrix[4][4], vec4& result, bool project)
{
	__m256 multiplyXandYs = _mm256_mul_ps(_mm256_setr_ps(vec.x, vec.x, vec.x, vec.x, vec.y, vec.y, vec.y, vec.y),
		_mm256_setr_ps(matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3], matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3]));

	__m128 multiplyZs = _mm_mul_ps(_mm_set1_ps(vec.z), _mm_setr_ps(matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3]));

	__m256 snd = _mm256_castps128_ps256(multiplyZs);
	snd = _mm256_insertf128_ps(snd, _mm_setr_ps(matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]), 1);
	__m256 add = _mm256_add_ps(multiplyXandYs, snd);

	__m128 sum = _mm_add_ps(_mm256_extractf128_ps(add, 0), _mm256_extractf128_ps(add, 1));
	float* sumArray = (float*)&sum;

	result.x = sumArray[0];
	result.y = sumArray[1];
	result.z = sumArray[2];
	result.w = sumArray[3];

	if (result.w != 1 && project)
	{
		result.x /= result.w;
		result.y /= result.w;
		result.z /= result.w;
	}
	return result.w;
}

float func::edge_f(const vec2& pixel, const vec4& v0, const vec4& v1)
{
	return (pixel.x - v0.x) * (v1.y - v0.y) - (pixel.y - v0.y) * (v1.x - v0.x);
}

void func::matrixXmatrix(const float leftMat[4][4], const float rightMat[4][4], float(&res)[4][4])
{
	for (uint8_t i = 0; i < 4; ++i) {
		for (uint8_t j = 0; j < 4; ++j) {
			res[i][j] = leftMat[i][0] * rightMat[0][j] +
				leftMat[i][1] * rightMat[1][j] +
				leftMat[i][2] * rightMat[2][j] +
				leftMat[i][3] * rightMat[3][j];
		}
	}
	return;
}

vec4 func::norm(const vec4& in)
{
	float mag = sqrt(dotPro(in, in));
	return vec4(in.x / mag, in.y / mag, in.z / mag);
}

vec4 func::crossProd(const vec4& l, const vec4& r)
{
	return vec4((l.y * r.z) - (l.z * r.y),
		(l.z * r.x) - (l.x * r.z),
		(l.x * r.y) - (l.y * r.x));
}

float func::dotPro(const vec4& l, const vec4& r)
{
	return ((l.x * r.x) + (l.y * r.y) + (l.z * r.z));
}

float func::dotpro2f(const vec2& l, const vec2& r)
{
	return (l.x * r.x) + (l.y * r.y);
}

sf::Vector3f func::vec3XScalar(const sf::Vector3f l, float r)
{
	return sf::Vector3f(l.x * r, l.y * r, l.z * r);
}

sf::Vector2f func::vec2XScalar(const sf::Vector2f l, float r)
{
	return sf::Vector2f(l.x * r, l.y * r);
}

vec4::vec4()
{
	x = 0;
	y = 0;
	z = 0;
	w = 1;
}
vec4::vec4(float newX, float newY, float newZ)
{
	x = newX;
	y = newY;
	z = newZ;
	w = 1;
}
vec4::vec4(const vec4& other)
{
	this->x = other.x;
	this->y = other.y;
	this->z = other.z;
	this->w = other.w;
}
vec4 vec4::operator+(const vec4& other) const
{
	return vec4(x + other.x, y + other.y, z + other.z);
}
vec4& vec4::operator+=(const vec4& other)
{
	this->x += other.x;
	this->y += other.y;
	this->z += other.z;

	return *this;
}
vec4 vec4::operator-(const vec4& other) const
{
	return vec4(x - other.x, y - other.y, z - other.z);
}
vec4& vec4::operator-=(const vec4& other)
{
	this->x -= other.x;
	this->y -= other.y;
	this->z -= other.z;

	return *this;
}
vec4 vec4::operator*(float r) const
{
	return vec4(x * r, y * r, z * r);
}

vec2::vec2()
{
	x = 0;
	y = 0;
}
vec2::vec2(float newX, float newY)
{
	x = newX;
	y = newY;
}

float func::getDist(const vec4& planeNormal, const vec4& planePoint, const vec4& point)
{
	return func::dotPro(planeNormal, (point - planePoint));
}