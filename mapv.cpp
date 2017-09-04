
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "CImg.h"

#include "WorldTransforms.hpp"

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


class Transmitter;

/**
 * A single transmitted nav Slot.
 *
 * Tracks master-slave relationships between transmitters (base stations).
 * The master originates the phase signal, the slave mirrors it.
 * Stations may transmit multiple navslots (e.g. F1 and F2), so the master/slave slots must be identified.
 */
class Slot {
	public:
		Transmitter *master;			///< The master transmitter.
		unsigned int masterSlot;		///< The slot number on the master which is being mirrored.
		unsigned int slaveSlot;			///< The slot number which the master phase is being mirrored to.

		/**
		 * Slot constructor.
		 *
		 * The <b>master</b> transmitter originates the phase signal in nav slot <b>masterSlot</b>.
		 * 
		 * The slave transmitter (the one which carries this <b>Slot</b> in its <b>slots</b> vector)
		 * mirrors the phase in a successive navslot.
		 *
		 * The <b>slots</b> vector is essentially equivalent to the Active Patterns list on a
		 * Datatrak receiver.
		 */
		Slot(Transmitter *master, const unsigned int masterSlot, const unsigned int slaveSlot)
		{
			this->master		= master;
			this->masterSlot	= masterSlot;
			this->slaveSlot		= slaveSlot;
		}
};


/**
 * A Transmitter (more appropriately a ground station).
 */
class Transmitter
{
	public:
		double			lat;
		double			lon;
		string			name;
		int				textAlign;	// -1 is left, 0 is centre, 1 is right
		vector<Slot*>	slots;

		Transmitter(const char *name, const double lat, const double lon, const int textAlign = -1)
		{
			this->name		= name;
			this->lat		= lat;
			this->lon		= lon;
			this->textAlign	= textAlign;
		}

		void addSlot(Transmitter* master, const unsigned int masterSlot, const unsigned int slaveSlot)
		{
			slots.push_back(new Slot(master, masterSlot, slaveSlot));
		}

		Slot *findSlot(const unsigned int slaveSlot)
		{
			for (vector<Slot*>::iterator it = this->slots.begin(); it != this->slots.end(); ++it)
			{
				if ((*it)->slaveSlot == slaveSlot) {
					return *it;
				}
			}
			return NULL;
		}
};


void initTransmitters(map<int, Transmitter*> &txes)
{
	txes[1]		= new Transmitter("Huntingdon",					52.248652,  -0.34541632);
	txes[2]		= new Transmitter("Selsey",						50.762753,  -0.79222675);
	txes[3]		= new Transmitter("Kent",						51.005200,   0.88739097);
//	txes[4]		= new Transmitter("Roves Farm",					51.594303,  -1.6948185);
	txes[4]		= new Transmitter("Stratford-upon-Avon",		52.147024,  -1.5071244, TEXTALIGN_RIGHT);		// alternate 4
	txes[5]		= new Transmitter("Lowestoft",					52.581448,   1.5563448, TEXTALIGN_RIGHT);
	txes[6]		= new Transmitter("Asserby",					53.273217,   0.23136825);	// slot A
	txes[7]		= new Transmitter("Southport",					53.620775,  -2.9806128);	// Slot A
	txes[8]		= new Transmitter("Cowbridge",					51.442644,  -3.4975458);
//	txes[9]		= new Transmitter("Swindon",					 10,  51.650225,  -1.6733554);
	txes[10]	= new Transmitter("Cumbria",					54.862266,  -3.1868761);
	txes[11]	= new Transmitter("Greenock",					55.935688,  -4.8527571);
	txes[12]	= new Transmitter("Alnwick",					55.354577,  -1.7026607);

	// Filey/Muston, 13 and 19. 13 is Southport-B
	// Don't know the master station so it isn't listed
	// Need master information (Active Patterns) from #3675RX.

	txes[14]	= new Transmitter("Kingsbridge",				50.221003,  -3.7279835);
	txes[15]	= new Transmitter("Stonehaven",					56.839071,  -2.3556843);


	// define active patterns (and Slot 1's role as the "root")
	txes[1]->addSlot(NULL, 0, 1);
	txes[2]->addSlot(txes[1], 1, 2);
	txes[3]->addSlot(txes[1], 1, 3);
	txes[4]->addSlot(txes[1], 1, 4);
	txes[5]->addSlot(txes[1], 1, 5);

	// Southport and Asserby are both multihomed (F1/F2 transition) transmitters.
	// This means they transmit an F1 and an F2 slot, and need to be added last.
	txes[6]->addSlot(txes[4], 4, 6);			// slot A
	txes[6]->addSlot(txes[10], 10, 16);		// slot B
	
	txes[7]->addSlot(txes[1], 1, 7);			// slot A
	txes[7]->addSlot(txes[10], 10, 13);		// slot B

	txes[8]->addSlot(txes[4], 4, 8);
//	txes[9]->addSlot(txes[1], 10, 9);
	txes[10]->addSlot(txes[7], 7, 10);
	txes[11]->addSlot(txes[10], 10, 11);
	txes[12]->addSlot(txes[10], 10, 12);
	txes[14]->addSlot(txes[4], 4, 14);
	txes[15]->addSlot(txes[10], 10, 15);
}


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

}
