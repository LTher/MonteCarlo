#include <iostream>
#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace glm;
using namespace std;

int main() {
	Mat mat;

	mat = imread("diablo3_pose_diffuse.png");

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