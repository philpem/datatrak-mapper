#include <fstream>
#include <string>

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
			std::ifstream fs(filename);
			std::string buf;

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

