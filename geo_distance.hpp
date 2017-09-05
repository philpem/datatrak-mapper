/*
 * both of these functions return the distance in km
 *
 * code originally from: https://gist.github.com/ed-flanagan/e6dc6b8d3383ef5a354a
 * explanation: http://www.dtcenter.org/met/users/docs/write_ups/gc_simple.pdf
 */

/**
 * Calculate the great-circle distance between two points on Earth using
 * the Haversine formula.
 *
 * This function calculates the
 * <a href="https://en.wikipedia.org/wiki/Great-circle_distance">Great-circle Distance</a>
 * between two points on Earth (given in longitude and latitude) using the
 * Haversine formula.
 *
 * Its accuracy is approximately 0.5% (per the above Wikipedia article),
 * primarily due to assuming the Earth is a perfect sphere (in fact, it's
 * slightly elliptical).
 *
 * The result is given in kilometres.
 *
 * @param	latitude1		Latitude of point 1
 * @param	longitude1		Longitude of point 1
 * @param	latitude2		Latitude of point 2
 * @param	longitude2		Longitude of point 2
 * @returns Distance between points 1 and 2, in kilometres.
 */
double haversine_distance(const double latitude1, const double longitude1,
                          const double latitude2, const double longitude2);

/**
 * Calculate the distance between two points on Earth using Vincenty's Formulae.
 *
 * This function uses <a href="https://en.wikipedia.org/wiki/Vincenty%27s_formulae">Vincenty's Formulae</a>
 * to calculate the distance between two points on Earth (given in longitude
 * and latitude).
 *
 * This is somewhat more accurate than Haversine (great-circle distance),
 * being accurate to around 0.5mm on the Earth ellipsoid.
 *
 * The result is given in kilometres.
 *
 * @param	latitude1		Latitude of point 1
 * @param	longitude1		Longitude of point 1
 * @param	latitude2		Latitude of point 2
 * @param	longitude2		Longitude of point 2
 * @returns Distance between points 1 and 2, in kilometres.
 */
double vincenty_distance(const double latitude1, const double longitude1,
                         const double latitude2, const double longitude2);
