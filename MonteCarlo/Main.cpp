#include <iostream>
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

// 输出 SRC 数组中的数据到图像
void writeImg(double* SRC, Mat& mat)
{



	unsigned char* image = new unsigned char[WIDTH * HEIGHT * 3];// 图像buffer
	unsigned char* p = image;
	double* S = SRC;    // 源数据

	FILE* fp;
	fopen_s(&fp, "diablo3_pose_diffuse.png", "wb");

	for (int i = 0; i < HEIGHT; i++)
	{


		for (int j = 0; j < WIDTH; j++)
		{

			double* tempS = S + (i * WIDTH + j) * 3+2;
			*p++ = (unsigned char)clamp((*tempS--) * 255, 0.0, 255.0);  // R 通道
			*p++ = (unsigned char)clamp((*tempS--) * 255, 0.0, 255.0);  // G 通道
			*p++ = (unsigned char)clamp((*tempS--) * 255, 0.0, 255.0);  // B 通道


		}
	}

	mat = Mat(WIDTH, HEIGHT, CV_8UC3, image).clone();
}

int main() {
	Mat mat;

	//mat = imread("diablo3_pose_diffuse.png");
	double* image = new double[WIDTH * HEIGHT * 3];
	memset(image, 0.0, sizeof(double) * WIDTH * HEIGHT * 3);
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

			vec3 color = direction;

			*p = color.x; p++;  // R 通道
			*p = color.y; p++;  // G 通道
			*p = color.z; p++;  // B 通道
		}
	}

	writeImg(image, mat);

	int picIndex = 0;
	// render loop
	// -----------
	while (1)
	{
		//flip(mat, mat, 0);
		imshow("CV", mat);
		picIndex++;
		cout << picIndex << endl;

		// 等待按键，退出循环
		if (waitKey(1) >= 0)
			break;
	}
	return 0;
}