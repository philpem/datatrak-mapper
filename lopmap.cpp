
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

#include "CImg.h"

#include "geo_distance.hpp"

#include "WorldTransforms.hpp"
#include "Transmitters.hpp"

using namespace std;
using namespace cimg_library;


const char * MAP_FILE = "natural_earth_3/2_no_clouds_16k.jpg";		// JPEG/PNG/etc file containing the map
const char * TFW_FILE = "natural_earth_3/2_no_clouds_16k.tfw";		// Esri worldfile for the map

const long CROP_X0 = 7615;				// Crop, X pixel coord, top-left corner
const long CROP_Y0 = 1370;				// Crop, Y pixel coord, top-left corner
const long CROP_X1 = CROP_X0 + 585;		// Crop, X pixel coord, bottom-right corner
const long CROP_Y1 = CROP_Y0 + 445;		// Crop, Y pixel coord, bottom-right corner



const float TEXT_ALPHA = 0.5f;



int main(int argc, char **argv)
{

	// read the worldfile
	CWorldfile wf(TFW_FILE);

	// load map file
	CImg<unsigned char> img(MAP_FILE);

	// define transmitters
	map<int, Transmitter*> txes;
	initTransmitters(txes);


	// marker colour
	unsigned char markerColours[][3] = {
		{ 255, 255, 255 },
		{ 255,   0,   0 },
		{ 255, 255,   0 },
		{   0, 255,   0 },
		{   0, 255, 255 },
		{   0,   0, 255 }
	};

	// plot txes on map
	for (map<int, Transmitter*>::iterator it=txes.begin(); it!=txes.end(); ++it)
	{
		long me_x, me_y;

		// used to build the tx label
		stringstream txlabel;
		txlabel << "(";

		// convert lat/lon to X/Y
		wf.coord2pix(it->second->lat, it->second->lon, me_x, me_y);
		//me_x -= CROP_X0;
		//me_y -= CROP_Y0;

		cout << "plotting " << it->second->name << "; lon " << it->second->lon << ", lat " << it->second->lat << endl;
		cout << "\tX, Y: " << me_x << ", " << me_y << endl;


		/*

		// Scan all the slots this transmitter provides
		const char *sep = "";
		for (vector<Slot*>::iterator si=it->second->slots.begin(); si!=it->second->slots.end(); ++si)
		{
			// figure out how many times the signal has been relayed
			int relayDepth = 0;
			Slot *s = *si;
			cout << "\tslot " << s->slaveSlot << " path:" << endl;

			txlabel << sep << s->slaveSlot;
			sep = ",";

			if (!s) {
				cout << "ERROR - cannot resolve slot list for this TX" << endl;
				break;
			}

			while (s && s->masterSlot != 0) {
				cout << "\t\t " << s->master->name << " " << s->slaveSlot << "->" << s->masterSlot << endl;
				s = s->master->findSlot(s->masterSlot);
				relayDepth++;
			}

			cout << "\trelay depth: " << relayDepth << endl;

			// draw line to master station
			if ((*si)->masterSlot != 0) {
				Transmitter* ma = (*si)->master;
				wf.coord2pix(ma->lat, ma->lon, master_x, master_y);

				img.draw_line(me_x, me_y, master_x, master_y, markerColours[relayDepth]);
			}
		}
		txlabel << ") " << it->second->name;

		*/

		// mark the station
		img.draw_circle(me_x, me_y, 5, markerColours[0], 1);

		// put text on the transmitter
		unsigned char white[] = {255,255,255};
		unsigned char black[] = {0,0,0};

		// draw text on a temporary image first, so we can measure the width and height
		CImg<unsigned char> imgtext;
		imgtext.draw_text(0,0,it->second->name.c_str(),white,black,1);

		if (it->second->textAlign < 0) {
			// left align
			img.draw_text(me_x, me_y, it->second->name.c_str(), white, black, TEXT_ALPHA);
		} else if (it->second->textAlign == 0) {
			// centre
			img.draw_text(me_x - (imgtext.width() / 2), me_y, it->second->name.c_str(), white, black, TEXT_ALPHA);
		} else {
			// right
			img.draw_text(me_x - imgtext.width(), me_y, it->second->name.c_str(), white, black, TEXT_ALPHA);
		}

	}

	// select the baseline to plot
//	txa = txes[1];
//	txb = txes[2];

	// get A/B transmitter lon/lat
	double a_lat = txes[1]->lat;
	double a_lon = txes[1]->lon;
	double b_lat = txes[2]->lat;
	double b_lon = txes[2]->lon;


	// create a temporary CImg to store our map
	CImg<float> basemap(CROP_X1-CROP_X0, CROP_Y1-CROP_Y0);

	// iterate over X/Y pixels
	cimg_forXY(basemap, x, y) {
		// convert X/Y offset to lon/lat
		double lon, lat;
		wf.pix2coord(x+CROP_X0, y+CROP_Y0, lon, lat);

		// calculate great-circle distance in metres
		double dist_a = vincenty_distance(a_lat, a_lon, lat, lon) * 1000.0;
		double dist_b = vincenty_distance(b_lat, b_lon, lat, lon) * 1000.0;

#if 1
		// abort if distance exceeds 200km (Datatrak Locator blanking distance)
		if ((dist_a > 200.0e3) || (dist_b > 200.0e3)) {
			basemap(x, y) = 0.0;
			continue;
		}
#endif

	//	cout << "lon " << lon << " lat " << lat;
	//	cout << "   distA " << dist_a << endl;
		//cout << "   distB " << dist_b << endl;

		// calculate propagation delay
		const double SPEED_OF_LIGHT = 299792458;	// metres per second

		// calculate difference in distance
		double dist_diff = abs(dist_a - dist_b);

		// calculate phase shift
		const double FREQ = 122.0e3 / 10.;	// 122 kHz -- show only every 10th LOP
		const double CYCTIME = 1.0 / FREQ;

		double propagation_time = dist_diff / SPEED_OF_LIGHT;	// seconds
		double phase_shift = fmod(propagation_time, CYCTIME) * (360. / CYCTIME);

		// plot!
		const double RANGE = 45.0;
		if ((phase_shift < RANGE) || (phase_shift > (360.0 - RANGE))) {
			basemap(x,y) = phase_shift;
		} else {
			basemap(x,y) = 0.0;
		}

		//basemap(x, y) = phase_shift;
	}


//	basemap.normalize(0,255);
	basemap.display("LOPs");

	// crop the map image appropriately (default is to cover the UK only)
//	img.crop(CROP_X0, CROP_Y0, CROP_X1, CROP_Y1);
//	img.display("Map");

}
