///运动物体检测――帧差法
#include"opencv2/opencv.hpp"

using namespace cv;
using namespace std;

//运动物体检测函数声明
Mat MoveDetect(const Mat &temp, const Mat &frame);

int main() {
    //读取帧、平滑、帧差或背景差、二值化、膨胀、腐蚀。

    VideoCapture video("in.mp4");
    if (!video.isOpened())
        std::cout << "!!! Failed to open file: " <<endl;
    while (true) {
        auto frameCount = video.get(CV_CAP_PROP_FRAME_COUNT);//获取帧数
        auto FPS = video.get(CV_CAP_PROP_FPS);//获取FPS
        Mat frame;//存储帧
        Mat temp;//存储前一帧图像
        Mat result;//存储结果图像
        for (int i = 0; i < frameCount; i++) {
            //读取帧
            video >> frame;//读帧进frame
            if (frame.empty())//对帧进行异常检测
            {
                cout << "frame is empty!" << endl;
                break;
            }
            imshow("frame", frame);

            //处理帧
            if (i == 0)//如果为第一帧（temp还为空）
            {
                result = MoveDetect(frame, frame);//调用MoveDetect()进行运动物体检测，返回值存入result
            } else//若不是第一帧（temp有值了）
            {
                result = MoveDetect(temp, frame);//调用MoveDetect()进行运动物体检测，返回值存入result

            }

            imshow("result", result);
            if (waitKey(static_cast<int>(1000.0 / FPS)) == 27)//按原FPS显示
            {
                cout << "ESC退出!" << endl;
                break;
            }
            temp = frame.clone();
        }
        if (waitKey(30) >= 0) break;
    }
    return 0;

}

Mat MoveDetect(const Mat &temp, const Mat &frame) {

    //平滑、帧差或背景差、二值化、膨胀、腐蚀。
    Mat result = frame.clone();
    //1.平滑处理
    IplImage *tempimg_src, *tempimg_dst;
    tempimg_src = new IplImage(temp);
    tempimg_dst = cvCreateImage(cvGetSize(tempimg_src), 8, 3);
    cvSmooth(tempimg_src, tempimg_dst, CV_BLUR, 3, 3, 0, 0);//平滑函数

    IplImage *frameimg_src, *frameimg_dst;
    frameimg_src = new IplImage(frame);
    frameimg_dst = cvCreateImage(cvGetSize(frameimg_src), 8, 3);
    cvSmooth(frameimg_src, frameimg_dst, CV_BLUR, 3, 3, 0, 0);//平滑函数

    //2.帧差
    Mat temp1, frame1;
    temp1 = cvarrToMat(tempimg_dst);
    frame1 = cvarrToMat(frameimg_dst);
    //2.1将background和frame转为灰度图
    Mat gray1, gray2;
    cvtColor(temp1, gray1, CV_BGR2GRAY);
    cvtColor(frame1, gray2, CV_BGR2GRAY);
    //2.2.将background和frame做差
    Mat diff;
    absdiff(gray1, gray2, diff);
    imshow("absdiff", diff);


    //3.对差值图diff_thresh进行阈值化处理  二值化
    Mat diff_thresh;

    /*
    getStructuringElement()
    这个函数的第一个参数表示内核的形状，有三种形状可以选择。
    矩形：MORPH_RECT;
    交叉形：MORPH_CORSS;
    椭圆形：MORPH_ELLIPSE;
    第二和第三个参数分别是内核的尺寸以及锚点的位置
    */
    Mat kernel_erode = getStructuringElement(MORPH_RECT, Size(3, 3));//函数会返回指定形状和尺寸的结构元素。
    //调用之后，调用膨胀与腐蚀函数的时候，第三个参数值保存了getStructuringElement返回值的Mat类型变量。也就是element变量。
    Mat kernel_dilate = getStructuringElement(MORPH_RECT, Size(18, 18));

    //进行二值化处理，选择50，255为阈值
    threshold(diff, diff_thresh, 50, 255, CV_THRESH_BINARY);
    imshow("threshold", diff_thresh);
    //4.膨胀
    dilate(diff_thresh, diff_thresh, kernel_dilate);
    imshow("dilate", diff_thresh);
    //5.腐蚀
    erode(diff_thresh, diff_thresh, kernel_erode);
    imshow("erode", diff_thresh);

    //6.查找轮廓并绘制轮廓
    vector<vector<Point> > contours;
    findContours(diff_thresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);//找轮廓函数
    drawContours(result, contours, -1, Scalar(0, 0, 255), 2);//在result上绘制轮廓
    //7.查找正外接矩形
    vector<Rect> boundRect(contours.size());
    for (int i = 0; i < contours.size(); i++) {
        boundRect[i] = boundingRect(contours[i]);
        rectangle(result, boundRect[i], Scalar(0, 255, 0), 2);//在result上绘制正外接矩形
    }
    return result;//返回result
}