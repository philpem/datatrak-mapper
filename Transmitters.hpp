#include <map>
#include <string>
#include <vector>


const int TEXTALIGN_LEFT   = -1;
const int TEXTALIGN_CENTRE = 0;
const int TEXTALIGN_RIGHT  = 1;

// forward declaration
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
		std::string		name;
		int				textAlign;	// -1 is left, 0 is centre, 1 is right
		std::vector<Slot*>	slots;

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
			for (std::vector<Slot*>::iterator it = this->slots.begin(); it != this->slots.end(); ++it)
			{
				if ((*it)->slaveSlot == slaveSlot) {
					return *it;
				}
			}
			return NULL;
		}
};


void initTransmitters(std::map<int, Transmitter*> &txes);

