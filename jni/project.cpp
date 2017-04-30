#include <jni.h>
#include <iostream>
#include <cstdio>
#include <ctype.h>
#include <time.h>
#include <set>
#include <climits>
#include <algorithm>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#define PI 3.14159265

using namespace cv;
using namespace std;
extern "C" {

	class motion_vector {
	public:
		Point2f point;
    float angle;    //angle in degrees
    motion_vector(Point2f point, float angle) {
    	this->point = point;
    	this->angle = acos(angle) * 180 / PI;
    };

};

float getAngle(Point2f p1, Point2f p2) {
	float cos_angle = (p2.x - p1.x) / (float) norm(p2 - p1);
	float angle = acos(cos_angle) * 180 / PI;
    return angle;
}
int max_x;
int colors[10][3];
int best_k;
vector <Point2f> bottom_point;
bool REMOVE_UNTRACKED_POINTS = true;
// int ANGLE_THRESHOLD_FOR_APPROACHING_OBJECTS = 50;
int THRESHOLD_FOR_CLASSIFYING_APPROACHING_OBJECTS = 60; // in %
int PIXEL_WINDOW_FOR_MOTION_CLASS_ESTIMATION = 30;
float ANGULAR_THRESHOLD_FOR_MOTION_CLASS_ESTIMATION = 20.0f;
float THRESHOLD_FOR_BACKGROUND_FOREGROUND_DISTINCTION = 0.75f;
int MINIMUM_NUMBER_OF_POINTS_IN_CLUSTER = 6;
vector <Point2f> temp_points[2];
vector <Point2f> points[2];
vector<bool> foreground;
vector <Point2f> new_points;
vector <motion_vector> foreground_motion_vectors;
vector <Point2f> foreground_motion_vectors_points;
vector <Point2f> foreground_motion_vectors_points_old;
vector<float> foreground_motion_vectors_angles;
vector<float> cluster_dist_from_bottom_point;
vector<int> cluster_points_indices;

bool compare_motion_vectors(const motion_vector m1, const motion_vector m2) {
	return m1.angle < m2.angle;
}

TermCriteria termcrit(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);
Size subPixWinSize(10, 10), winSize(31, 31);
const int MAX_COUNT = 500, MAX_COUNT_INCREMENT = 100;
bool needToInit = true;
bool needToFindK = true;
bool nightMode = false;
// check for argument or camera capturing

Mat gray, prevGray, frame;
int step = 20; // 10 pixels spacing between kp's
time_t t1 = time(0), t2;
// Mat frame;
Mat_<float> H;
Size reduced_size(640, 480);
int offsetx;
int offsety;
int SAMPLE_EVERY_N_FRAMES = 3;
int counter = 0;

JNIEXPORT jstring JNICALL Java_com_example_adesh_myapplication_MainActivity_Wrapper(JNIEnv * env, jclass , jlong addrmat )
{

    srand(time(NULL));
	counter ++ ;
// if(counter%SAMPLE_EVERY_N_FRAMES != 0)
//  continue;
	Mat &frame = *(Mat *) addrmat;
//	Mat frame;
//	cv::resize(frame, frame, reduced_size ) ;
	if ( frame . empty())
		return (*env).NewStringUTF("ERROR: NO FRAME RECEIVED!");
	// frame . copyTo(frame);
	cvtColor(frame, gray, COLOR_BGR2GRAY ) ;
	if ( nightMode )
		frame = Scalar::all(0);
	if ( needToInit )
	{
		points [ 1 ] . clear();
		for ( int y = step;
			y<frame . rows - step;
			y += step ) {
			for ( int x = step;
				x<frame. cols - step;
				x += step ) {
				offsetx = rand() % 10;
			offsety = rand() % 10;
			points [ 1 ] . push_back(Point2f((float) x + offsetx, (float) y + offsety));
		}
	}
	needToInit = false;
}

else if( !points[0].

	empty()

	)
{
	vector <uchar> status;
	vector<float> err;
	if(prevGray.

		empty()

		)
		gray.
	copyTo(prevGray);
	calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize,
		3, termcrit, 0, 0.001);
	foreground.

	clear();

	if(REMOVE_UNTRACKED_POINTS){
		temp_points[0].

		clear();

		temp_points[1].

		clear();

		for (
			int i = 0;
			i<points[0].

			size();

			++i)
		{
			if(status[i] != 0){
				temp_points[0].
				push_back(points[0][i]);
				temp_points[1].
				push_back(points[1][i]);
			}
		}
		points[0].

		clear();

		points[1].

		clear();

		points[0].
		insert(points[0]
			.

			end(), temp_points[0]

			.

			begin(), temp_points[0]

			.

			end()

			);
		points[1].
		insert(points[1]
			.

			end(), temp_points[1]

			.

			begin(), temp_points[1]

			.

			end()

			);
	}








// FINDING  HOMOGRAPHY  MATRIX //////////////////////////////////////////////////////////////////////////////////////////////////

	try{
		H = findHomography(points[0], points[1], CV_RANSAC);
	}
	catch(
		Exception e
		){
// cout<<"Error! Insufficient number of points for findHomography. Skipping frame.\n";
		return (*env).NewStringUTF("HOMOGRAPHY FAILED!");
	}

//// HOMOGRAPHY DONE//////////////////////////////////////////////////////////////////////////////////////////////














//// FIND FOREGROUND POINTS /////////////////////////////////////////////////////////////////

	for (
		int i = 0;
		i<points[0].

		size();

		++i)
	{
		if(status[i] == 0){
// foreground.push_back(false);
			continue;
		}
		Vec3f old_point = Vec3f((float) points[0][i].x, (float) points[0][i].y, 1.0);
		Mat new_point_matrix = H * Mat(old_point);
		Point2f new_point = Point2f(new_point_matrix.at<float>(0, 0) / new_point_matrix.at<float>(2, 0),
			new_point_matrix.at<float>(1, 0) / new_point_matrix.at<float>(2, 0));
// if(norm(points[1][i] - new_point)>2)
//     foreground.push_back(true);
// else
//     foreground.push_back(false);
		if(
			norm(points[1][i]
				- new_point)>THRESHOLD_FOR_BACKGROUND_FOREGROUND_DISTINCTION){
//foreground point
			float cos_angle = (points[1][i].x - points[0][i].x) / (float) norm(points[1][i] - points[0][i]);
		motion_vector mVector(points[1][i], cos_angle);
		foreground_motion_vectors_points.
push_back(points[1][i]);//////////////////////////////////
foreground_motion_vectors_points_old.
push_back(points[0][i]);//////////////////////////////////
float angle = acos(cos_angle) * 180 / PI;
foreground_motion_vectors_angles.
push_back(angle);

foreground_motion_vectors.
push_back(mVector);
}
}

///// FOREGROUND MOTION DONE ////////////////////////////////////////////////////////////////////////////////////////////

















//// KMEANS  CLUSTERINGS STARTS //////////////////////////////////////////////////////////////////////////////////////////

Mat pts = Mat(foreground_motion_vectors_points);
Mat angles = Mat(foreground_motion_vectors_angles);
Mat sample = Mat(foreground_motion_vectors_points);
Mat labels;
int attempts = 5;
Mat centers;
int clusterCount;

if(foreground_motion_vectors_points.

	size()

	>MINIMUM_NUMBER_OF_POINTS_IN_CLUSTER ){

// if(needToFindK){
//     float arr[10];
//     for(int i=1;i<=4;i++){
//         float comp=kmeans(sample,i, labels, TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,
//                 10000, 0.0001), attempts, KMEANS_PP_CENTERS, centers );
//         arr[i]=comp;
//     }
//     float max_diff=0;float diff=0;;
//     for(int i=1;i<3;i++){
//         diff=arr[i+1]-arr[i];
//         if(i==1){
//             max_diff=diff;best_k=2;
//         }
//         else if(diff>max_diff){
//             max_diff=diff;best_k=i+1;
//         }
//     }
//     // cout<<"found best k = "<<best_k<<endl;
//     needToFindK = false;
// }
// clusterCount = best_k;
	clusterCount = 3;
float compactness = kmeans(sample, clusterCount, labels,
	TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS,
		10000, 0.0001), attempts, KMEANS_PP_CENTERS, centers);

////// CLUSTERING  DONE /////////////////////////////////////////////////////////////////////////////////////////////













///// APPROACHING AND URGENT //////////////////////////////////////////////////////////////////////////////////////

// printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
bottom_point.
push_back(Point2f(frame.rows, frame.cols / 2));
for(
	int i = 0;
	i<clusterCount;
	i++)
{
	cluster_points_indices.

	clear();

	for(
		int p = 0;
		p<foreground_motion_vectors_points.

		size();

p++){//loop over all points (to identify points in cluster i)
		if(labels.
			at<int>(p,
				0) == i){
			cluster_points_indices.
push_back(p);               // adding the index of the point belongong to cluster i
}
}
// cout<<"****************"<<cluster_points_indices.size()<<"****************************************************"<<endl;

int approaching = 0;
int departing = 0;                    // for storing the voting of approaching/departing based on angle
for(
	int v = 0;
	v<cluster_points_indices.

	size();

	v++){
// loop over all cluster points for voting
	int point_index = cluster_points_indices.at(v);
if((foreground_motion_vectors_points[point_index].y - foreground_motion_vectors_points_old[point_index].y) >=1 )//&& foreground_motion_vectors_angles.at(point_index) >=70 && foreground_motion_vectors_angles.at(point_index) <=110)
{
	approaching = approaching + 1;
// cout<<"OLD : "<<foreground_motion_vectors_points_old[point_index].y<<endl<<"NEW : "<<foreground_motion_vectors_points[point_index].y<<endl;
}

else
	departing = departing + 1;
}
approaching = (approaching * 100) / (cluster_points_indices.size());     //converting into %
departing = (departing * 100) / (cluster_points_indices.size());         //converting into %

// printf("Approaching: %d Departing:%d\n", approaching, departing);
float dist_from_bottom_point = 0;
float avg_dist_from_bottom_point = 0;

if(approaching >= THRESHOLD_FOR_CLASSIFYING_APPROACHING_OBJECTS)     // if cluster is approaching
{
// finding distance from bottom point
// cout<<"YES APPROACHING :) \n";
// printf("********************************************************************\n");
	for(
		int j = 0;
		j<cluster_points_indices.

		size();

		j++){
		dist_from_bottom_point = dist_from_bottom_point +
	norm(foreground_motion_vectors_points.at(cluster_points_indices.at(j)) -
		bottom_point.at(0));
}
// taking average disatnce
avg_dist_from_bottom_point = dist_from_bottom_point / cluster_points_indices.size();
}
//pushing average distance for cluster i.
cluster_dist_from_bottom_point.
push_back(avg_dist_from_bottom_point);

}
// finding cluster npo wih minimum distance from bottom point
float min_dist = INT_MAX;
int min_cluster_no = -1;
for(
	int i = 0;
	i<cluster_dist_from_bottom_point.

	size();

	i++){
// // printf("------ \nCluster: %d dist: %f  CLOSEST !!\n", i, cluster_dist_from_bottom_point.at(i));
	if(cluster_dist_from_bottom_point.
		at(i)
		!=0 && cluster_dist_from_bottom_point.
		at(i)
		< min_dist){
		min_dist = cluster_dist_from_bottom_point.at(i);
	min_cluster_no = i;
}
}



/////////////////// APPROACHING / URGENT END ////////////////////////////////////////////////////////////////////////////////////////












////// SHOWING POINTS ///////////////////////////////////////////////////////////////////////////////////////////////////

// TO SHOW CLUSTER POINTS

for(
	int i = 0;
	i<foreground_motion_vectors_points.

	size();

	i++)
{
	int l = labels.at<int>(i, 0);
	circle( frame, foreground_motion_vectors_points[i],
		3,
		Scalar(colors[l][0], colors[l][1], colors[l][2]
			), -1, 8);
	circle( frame, foreground_motion_vectors_points[i],
		3, Scalar(0,255,0), -1, 8);

}

// TO SHOW URGENT APPROACHING POINTS

for(
	int i = 0;
	i<foreground_motion_vectors_points.

	size();

	i++){
	int l = labels.at<int>(i, 0);
if(l==min_cluster_no){
	circle( frame, foreground_motion_vectors_points[i],
		3, Scalar(0,0,255), -1, 8);
// // cout<<"SHOWING CLUSTER "<<min_cluster_no<<endl;
}
}

//// POINT SHOWING DONE /////////////////////////////////////////////////////////////////////////////////////////////











//// NAVIGATION STARTS ////////////////////////////////////////////////////////////////////////////////////////////
if(min_cluster_no != -1 && counter%5 == 0){

	string instruction;
// cout<<min_cluster_no<<'\n';
	float urgent_centroid_x = centers.at<float>(min_cluster_no, 0);
// cout<<(float)urgent_centroid_x<<" ";
// cout<<centers<<"\n";
// max_x = max(max_x, (int)urgent_centroid_x);
// cout<<"Frame max x "<<frame.cols/2<<"\n";
	if(urgent_centroid_x<frame.cols/3)
        return (*env).NewStringUTF("MOVE LEFT");
	else if(urgent_centroid_x > 2*frame.cols/3)
        return (*env).NewStringUTF("MOVE RIGHT");
	else
        return (*env).NewStringUTF("MOVE FORWARD");
	cout<<instruction<<"\n";
// cout<<instruction<<" Max: "<<max_x<<"\n";
}


//// NAVIGATION ENDS ////////////////////////////////////////////////////////////////////////////////////////////////
}








foreground_motion_vectors_points.

clear();

foreground_motion_vectors_points_old.

clear();

foreground_motion_vectors_angles.

clear();

cluster_dist_from_bottom_point.

clear();

cluster_points_indices.

clear();

}

std::swap(points[1], points[0]
	);
cv::swap(prevGray, gray
	);
t2 = time(0);
if(
	difftime(t2, t1
		)>1){
    srand(time(NULL));
	int r1, r2, r3;
t1 = time(0);
needToInit = true;
needToFindK = true;
for(
	int i = 0;
	i<10;i++){
	r1 = rand() % 255;
r2 = rand() % 255;
r3 = rand() % 255;
colors[i][0] =
r1;
colors[i][1] =
r2;
colors[i][2] =
r3;
}
foreground_motion_vectors.

clear();

}
    return (*env).NewStringUTF("papararararara");
}

}