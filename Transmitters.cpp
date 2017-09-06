#include <map>
#include "Transmitters.hpp"

using namespace std;

void initTransmitters(map<int, Transmitter*> &txes)
{
	txes[1]		= new Transmitter("Huntingdon",					52.248652,  -0.34541632);
	txes[2]		= new Transmitter("Selsey",						50.762753,  -0.79222675);
	txes[3]		= new Transmitter("Kent",						51.005200,   0.88739097);
//	txes[4]		= new Transmitter("Roves Farm",					51.594303,  -1.6948185);
	txes[4]		= new Transmitter("Stratford-upon-Avon",		52.147024,  -1.5071244, TEXTALIGN_RIGHT);		// alternate 4
	txes[5]		= new Transmitter("Lowestoft",					52.581448,   1.5563448, TEXTALIGN_RIGHT);
	txes[6]		= new Transmitter("Asserby",					53.273217,   0.23136825);
	txes[7]		= new Transmitter("Southport",					53.620775,  -2.9806128, TEXTALIGN_RIGHT);
	txes[8]		= new Transmitter("Cowbridge",					51.442644,  -3.4975458, TEXTALIGN_RIGHT);
//	txes[9]		= new Transmitter("Swindon",					 10,  51.650225,  -1.6733554);
	txes[10]	= new Transmitter("Cumbria",					54.862266,  -3.1868761, TEXTALIGN_RIGHT);
	txes[11]	= new Transmitter("Greenock",					55.935688,  -4.8527571, TEXTALIGN_RIGHT);
	txes[12]	= new Transmitter("Alnwick",					55.354577,  -1.7026607);

	// Filey/Muston, 13 and 19. 13 is Southport-B
	// Don't know the master station so it isn't listed
	// Need master information (Active Patterns) from #3675RX.

	txes[14]	= new Transmitter("Kingsbridge",				50.221003,  -3.7279835, TEXTALIGN_RIGHT);
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


