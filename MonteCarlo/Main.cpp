#include <iostream>
#include <random>
#include <omp.h>    // openmp多线程加速
#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace glm;
using namespace std;
// --------------------- end of include --------------------- //

// 采样次数
const int SAMPLE = 4096;

// 每次采样的亮度
const double BRIGHTNESS = (2.0f * 3.1415926f) * (1.0f / double(SAMPLE));

// 输出图像分辨率
const int WIDTH = 256;
const int HEIGHT = 256;

// 相机参数
const double SCREEN_Z = 1.1;        // 视平面 z 坐标
const vec3 EYE = vec3(0, 0, 4.0);   // 相机位置

// 颜色
const vec3 RED(1, 0.5, 0.5);
const vec3 GREEN(0.5, 1, 0.5);
const vec3 BLUE(0.5, 0.5, 1);
const vec3 YELLOW(1.0, 1.0, 0.1);
const vec3 CYAN(0.1, 1.0, 1.0);
const vec3 MAGENTA(1.0, 0.1, 1.0);
const vec3 GRAY(0.5, 0.5, 0.5);
const vec3 WHITE(1, 1, 1);

// --------------- end of global variable definition --------------- //

// 光线
typedef struct Ray
{
	vec3 startPoint = vec3(0, 0, 0);    // 起点
	vec3 direction = vec3(0, 0, 0);     // 方向
}Ray;

// 物体表面材质定义
typedef struct Material
{
	bool isEmissive = false;        // 是否发光
	vec3 normal = vec3(0, 0, 0);    // 法向量
	vec3 color = vec3(0, 0, 0);     // 颜色
}Material;

// 光线求交结果
typedef struct HitResult
{
	bool isHit = false;             // 是否命中
	double distance = 0.0f;         // 与交点的距离
	vec3 hitPoint = vec3(0, 0, 0);  // 光线命中点
	Material material;              // 命中点的表面材质
}HitResult;

class Shape
{


public:
	Shape() {

	}
	virtual HitResult intersect(Ray ray) {

		return HitResult();
	}
};

// 三角形
class Triangle : public Shape
{
public:
	Triangle() {

	}
	Triangle(vec3 P1, vec3 P2, vec3 P3, vec3 C)
	{
		p1 = P1, p2 = P2, p3 = P3;
		material.normal = normalize(cross(p2 - p1, p3 - p1)); material.color = C;
	}
	vec3 p1, p2, p3;    // 三顶点
	Material material;  // 材质

	// 与光线求交
	HitResult intersect(Ray ray)
	{


		HitResult res;

		vec3 S = ray.startPoint;        // 射线起点
		vec3 d = ray.direction;         // 射线方向
		vec3 N = material.normal;       // 法向量
		if (dot(N, d) > 0.0f) N = -N;   // 获取正确的法向量

		// 如果视线和三角形平行
		if (fabs(dot(N, d)) < 0.00001f) return res;

		// 距离
		float t = (dot(N, p1) - dot(S, N)) / dot(d, N);
		if (t < 0.0005f) return res;    // 如果三角形在相机背面

		// 交点计算
		vec3 P = S + d * t;

		// 判断交点是否在三角形中
		vec3 c1 = cross(p2 - p1, P - p1);
		vec3 c2 = cross(p3 - p2, P - p2);
		vec3 c3 = cross(p1 - p3, P - p3);
		vec3 n = material.normal;   // 需要 "原生法向量" 来判断
		if (dot(c1, n) < 0 || dot(c2, n) < 0 || dot(c3, n) < 0) return res;

		// 装填返回结果
		res.isHit = true;
		res.distance = t;
		res.hitPoint = P;
		res.material = material;
		res.material.normal = N;    // 要返回正确的法向
		return res;
	};
};

// 输出 SRC 数组中的数据到图像
void writeImg(double* SRC, Mat& mat)
{



	unsigned char* image = new unsigned char[WIDTH * HEIGHT * 3];// 图像buffer
	unsigned char* p = image;
	double* S = SRC;    // 源数据

	for (int i = 0; i < HEIGHT; i++)
	{


		for (int j = 0; j < WIDTH; j++)
		{

			double* tempS = S + (i * WIDTH + j) * 3 + 2;
			*p++ = (unsigned char)clamp((*tempS--) * 255, 0.0, 255.0);  // opencv B 通道
			*p++ = (unsigned char)clamp((*tempS--) * 255, 0.0, 255.0);  // opencv G 通道
			*p++ = (unsigned char)clamp((*tempS--) * 255, 0.0, 255.0);  // opencv R 通道


		}
	}

	mat = Mat(WIDTH, HEIGHT, CV_8UC3, image).clone();
}

// 返回距离最近 hit 的结果
HitResult shoot(vector<Shape*>& shapes, Ray ray)
{


	HitResult res, r;
	res.distance = 1145141919.810f; // inf

	// 遍历所有图形，求最近交点
	for (auto& shape : shapes)
	{


		r = shape->intersect(ray);
		if (r.isHit && r.distance < res.distance) res = r;  // 记录距离最近的求交结果
	}

	return res;
}


// 0-1 随机数生成
std::uniform_real_distribution<> dis(0.0, 1.0);
random_device rd;
mt19937 gen(rd());
double randf()
{
	return dis(gen);
}

// 单位球内的随机向量
vec3 randomVec3()
{
	vec3 d;
	do
	{
		d = 2.0f * vec3(randf(), randf(), randf()) - vec3(1, 1, 1);
	} while (dot(d, d) > 1.0);
	return normalize(d);
}
// 法向半球随机向量
//vec3 randomDirection(vec3 n)
//{
//	// 法向半球
//	vec3 d;
//	do
//	{
//		d = randomVec3();
//	} while (dot(d, n) < 0.0f);
//	return d;
//}
// 法向半球随机向量
vec3 randomDirection(vec3 n)
{
	return normalize(randomVec3() + n);
}
// 路径追踪
vec3 pathTracing(vector<Shape*>& shapes, Ray ray)
{


	HitResult res = shoot(shapes, ray);

	if (!res.isHit) return vec3(0); // 未命中

	// 如果发光则返回颜色
	if (res.material.isEmissive) return res.material.color;

	// 否则直接返回
	return vec3(0);
}



int main() {
	Mat mat;

	double* image = new double[WIDTH * HEIGHT * 3];
	memset(image, 0.0, sizeof(double) * WIDTH * HEIGHT * 3);

	vector<Shape*> shapes;  // 几何物体的集合
	// 三角形
	shapes.push_back(new Triangle(vec3(-0.5, -0.5, -0.5), vec3(0.5, -0.5, -0.5), vec3(0, -0.5, 0.5), CYAN));
	// 底部平面
	shapes.push_back(new Triangle(vec3(10, -1, 10), vec3(-10, -1, -10), vec3(-10, -1, 10), WHITE));
	shapes.push_back(new Triangle(vec3(10, -1, 10), vec3(10, -1, -10), vec3(-10, -1, -10), WHITE));
	// 光源
	Triangle l1 = Triangle(vec3(0.6, 0.99, 0.4), vec3(-0.2, 0.99, -0.4), vec3(-0.2, 0.99, 0.4), WHITE);
	Triangle l2 = Triangle(vec3(0.6, 0.99, 0.4), vec3(0.6, 0.99, -0.4), vec3(-0.2, 0.99, -0.4), WHITE);
	l1.material.isEmissive = true;
	l2.material.isEmissive = true;
	shapes.push_back(&l1);
	shapes.push_back(&l2);

	int picIndex = 0;
	// render loop
	// -----------
	while (1)
	{
		omp_set_num_threads(50); // 线程个数
#pragma omp parallel for
		for (int k = 0; k < SAMPLE; k++)
		{
			double* p = image;
			for (int i = 0; i < HEIGHT; i++)
			{
				for (int j = 0; j < WIDTH; j++)
				{
					// 像素坐标转投影平面坐标
					double x = 2.0 * double(j) / double(WIDTH) - 1.0;
					double y = 2.0 * double(HEIGHT - i) / double(HEIGHT) - 1.0;

					vec3 coord = vec3(x, y, SCREEN_Z);          // 计算投影平面坐标
					vec3 direction = normalize(coord - EYE);    // 计算光线投射方向

					// 生成光线
					Ray ray;
					ray.startPoint = coord;
					ray.direction = direction;

					// 与场景的交点
					HitResult res = shoot(shapes, ray);
					vec3 color = vec3(0, 0, 0);

					if (res.isHit)
					{


						// 命中光源直接返回光源颜色
						if (res.material.isEmissive)
						{
							color = res.material.color;
						}
						// 命中实体则选择一个随机方向重新发射光线并且进行路径追踪
						else
						{
							// 根据交点处法向量生成交点处反射的随机半球向量
							Ray randomRay;
							randomRay.startPoint = res.hitPoint;
							randomRay.direction = randomDirection(res.material.normal);

							// 颜色积累
							vec3 srcColor = res.material.color;
							vec3 ptColor = pathTracing(shapes, randomRay);
							color = ptColor * srcColor;    // 和原颜色混合
							color *= BRIGHTNESS;
						}
					}

					*p += color.x; p++;  // R 通道
					*p += color.y; p++;  // G 通道
					*p += color.z; p++;  // B 通道
				}
			}
		}
		writeImg(image, mat);
		//flip(mat, mat, 0);
		imshow("CV", mat);
		picIndex++;
		cout << picIndex << endl;

		waitKey(0);

		// 等待按键，退出循环
		if (waitKey(1) >= 0)
			break;

	}
	return 0;
}