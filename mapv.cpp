
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "CImg/CImg.h"

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
		long master_x, master_y;
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

		// mark the station
		img.draw_circle(me_x, me_y, 5, markerColours[0], 1);

		// put text on the transmitter
		unsigned char white[] = {255,255,255};
		unsigned char black[] = {0,0,0};

		// draw text on a temporary image first, so we can measure the width and height
		CImg<unsigned char> imgtext;
		imgtext.draw_text(0,0,txlabel.str().c_str(),white,black,1);

		if (it->second->textAlign < 0) {
			// left align
			img.draw_text(me_x, me_y, txlabel.str().c_str(), white, black, TEXT_ALPHA);
		} else if (it->second->textAlign == 0) {
			// centre
			img.draw_text(me_x - (imgtext.width() / 2), me_y, txlabel.str().c_str(), white, black, TEXT_ALPHA);
		} else {
			// right
			img.draw_text(me_x - imgtext.width(), me_y, txlabel.str().c_str(), white, black, TEXT_ALPHA);
		}
	}

	// crop the map image appropriately (default is to cover the UK only)
	img.crop(CROP_X0, CROP_Y0, CROP_X1, CROP_Y1);
	img.display("Map");
	img.save("map_txrefs.png");

}
