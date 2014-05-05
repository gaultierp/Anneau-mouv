/* mouv5.cpp
* detection d'un anneau a partir d'une video
* suivi de l'anneau par ses points d'interet */

#include <vector>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include "math.h"

#define PI 3.14159265359

using namespace std;
using namespace cv;

/* tests if there is a point of interest around the Point (at a distance max distMax)
* puts the points of interest which are on the circle in newCorners
* 
*/
void testPtIntCircle(vector<Point2f>& corners, vector<Point2f>& newCorners, Point2f& c, int r1, int r2, int distMax, Mat& im){
	double i,j, k=0;
	Point2f p;

	cout << "fonction" << endl;

	// points
	for(j=0; j<corners.size(); j++){
		// circle
		for(i=0; i<2*PI; i+=0.01){	 // i describes the circles

			// if the point is between the 2 circles -> i describe the circle
			if((corners[j].x <= c.x+(r1*cos(i))) && (corners[j].x >= c.x+(r2*cos(i)))	 // X
			&& (corners[j].y <= c.y+(r1*sin(i))) && (corners[j].y >= c.y+(r2*sin(i)))){	 // Y 

				// backup of each point only once
				if(k==0){
					newCorners[k].x = corners[j].x;
					newCorners[k].y = corners[j].y;
					k++;
					cout << "pt trouve" << endl;
				}
				else{	
					if((corners[j].x!=newCorners[k-1].x) || (corners[j].y!=newCorners[k-1].y)){
						newCorners[k].x = corners[j].x;
						newCorners[k].y = corners[j].y;
						k++;
						cout << "pt trouve" << endl;
					}
				}


				cout << "k " << k << endl;

				line(im, corners[j], corners[j], Scalar(255,255,255), 6, 8, 0);

			}
		}
	}
	
	// suppression of empty spots
	for(i=newCorners.size(); i>0; i--){
		if((newCorners[i].x) == 0 && (newCorners[i].y == 0)){
			newCorners.pop_back();
		}
	} 

	cout << "j " << j << endl;
	cout << "fin fonction" << endl;

}


int main(int argc, char** argv){
	VideoCapture video(0);
	Mat im;	 // image
	Mat imprec;	// image precedente
	Mat imaff;	// image affichee (im pas modifiee -> imprec)
	Mat imref;
	Mat imrefgris;
	Mat imrefgrisAncienne;
	Mat imHSV;

	vector<Mat> channels;

	// circles to calibrate
	Point2f centerColor;
	int radiusColor;
	int radiusColor2;

	// goodfeaturestotrack
	double qualityLevel = 0.01;
	double minDistance = 10;
	int blockSize = 3;
	bool useHarrisDetector = false;	
	double k = 0.04;
	int maxCorners = 500;

	vector<Point2f> corners;
	vector<Point2f> newCorners(20*sizeof(Point2f));	// init sinon seg fault -> taille a optimiser => 0 a la fin supp ds la fonction
	vector<Point2f> oldCorners;
	Size winSize(10,10);
	TermCriteria termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03);
	
	vector<unsigned char> status;
	//vector<float> err;
	Mat err;

	double i;
	int j, j1;

	char key;
	int width, height;
	int seuil = 30;	 // seuil de detection ( difference max entre 2 couleurs)
	int resolution = 1;	 // taille des blocs de pixels etudi��s
	int attente = 50;	 // attente entre 2 images de la video (en ms)


	// ---------- lecture of the video ------------------------------------

	if(!video.isOpened()) cout << "video introuvable" << endl;
	else{
		// creation d'une fenetre
		cvNamedWindow("Francis", CV_WINDOW_AUTOSIZE);
		//cvNamedWindow("Francis1", CV_WINDOW_AUTOSIZE);
		//cvNamedWindow("Francis2", CV_WINDOW_AUTOSIZE);

		// lecture de la video
		video.read(im);
		cout << "lecture vid OK" << endl;	

		// recuperation des donnees des images
		width = video.get(CV_CAP_PROP_FRAME_WIDTH);
		height = video.get(CV_CAP_PROP_FRAME_HEIGHT);
		centerColor.x = width/2;
		centerColor.y = height/2;
		radiusColor = width/4;
		radiusColor2 = width/5;
       
       // copies pour initialiser les images
		im.copyTo(imaff);
		im.copyTo(imprec);

// ---------- choice of the reference picture -------------------------



		while(key!='a'){
			video.read(im);
			key = cvWaitKey(attente);
			circle(im, centerColor, radiusColor, Scalar(0,0,255), 5, 8, 0);
			circle(im, centerColor, radiusColor2, Scalar(0,0,255), 5, 8, 0);
			imshow("Francis", im);
		}

		// recovery of the referent
		video.read(imref);
		cout << "im ref OK" << endl;

// ---------- choice of the points of interest ------------------------	


		// grayscale
		//cvtColor(imref, imrefgris, CV_RGB2GRAY);

		// H of the HSV picture
		cvtColor(im, imHSV, CV_RGB2HSV, 0);
		split(imHSV, channels);
		imrefgris = channels[0];


		// creation of the points of interest
		goodFeaturesToTrack(imrefgris, corners, maxCorners, qualityLevel, minDistance, Mat(), blockSize, useHarrisDetector, k );

		cout << "nb pt interet total : " << corners.size() << endl;

		cout << "pt interet OK" << endl;

		// only the points of interest which are on the circle are kept
		testPtIntCircle(corners, newCorners, centerColor, radiusColor, radiusColor2, 0, im);

		cout << "detection cercle pt interet OK" << endl;

		// coordinates of the points between the circles
		for(i=0; i<newCorners.size(); i++){
			cout << i << " " << newCorners[i].x << " " << newCorners[i].y << endl;
		}


// ---------- screens -------------------------------------------------	

		// draw all the points
		for(i=0; i<corners.size(); i++){
			line(im, corners[i], corners[i], Scalar(0,255,0), 5, 8, 0);
		}

		//cout << corners.size() << endl;
		cout << "nb pt interet cercle : " << newCorners.size() << endl;

// ---------- tracking ------------------------------------------------

		oldCorners = newCorners;
		j=0;

		while(key!='z'){
			cout << "Tracking" << endl;
			key = cvWaitKey(attente);
			
			// reading video
			video.read(im);
			
			// grayscale
			cvtColor(im, imrefgris, CV_BGR2GRAY);
			cvtColor(im, imrefgrisAncienne, CV_BGR2GRAY);
			
			// H of the HSV picture -> more precise
			//cvtColor(im, imHSV, CV_RGB2HSV, 0);
			//split(imHSV, channels);
			//imrefgris = channels[0];
			//imrefgrisAncienne = channels[0];
		
			cout << "taille OldCorners : " << oldCorners.size() << endl;
			cout << "taille NewCorners : " << newCorners.size() << endl;

			// tracking
			calcOpticalFlowPyrLK(imrefgrisAncienne, imrefgris, oldCorners, newCorners, status, err, winSize, 3, termcrit, 0); // flow trouvé mais pas de modif des points
		
			// draw corners
			for(i=0; i<newCorners.size(); i++){
				line(im, newCorners[i], newCorners[i], Scalar(255,0,0), 5, 8, 0);
			}	
			
			// aff newCorners		-> tout le tps la meme chose
			//cout << "aff coordonnees NC" << endl;
			//for(i=0; i<newCorners.size(); i++){
			//	cout << newCorners[i].x << " " << newCorners[i].y << endl;
			//}
		
			imshow("Francis", im);
			//imshow("Francis1", imrefgris);
			//imshow("Francis2", imrefgrisAncienne);
			
			cout << "        j : " << j << endl;	// number of iterations
			j++;
				
			// backups
			imrefgris.copyTo(imrefgrisAncienne);	// OK
			
			oldCorners.resize(newCorners.size());
			//oldCorners = newCorners;
			//oldCorners.swap(newCorners);			// -> pb recopie de new dans old	=> resize ?
			
			for(i=0; i<newCorners.size(); i++){
				oldCorners[i].x = newCorners[i].x;
				oldCorners[i].y = newCorners[i].y;
			}
			
			//cout << "svg ancienne image ok" << endl;
			
			/*			
			cout << "    NC" << endl;
			for(i=0; i<newCorners.size(); i++){
				cout << newCorners[i].x << " " << newCorners[i].y << endl;
			}
			cout << "    OC" << endl;
			for(i=0; i<oldCorners.size(); i++){
				cout << oldCorners[i].x << " " << oldCorners[i].y << endl;
			}*/
		}
		
	}

	//corners.release();
	//newCorners.release();
	cout << "fin" << endl;
	video.release();

	return 0;
}
