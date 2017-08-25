
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "CImg.h"


using namespace std;
using namespace cimg_library;


const char * MAP_FILE = "natural_earth_3/2_no_clouds_16k.jpg";		// JPEG/PNG/etc file containing the map
const char * TFW_FILE = "natural_earth_3/2_no_clouds_16k.tfw";		// Esri worldfile for the map

const long CROP_X0 = 7615;				// Crop, X pixel coord, top-left corner
const long CROP_Y0 = 1370;				// Crop, Y pixel coord, top-left corner
const long CROP_X1 = CROP_X0 + 585;		// Crop, X pixel coord, bottom-right corner
const long CROP_Y1 = CROP_Y0 + 445;		// Crop, Y pixel coord, bottom-right corner



const int TEXTALIGN_LEFT   = -1;
const int TEXTALIGN_CENTRE = 0;
const int TEXTALIGN_RIGHT  = 1;

const float TEXT_ALPHA = 0.5f;

// TODO: WGS84Coord class
// TODO: PixelCoord class
// TODO: OSGBCoord class with conversions from/to LonLatCoord (friend of WGS84Coord)

/**
 * Worldfile class.
 *
 * Reads Esri Worldfiles (TFW format) and uses the data contained within to
 * perform coordinate conversions between lat/lon (Easting/Northing) and pixel
 * coordinates on a map (e.g. JPEG or PNG file).
 *
 * The algorithms used herein were obtained from Wikipedia:
 *   https://en.wikipedia.org/wiki/World_file
 * Reference date: 14-Aug-2017
 */
class CWorldfile {

	private:
		double xPixSz;				///< X pixel size (A)
		double yPixSz;				///< Y pixel size (E)
		double xRot;				///< X-rotation (B)
		double yRot;				///< Y-rotation (D)
		double xOrigin;				///< X origin of map (C)
		double yOrigin;				///< Y origin of map (F)

	public:
		CWorldfile(const char * filename)
		{
			ifstream fs(filename);
			string buf;

			// see https://en.wikipedia.org/wiki/World_file for details on a World file

			fs >> this->xPixSz;		// A
			fs >> this->yRot;		// D
			fs >> this->xRot;		// B
			fs >> this->yPixSz;		// E
			fs >> this->xOrigin;	// C
			fs >> this->yOrigin;	// F
		}

		/**
		 * Convert a pixel location into a geographic longitude and latitude.
		 *
		 * @param[in]	px		Pixel X location.
		 * @param[in]	py		Pixel Y location.
		 * @param[out]	lat		Latitude (geo-Y) in degrees (Northing).
		 * 						North is positive, South is negative.
		 * @param[out]	lon		Longitude (geo-X) in degrees (Easting).
		 * 						East is positive, West is negative.
		 */
		void pix2coord(const long px, const long py, double &lon, double &lat)
		{
			lon = this->xOrigin + (py * this->xRot) + (px * this->xPixSz);
			lat = this->yOrigin + (px * this->yRot) + (py * this->yPixSz);
		}

		/**
		 * Convert a geographic longitude and latitude into a pixel location.
		 *
		 * @param[in]	lat		Latitude (geo-Y) in degrees (Northing).
		 * 						North is positive, South is negative.
		 * @param[in]	lon		Longitude (geo-X) in degrees (Easting).
		 * 						East is positive, West is negative.
		 * @param[out]	px		Pixel X location.
		 * @param[out]	py		Pixel Y location.
		 */
		void coord2pix(const double lat, const double lon, long &px, long &py)
		{
			px = round(((this->yPixSz * lon) - (this->xRot * lat) + (this->xRot * this->yOrigin) - (this->yPixSz * this->xOrigin)) / ((this->xPixSz * this->yPixSz) - (this->yRot * this->xRot)));
			py = round(((this->xPixSz * lat) + (-this->yRot * lon) + (this->yRot * this->xOrigin) - (this->xPixSz * this->yOrigin)) / ((this->xPixSz * this->yPixSz) - (this->yRot * this->xRot)));
		}
};


class Transmitter
{
	public:
		double			lat;
		double			lon;
		unsigned int	master;
		string			name;
		int				textAlign;	// -1 is left, 0 is centre, 1 is right

		Transmitter(const char *name, const unsigned int master, const double lat, const double lon, const int textAlign = -1)
		{
			this->lat		= lat;
			this->lon		= lon;
			this->master	= master;
			this->name		= name;
			this->textAlign	= textAlign;
		}
};


int main(int argc, char **argv)
{

	// read the worldfile

	CWorldfile wf(TFW_FILE);

	long pix, piy;

	wf.coord2pix(52.393035, 5.106178, pix, piy);


	// load map file
	CImg<unsigned char> img(MAP_FILE);


	// define transmitters
	map<int, Transmitter*> txes;
	txes[1]		= new Transmitter("Huntingdon",					  0,  52.248652,  -0.34541632);
	txes[2]		= new Transmitter("Selsey",						  1,  50.762753,  -0.79222675);
	txes[3]		= new Transmitter("Kent",						  1,  51.005200,   0.88739097);
	
//	txes[4]		= new Transmitter("Roves Farm",					  1,  51.594303,  -1.6948185);
	txes[4]		= new Transmitter("Stratford-upon-Avon",		  1,  52.147024,  -1.5071244, TEXTALIGN_RIGHT);		// alternate 4
	
	txes[5]		= new Transmitter("Lowestoft",					  1,  52.581448,   1.5563448, TEXTALIGN_RIGHT);
	
	txes[6]		= new Transmitter("Asserby",					  4,  53.273217,   0.23136825);	// slot A
	txes[16]	= new Transmitter("Asserby",					 10,  53.273217,   0.23136825, TEXTALIGN_RIGHT);	// slot B
	
	txes[7]		= new Transmitter("Southport",					  1,  53.620775,  -2.9806128);	// Slot A
	txes[13]	= new Transmitter("Southport",					 10,  53.620775,  -2.9806128, TEXTALIGN_RIGHT);	// Slot B

	txes[8]		= new Transmitter("Cowbridge",					  4,  51.442644,  -3.4975458);

//	txes[9]		= new Transmitter("Swindon",					 10,  51.650225,  -1.6733554);

	txes[10]	= new Transmitter("Cumbria",					  7,  54.862266,  -3.1868761);

	txes[11]	= new Transmitter("Greenock",					 10,  55.935688,  -4.8527571);

	txes[12]	= new Transmitter("Alnwick",					 10,  55.354577,  -1.7026607);

	// Filey/Muston, 13 and 19. 13 is Southport-B
	// Don't know the master station so it isn't listed
	// Need master information (Active Patterns) from #3675RX.

	txes[14]	= new Transmitter("Kingsbridge",				  4,  50.221003,  -3.7279835);

	txes[15]	= new Transmitter("Stonehaven",					 10,  56.839071,  -2.3556843);

	// marker colour
	unsigned char markerColours[][3] = {
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

		// convert lat/lon to X/Y
		wf.coord2pix(it->second->lat, it->second->lon, me_x, me_y);
		//me_x -= CROP_X0;
		//me_y -= CROP_Y0;

		cout << "plotting " << it->second->name << "; lon " << it->second->lon << ", lat " << it->second->lat << endl;
		cout << "\tX: " << me_x << endl;
		cout << "\tY: " << me_y << endl;

		// figure out how many times the signal has been relayed
		int relayDepth = 0;
		int nextHop = it->second->master;
		while (nextHop != 0) {
			nextHop = txes[nextHop]->master;
			relayDepth++;
		}
		cout << "\trelay depth: " << relayDepth << endl;
		
		// mark the station
		img.draw_circle(me_x, me_y, 5, markerColours[relayDepth], 1);

		// draw line to master station
		if (it->second->master != 0) {
			Transmitter* ma = txes[it->second->master];
			wf.coord2pix(ma->lat, ma->lon, master_x, master_y);

			img.draw_line(me_x, me_y, master_x, master_y, markerColours[relayDepth-1]);
		}

		// put text on the transmitter
		unsigned char white[] = {255,255,255};
		unsigned char black[] = {0,0,0};
		stringstream ss;
		// Station alternates are identified by adding a multiple of 100 to the slot number
		ss << "(" << it->first % 100 << ") " << it->second->name;

		// draw text on a temporary image first, so we can measure the width and height
		CImg<unsigned char> imgtext;
		imgtext.draw_text(0,0,ss.str().c_str(),white,black,1);

		if (it->second->textAlign < 0) {
			// left align
			img.draw_text(me_x, me_y, ss.str().c_str(), white, black, TEXT_ALPHA);
		} else if (it->second->textAlign == 0) {
			// centre
			img.draw_text(me_x - (imgtext.width() / 2), me_y, ss.str().c_str(), white, black, TEXT_ALPHA);
		} else {
			// right
			img.draw_text(me_x - imgtext.width(), me_y, ss.str().c_str(), white, black, TEXT_ALPHA);
		}
	}

	// crop the map image appropriately (default is to cover the UK only)
	img.crop(CROP_X0, CROP_Y0, CROP_X1, CROP_Y1);
	img.display("Map");

}
