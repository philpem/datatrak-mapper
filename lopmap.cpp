
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

const float TEXT_ALPHA = 0.5f;				///< Text transparency level

const double BLANKING_DISTANCE = 200.0e3;	///< Blanking distance in metres. If distance to master or slave exceeds this, the LOP will not be plotted.
const double LOP_RANGE = 15.0;				///< Plot the LOP only if its phase is within this number of degrees of zero.
const double FREQ = 122.0e3 / 10.;			///< Transmitter frequency. Divided by ten so we only show every 10th LOP.

const double SPEED_OF_LIGHT = 299792458;	///< Speed of light in metres per second.

const unsigned char MARKERCOLOURS[][3] = {
	{ 255, 255, 255 },
	{ 255,   0,   0 },
	{ 255, 255,   0 },
	{   0, 255,   0 },
	{   0, 255, 255 },
	{   0,   0, 255 }
};

const unsigned char white[] = {255,255,255};
const unsigned char black[] = {0,0,0};





int main(int argc, char **argv)
{

	// read the worldfile
	CWorldfile wf(TFW_FILE);

	// load map file
	CImg<unsigned char> img(MAP_FILE);
	// crop the map
	img.crop(CROP_X0, CROP_Y0, CROP_X1, CROP_Y1);


	// define transmitters
	map<int, Transmitter*> txes;
	initTransmitters(txes);



	// plot txes on map
	for (map<int, Transmitter*>::iterator it=txes.begin(); it!=txes.end(); ++it)
	{
		long me_x, me_y;

		// used to build the tx label
		stringstream txlabel;
		txlabel << "(";

		// convert lat/lon to X/Y
		wf.coord2pix(it->second->lat, it->second->lon, me_x, me_y);
		me_x -= CROP_X0;
		me_y -= CROP_Y0;

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

				img.draw_line(me_x, me_y, master_x, master_y, MARKERCOLOURS[relayDepth]);
			}
		}
		txlabel << ") " << it->second->name;

		*/

		// mark the station
		img.draw_circle(me_x, me_y, 5, MARKERCOLOURS[0], 1);

		// put text on the transmitter

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


	// plot all the baselines and LOPs

	// select the baseline to plot
//	txa = txes[1];
//	txb = txes[2];

	// get A/B transmitter lon/lat
	double a_lat = txes[1]->lat;
	double a_lon = txes[1]->lon;
	double b_lat = txes[2]->lat;
	double b_lon = txes[2]->lon;


	// iterate over X/Y pixels
	cimg_forXY(img, x, y) {
		// convert X/Y offset to lon/lat
		double lon, lat;
		wf.pix2coord(x+CROP_X0, y+CROP_Y0, lon, lat);

		// calculate great-circle distance in metres
		double dist_a = vincenty_distance(a_lat, a_lon, lat, lon) * 1000.0;
		double dist_b = vincenty_distance(b_lat, b_lon, lat, lon) * 1000.0;

#if 1
		// skip this point if the distance to TX A or TX B exceeds 200km
		// (Datatrak Locator blanking distance)
		if ((dist_a > 200.0e3) || (dist_b > 200.0e3)) {
			//basemap(x, y) = 0.0;
			continue;
		}
#endif


		// calculate difference in distance
		double dist_diff = abs(dist_a - dist_b);

		// calculate phase shift (via the differential distance)
		const double CYCTIME = 1.0 / FREQ;

		double propagation_time = dist_diff / SPEED_OF_LIGHT;	// seconds
		double phase_shift = fmod(propagation_time, CYCTIME) * (360.0 / CYCTIME);

		// plot only if LOP phase is within +/- <RANGE> of zero
		if ((phase_shift < LOP_RANGE) || (phase_shift > (360.0 - LOP_RANGE))) {
			img(x,y,0) = MARKERCOLOURS[0][0];// * (phase_shift * 255.0 / 360.0);
			img(x,y,1) = MARKERCOLOURS[0][1];// * (phase_shift * 255.0 / 360.0);
			img(x,y,2) = MARKERCOLOURS[0][2];// * (phase_shift * 255.0 / 360.0);
		}
	}


//	basemap.normalize(0,255);
//	basemap.display("LOPs");

	img.display("Datatrak transmitters with LOPs shown");

}
