/* mouv3.cpp
 * detecter les objets immobiles dans une video */

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

using namespace std;
using namespace cv;


/* compare les champs de la meme couleur de 2 pixels 
 * renvoie 1 si les couleurs sont semblables, 0 sinon
 */
bool compPixelCoul(uint16_t p1, uint16_t p2, int seuil){
	bool res = false;


	if((((p1-p2)<seuil) && ((p1-p2)>=0)) || (((p2-p1)<seuil) && ((p2-p1)>=0))){
		res = true;
	}
	
	return res;
}

/* compare 2 pixels
 * renvoie 1 si les pixels sont semblables
 */ 
bool compPixel(Vec3b pix1, Vec3b pix2, int seuil){
	bool res = false;
	
	if(compPixelCoul(pix1[0], pix2[0], seuil) 		// B
	&& compPixelCoul(pix1[1], pix2[1], seuil) 		// G
	&& compPixelCoul(pix1[2], pix2[2], seuil)){		// R
		res = true;
	}		
		
	return res;
}


/* supprime l'ellipse la plus eloignee des autres
 */
void suppPlusLoin(int& nbEllipses, Vector<RotatedRect>& tabBox, Point2f& centreMoy, int& rmoy){
	int i,j;
	float ecartMax = 0;
	float ecartMax2 = 0;
	int ellipseLoin = -1;

	if(nbEllipses>2){
		// determination de l'ellipse la plus lointaine (a partir de centre moy)
		for(i=0; i<nbEllipses; i++){
			ecartMax = sqrt(pow((centreMoy.x - tabBox[i].center.x),2) + pow((centreMoy.y - tabBox[i].center.y),2));
			if(ecartMax>ecartMax2){
				ellipseLoin = i;
				ecartMax2 = ecartMax;
			}
		}
			
		// recalcul de centre moyen et du rayon moyen sans l'ellipse la plus eloignee
		for(j=0; j<nbEllipses; j++){
			if(j!=ellipseLoin){
				centreMoy.x += tabBox[j].center.x;
				centreMoy.y += tabBox[j].center.y;
					
				rmoy += (tabBox[j].size.width + tabBox[j].size.height)/4;
			}
		}
		
		centreMoy.x = centreMoy.x/nbEllipses*2;
		centreMoy.y = centreMoy.y/nbEllipses*2;
		
		// moyenne rayon
		rmoy = rmoy/nbEllipses;
	}
}

/* supprime l'ellipse la moins ronde dans l'image
 */
void suppMoinsRonde(int& nbEllipses, Vector<RotatedRect>& tabBox, Point2f& centreMoy, int& rmoy){
	int ecartOvale = 0;
	int ecartOvale2 = 0;
	int ellipseMoinsRonde = -1;
	int i,j;

	if(nbEllipses>1){
		for(j=0; j<nbEllipses; j++){
			ecartOvale = abs(tabBox[j].size.width - tabBox[j].size.height);
			if(ecartOvale>ecartOvale2){
				ellipseMoinsRonde = j;
			}
		}
	}
	
	//cout << "fin suppression ellipse moins ronde" << endl;
	//cout << nbEllipses << endl;
	
	if(nbEllipses){
	// recalcul du cercle moyen
		for(i=0; i<nbEllipses; i++){
			//cout << "rené" << endl;
			if(i!=ellipseMoinsRonde){
				//cout << "josé" << endl;
				centreMoy.x += tabBox[i].center.x;
				centreMoy.y += tabBox[i].center.y;
					
				rmoy += (tabBox[i].size.width + tabBox[i].size.height)/4;
			}
		}
		
		centreMoy.x = centreMoy.x/nbEllipses*2;
		centreMoy.y = centreMoy.y/nbEllipses*2;
		
		// moyenne rayon
		rmoy = rmoy/nbEllipses*2;
	}
}


/* supprime l'ellipse la plus eloignee puis la moins ronde
 */
void supp2Ellipses(int& nbEllipses, Vector<RotatedRect>& tabBox, Point2f& centreMoy, int& rmoy){
	int ecartOvale = 0;
	int ecartOvale2 = 0;
	int ellipseMoinsRonde = -1;
	float ecartMax = 0;
	float ecartMax2 = 0;
	int ellipseLoin = -1;
	int i,j, k=0;
	Vector<RotatedRect>::iterator it1 = tabBox.begin();
	Vector<RotatedRect>::iterator it2;
	
	// detection de l'ellipse la moins ronde
	if(nbEllipses>1){
		for(i=0; i<nbEllipses; i++){
			ecartOvale = abs(tabBox[i].size.width - tabBox[i].size.height);
			if(ecartOvale>ecartOvale2){
				ellipseMoinsRonde = i;
			}
		}
		// suppression de l'ellipse
		//tabBox.erase(tabBox.begin()+ellipseMoinsRonde); // pb : erase existe pas
	}
	
	// detection de l'ellipse la plus eloignée
	if(nbEllipses>2){
		// determination de l'ellipse la plus lointaine (a partir de centre moy)
		for(i=0; i<nbEllipses; i++){
			ecartMax = sqrt(pow((centreMoy.x - tabBox[i].center.x),2) + pow((centreMoy.y - tabBox[i].center.y),2));
			if(ecartMax>ecartMax2){
				ellipseLoin = i;
				ecartMax2 = ecartMax;
			}
			
		}
		// suppression de l'ellipse
		//tabBox.erase(tabBox.begin()+ellipseLoin);
		
	}
	
	if(ellipseLoin!=-1) k++;
	if(ellipseMoinsRonde!=-1) k++; 
	
	if(nbEllipses){
		// recalcul du cercle moyen
		for(j=0; j<tabBox.size(); j++){
			if((j!=ellipseLoin) && (j!=ellipseMoinsRonde)){
				centreMoy.x += tabBox[j].center.x;
				centreMoy.y += tabBox[j].center.y;
			}	
			rmoy += (tabBox[j].size.width + tabBox[j].size.height)/4;
		}
		
		centreMoy.x = centreMoy.x/(nbEllipses-k)*2;
		centreMoy.y = centreMoy.y/(nbEllipses-k)*2;

		rmoy = rmoy/nbEllipses*2;
	}
}





int main(int argc, char** argv){
	VideoCapture video(0);
	Mat im;		// image
	Mat imprec;	// image precedente
	Mat imaff;	// image affichee (im pas modifiee -> imprec)
	Mat imgris;	// imaff en niveaux de gris
	Mat canny;	// images apres canny (que les contours)
	
	Mat ref = imread("~/mouv/anneau_simple.png", CV_LOAD_IMAGE_GRAYSCALE); // image de reference pour le SURF
	
	char key;
	int width, height, i, j, i1, j1, i2, i3, i4, i5, i6, i7, i8, i9;
	int seuil = 30;			// seuil de detection ( difference max entre 2 couleurs)
	int resolution = 1;		// taille des blocs de pixels etudiés
	int attente = 50;		// attente entre 2 images de la video (en ms)
	int canny_bas = 100;		// seuil bas pour le canny
	int canny_haut = 100;	// seuil haut pour le canny
	
	int bord = 5; // distance a partir du bord ou les ellipses sont negligees
	
	int cpt = 0;
	float moyenne = 0;
	
	vector<vector<Point> > contours;
	vector<vector<Point> > contours2;
	
	Point2f centreMoy;	
	Point2f centreAncien;
	int rmoy = 0;


	if(!video.isOpened()) cout << "video introuvable" << endl;
	else{
		// creation d'une fenetre avec trackbar
		cvNamedWindow("Francis", CV_WINDOW_AUTOSIZE);
		
		// parametres
        int width = 160;
        int height = 120;
        video.set(CV_CAP_PROP_FRAME_WIDTH, 160);
        video.set(CV_CAP_PROP_FRAME_HEIGHT, 120);
        video.set(CV_CAP_PROP_FPS, 60);
	
		// lecture de la video
		video.read(im);
				
		// recuperation des donnees des images
        //width = video.get(CV_CAP_PROP_FRAME_WIDTH);
        //height = video.get(CV_CAP_PROP_FRAME_HEIGHT);
		
		centreMoy.x = width/2;
		centreMoy.y = height/2;
		centreAncien.x = width/2;
		centreAncien.y = height/2;
		
		// copies pour initialiser les images
		im.copyTo(imaff);
		im.copyTo(imprec);
		
		// affichage de la video
		while(key!='a'){			// on sort en appuyant sur a
			key = cvWaitKey(attente); 	// attente
			
			// recup image
			video.read(im);
			
			// copies
			im.copyTo(imaff);

// ---------- effacement des zones immobiles --------------------------
			for(j=0; j<height; j++){		// lignes
				for(i=0; i<width; i++){		// colonnes
					
					if((!(i%resolution)) && (!(j%resolution))){
						
						// comparaison de im avec imprec
						if(compPixel(im.at<cv::Vec3b>(j,i), imprec.at<cv::Vec3b>(j,i), seuil)){
							
							// pixels semblables : effacement du pixel sur im (blanc : 255 255 255)
							imaff.at<cv::Vec3b>(j,i)[0] = 255;
							imaff.at<cv::Vec3b>(j,i)[1] = 255;
							imaff.at<cv::Vec3b>(j,i)[2]	= 255;	
							
							// effacement d'un bloc de pixels
							for(i1=0; i1<resolution; i1++){
								for(j1=0; j1<resolution; j1++){
									imaff.at<cv::Vec3b>(j+j1,i+i1)[0] = 255;
									imaff.at<cv::Vec3b>(j+j1,i+i1)[1] = 255;
									imaff.at<cv::Vec3b>(j+j1,i+i1)[2] = 255;
								}
							}
						}
					}
				}
			}
			
// ---------- detection des contours sur imaff ------------------------
			
			// matrice vide
			Mat cimage = Mat::zeros(imaff.size(), CV_8UC3);
			i9 = 0;
			
			// passage en niveaux de gris
			cvtColor(imaff, imgris, CV_RGB2GRAY, 0);		
			//GaussianBlur(imgris, imgris, Size(3, 3), 2, 2);
			
			Canny(imgris, canny, canny_bas, canny_haut, 3);
			GaussianBlur(canny, canny, Size(3, 3), 2, 2);
			// detection des contours
 			findContours(canny, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE); 		
 				
			int nbEllipses = 0; // nombre d'ellipses
			Vector<RotatedRect> tabBox(contours.size()); // svg des differentes box

 			// traitement des contours
			for(size_t i2 = 0; i2 < contours.size(); i2++){
				
				//drawContours(imaff, contours, (int)i2, Scalar::all(255), 1, 8);
				
				// elimination des contours trop petits (moins de 6 pi)
				size_t count = contours[i2].size();
				if( count < 6 ) continue;
	
				// suppression des contours non convexes -> marche pas
				//if(!isContourConvex(contours[i2])) continue;
	
				Mat pointsf;
				Mat(contours[i2]).convertTo(pointsf, CV_32F);
				
				RotatedRect box = fitEllipse(pointsf);
        
				box.angle = -box.angle;
				// suppression des ellipses pas assez rondes
				if( MAX(box.size.width, box.size.height) > MIN(box.size.width, box.size.height)*2 ) continue;
				// suppression des ellipses trop petites
				if( MAX(box.size.width, box.size.height) < 10) continue;
				
// ---------- dessin des ellipses, svg infos des ellipses -------------
				
				//drawContours(cimage, contours, (int)i2, Scalar::all(255), 1, 8);
				//drawContours(imaff, contours, -1, Scalar::all(255), 1, 8);
				
				// svg contours convexes
				//contours2[i9] = contours[i2];
				//i9++;
				
				nbEllipses++;	// nombre d'ellipses
				tabBox[nbEllipses] = box;	// RotatedRect correspondant aux ellipses
				ellipse(imaff, box, Scalar(0,255,0), 1, CV_AA);	// dessin
								
			}			
			
			//cout << "nb ellipses " << nbEllipses << endl;
			cpt++;
			moyenne += nbEllipses;
			//cout << "contours" << contours.size() << " " << contours2.size() << endl;
			
// --------------------------------------------------------------------			
// ---------- detection de l'anneau -----------------------------------
// --------------------------------------------------------------------								
								
								
								
// ---------- centre moyen des ellipses -------------------------------
//-> peu precis tout seul			
			
			//centreMoy.x = 0;
			//centreMoy.y = 0;
			//rmoy = 0;
			
			// si il n'y a pas d'ellipses on garde le cercle moyen d'avant
			if(!nbEllipses){
			//	centreMoy.x = width/2;
			//	centreMoy.y = height/2;
			}
			else{	
				centreMoy.x = 0;
				centreMoy.y = 0;
				rmoy = 0;	
					
				for(i3=0; i3<nbEllipses; i3++){
					// supression des ellipses trop proches du bord
					if(!((tabBox[i3].center.x<bord) || (tabBox[i3].center.x>(width-bord)) || (tabBox[i3].center.y<bord) || (tabBox[i3].center.y>(height-bord)))){
						centreMoy.x += tabBox[i3].center.x;
						centreMoy.y += tabBox[i3].center.y;
						
						rmoy += (tabBox[i3].size.width + tabBox[i3].size.height)/2;
					}
				}
				// moyenne centre avec ancien centre pondéré
				//centreMoy.x = (((centreMoy.x+(centreAncien.x*2)))/(nbEllipses+2)); // remonte en haut a gauche
				//centreMoy.y = (((centreMoy.y+(centreAncien.y*2)))/(nbEllipses+2));
				
				// moyenne centre
				centreMoy.x = centreMoy.x/nbEllipses*2;
				centreMoy.y = centreMoy.y/nbEllipses*2;
				
				// moyenne rayon
				rmoy = rmoy/nbEllipses;
			}
		
		//cout << "fin traitement" << endl;
		
// ---------- suppression d'ellipses ----------------------------------
			
            suppPlusLoin(nbEllipses, tabBox, centreMoy, rmoy);
			
			//suppMoinsRonde(nbEllipses, tabBox, centreMoy, rmoy);
			
			//supp2Ellipses(nbEllipses, tabBox, centreMoy, rmoy);



			
// ---------- affichage -----------------------------------------------

			//if(nbEllipses){	// pas d'affichage si pas d'ellipses
				line(imaff, centreMoy, centreMoy, Scalar(0,0,255), 5, 8, 0);
				circle(imaff, centreMoy, rmoy, Scalar(0,0,255), 5, 8, 0);
			//}
			
			centreAncien.x = centreMoy.x;
			centreAncien.y = centreMoy.y;
			
			// aff imaff
			imshow("Francis", imaff);
			//imshow("Francis", cimage);
			
			// svg imprec <- im
			im.copyTo(imprec);		
				
		}	// fin while (lecture de la video)		
	} // fin else (video ouverte)
	
	cout << "Moyenne nb ellipses/image : " << moyenne/cpt << endl;

	// liberation memoire
	video.release();

	
	return 0;
}
