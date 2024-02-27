#include <iostream>
#include <random>
#include <omp.h>    // openmp���̼߳���
#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace glm;
using namespace std;
// --------------------- end of include --------------------- //

// ��������
const int SAMPLE = 4096;

// ÿ�β���������
const double BRIGHTNESS = (2.0f * 3.1415926f) * (1.0f / double(SAMPLE));

// ���ͼ��ֱ���
const int WIDTH = 256;
const int HEIGHT = 256;

// �������
const double SCREEN_Z = 1.1;        // ��ƽ�� z ����
const vec3 EYE = vec3(0, 0, 4.0);   // ���λ��

// ��ɫ
const vec3 RED(1, 0.5, 0.5);
const vec3 GREEN(0.5, 1, 0.5);
const vec3 BLUE(0.5, 0.5, 1);
const vec3 YELLOW(1.0, 1.0, 0.1);
const vec3 CYAN(0.1, 1.0, 1.0);
const vec3 MAGENTA(1.0, 0.1, 1.0);
const vec3 GRAY(0.5, 0.5, 0.5);
const vec3 WHITE(1, 1, 1);

// --------------- end of global variable definition --------------- //

// ����
typedef struct Ray
{
	vec3 startPoint = vec3(0, 0, 0);    // ���
	vec3 direction = vec3(0, 0, 0);     // ����
}Ray;

// ���������ʶ���
typedef struct Material
{
	bool isEmissive = false;        // �Ƿ񷢹�
	vec3 normal = vec3(0, 0, 0);    // ������
	vec3 color = vec3(0, 0, 0);     // ��ɫ
}Material;

// �����󽻽��
typedef struct HitResult
{
	bool isHit = false;             // �Ƿ�����
	double distance = 0.0f;         // �뽻��ľ���
	vec3 hitPoint = vec3(0, 0, 0);  // �������е�
	Material material;              // ���е�ı������
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

// ������
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
	vec3 p1, p2, p3;    // ������
	Material material;  // ����

	// �������
	HitResult intersect(Ray ray)
	{


		HitResult res;

		vec3 S = ray.startPoint;        // �������
		vec3 d = ray.direction;         // ���߷���
		vec3 N = material.normal;       // ������
		if (dot(N, d) > 0.0f) N = -N;   // ��ȡ��ȷ�ķ�����

		// ������ߺ�������ƽ��
		if (fabs(dot(N, d)) < 0.00001f) return res;

		// ����
		float t = (dot(N, p1) - dot(S, N)) / dot(d, N);
		if (t < 0.0005f) return res;    // ������������������

		// �������
		vec3 P = S + d * t;

		// �жϽ����Ƿ�����������
		vec3 c1 = cross(p2 - p1, P - p1);
		vec3 c2 = cross(p3 - p2, P - p2);
		vec3 c3 = cross(p1 - p3, P - p3);
		vec3 n = material.normal;   // ��Ҫ "ԭ��������" ���ж�
		if (dot(c1, n) < 0 || dot(c2, n) < 0 || dot(c3, n) < 0) return res;

		// װ��ؽ��
		res.isHit = true;
		res.distance = t;
		res.hitPoint = P;
		res.material = material;
		res.material.normal = N;    // Ҫ������ȷ�ķ���
		return res;
	};
};

// ��� SRC �����е����ݵ�ͼ��
void writeImg(double* SRC, Mat& mat)
{



	unsigned char* image = new unsigned char[WIDTH * HEIGHT * 3];// ͼ��buffer
	unsigned char* p = image;
	double* S = SRC;    // Դ����

	for (int i = 0; i < HEIGHT; i++)
	{


		for (int j = 0; j < WIDTH; j++)
		{

			double* tempS = S + (i * WIDTH + j) * 3 + 2;
			*p++ = (unsigned char)clamp((*tempS--) * 255, 0.0, 255.0);  // opencv B ͨ��
			*p++ = (unsigned char)clamp((*tempS--) * 255, 0.0, 255.0);  // opencv G ͨ��
			*p++ = (unsigned char)clamp((*tempS--) * 255, 0.0, 255.0);  // opencv R ͨ��


		}
	}

	mat = Mat(WIDTH, HEIGHT, CV_8UC3, image).clone();
}

// ���ؾ������ hit �Ľ��
HitResult shoot(vector<Shape*>& shapes, Ray ray)
{


	HitResult res, r;
	res.distance = 1145141919.810f; // inf

	// ��������ͼ�Σ����������
	for (auto& shape : shapes)
	{


		r = shape->intersect(ray);
		if (r.isHit && r.distance < res.distance) res = r;  // ��¼����������󽻽��
	}

	return res;
}


// 0-1 ���������
std::uniform_real_distribution<> dis(0.0, 1.0);
random_device rd;
mt19937 gen(rd());
double randf()
{
	return dis(gen);
}

// ��λ���ڵ��������
vec3 randomVec3()
{
	vec3 d;
	do
	{
		d = 2.0f * vec3(randf(), randf(), randf()) - vec3(1, 1, 1);
	} while (dot(d, d) > 1.0);
	return normalize(d);
}
// ��������������
//vec3 randomDirection(vec3 n)
//{
//	// �������
//	vec3 d;
//	do
//	{
//		d = randomVec3();
//	} while (dot(d, n) < 0.0f);
//	return d;
//}
// ��������������
vec3 randomDirection(vec3 n)
{
	return normalize(randomVec3() + n);
}
// ·��׷��
vec3 pathTracing(vector<Shape*>& shapes, Ray ray)
{


	HitResult res = shoot(shapes, ray);

	if (!res.isHit) return vec3(0); // δ����

	// ��������򷵻���ɫ
	if (res.material.isEmissive) return res.material.color;

	// ����ֱ�ӷ���
	return vec3(0);
}



int main() {
	Mat mat;

	double* image = new double[WIDTH * HEIGHT * 3];
	memset(image, 0.0, sizeof(double) * WIDTH * HEIGHT * 3);

	vector<Shape*> shapes;  // ��������ļ���
	// ������
	shapes.push_back(new Triangle(vec3(-0.5, -0.5, -0.5), vec3(0.5, -0.5, -0.5), vec3(0, -0.5, 0.5), CYAN));
	// �ײ�ƽ��
	shapes.push_back(new Triangle(vec3(10, -1, 10), vec3(-10, -1, -10), vec3(-10, -1, 10), WHITE));
	shapes.push_back(new Triangle(vec3(10, -1, 10), vec3(10, -1, -10), vec3(-10, -1, -10), WHITE));
	// ��Դ
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
		omp_set_num_threads(50); // �̸߳���
#pragma omp parallel for
		for (int k = 0; k < SAMPLE; k++)
		{
			double* p = image;
			for (int i = 0; i < HEIGHT; i++)
			{
				for (int j = 0; j < WIDTH; j++)
				{
					// ��������תͶӰƽ������
					double x = 2.0 * double(j) / double(WIDTH) - 1.0;
					double y = 2.0 * double(HEIGHT - i) / double(HEIGHT) - 1.0;

					vec3 coord = vec3(x, y, SCREEN_Z);          // ����ͶӰƽ������
					vec3 direction = normalize(coord - EYE);    // �������Ͷ�䷽��

					// ���ɹ���
					Ray ray;
					ray.startPoint = coord;
					ray.direction = direction;

					// �볡���Ľ���
					HitResult res = shoot(shapes, ray);
					vec3 color = vec3(0, 0, 0);

					if (res.isHit)
					{


						// ���й�Դֱ�ӷ��ع�Դ��ɫ
						if (res.material.isEmissive)
						{
							color = res.material.color;
						}
						// ����ʵ����ѡ��һ������������·�����߲��ҽ���·��׷��
						else
						{
							// ���ݽ��㴦���������ɽ��㴦����������������
							Ray randomRay;
							randomRay.startPoint = res.hitPoint;
							randomRay.direction = randomDirection(res.material.normal);

							// ��ɫ����
							vec3 srcColor = res.material.color;
							vec3 ptColor = pathTracing(shapes, randomRay);
							color = ptColor * srcColor;    // ��ԭ��ɫ���
							color *= BRIGHTNESS;
						}
					}

					*p += color.x; p++;  // R ͨ��
					*p += color.y; p++;  // G ͨ��
					*p += color.z; p++;  // B ͨ��
				}
			}
		}
		writeImg(image, mat);
		//flip(mat, mat, 0);
		imshow("CV", mat);
		picIndex++;
		cout << picIndex << endl;

		waitKey(0);

		// �ȴ��������˳�ѭ��
		if (waitKey(1) >= 0)
			break;

	}
	return 0;
}