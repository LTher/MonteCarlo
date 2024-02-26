#include <iostream>
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

// ��� SRC �����е����ݵ�ͼ��
void writeImg(double* SRC, Mat& mat)
{



	unsigned char* image = new unsigned char[WIDTH * HEIGHT * 3];// ͼ��buffer
	unsigned char* p = image;
	double* S = SRC;    // Դ����

	FILE* fp;
	fopen_s(&fp, "diablo3_pose_diffuse.png", "wb");

	for (int i = 0; i < HEIGHT; i++)
	{


		for (int j = 0; j < WIDTH; j++)
		{

			double* tempS = S + (i * WIDTH + j) * 3+2;
			*p++ = (unsigned char)clamp((*tempS--) * 255, 0.0, 255.0);  // R ͨ��
			*p++ = (unsigned char)clamp((*tempS--) * 255, 0.0, 255.0);  // G ͨ��
			*p++ = (unsigned char)clamp((*tempS--) * 255, 0.0, 255.0);  // B ͨ��


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


			// ��������תͶӰƽ������
			double x = 2.0 * double(j) / double(WIDTH) - 1.0;
			double y = 2.0 * double(HEIGHT - i) / double(HEIGHT) - 1.0;

			vec3 coord = vec3(x, y, SCREEN_Z);          // ����ͶӰƽ������
			vec3 direction = normalize(coord - EYE);    // �������Ͷ�䷽��

			vec3 color = direction;

			*p = color.x; p++;  // R ͨ��
			*p = color.y; p++;  // G ͨ��
			*p = color.z; p++;  // B ͨ��
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

		// �ȴ��������˳�ѭ��
		if (waitKey(1) >= 0)
			break;
	}
	return 0;
}