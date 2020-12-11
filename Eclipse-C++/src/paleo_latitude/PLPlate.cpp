
#include "PLPlate.h"

#include "pugixml.hpp"

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/algorithm/string/case_conv.hpp>


#include <random>
#include <vector>
#include "../util/Logger.h"
#include "../util/Util.h"
#include "../util/Exception.h"


namespace bg = boost::geometry;

using namespace paleo_latitude;
using pugi::xml_document;

PLPlate::PLPlate(unsigned int plate_id, string plate_name, vector<Coordinate>* polygon_coordinates) :
				_id(plate_id), _name(PLPlate::_filterPlateName(plate_name)), _polygon_coordinates(polygon_coordinates)
{
	assert(_polygon_coordinates != NULL);
}

string PLPlate::_filterPlateName(const string plate_name){
	string plate_name_lower = boost::to_lower_copy(plate_name);
	if (plate_name_lower.find("unconstrained") != string::npos|| plate_name_lower.find("mobile") != string::npos){
		return "mobile belt (unconstrained)";
	} else {
		return plate_name;
	}
}

/**
 * In exceptional circumstances, plates are fully contained within another plate. This
 * method tests whether that's the case. By checking whether the other plate's coordinates
 * are inside this plate.
 *
 * Note that this is an expensive operation - only use it when really necessary!
 */
bool PLPlate::contains(const PLPlate& other_plate) const {
	// Count number of coordinates of other_plate that sit within this plate
	unsigned int poly_coordinates_in_this_plate = 0;
	for (unsigned int i = 0; i < other_plate._polygon_coordinates->size(); i++){
		const Coordinate& other_poly_coord = (*other_plate._polygon_coordinates)[i];
		poly_coordinates_in_this_plate += this->contains(other_poly_coord) ? 1 : 0;
	}

	double fraction_in = (double)poly_coordinates_in_this_plate / other_plate._polygon_coordinates->size();

	// Assume the other plate is fully located within this plate if > 90% of its polygon
	// coordinates fall within this plate.
	return fraction_in > 0.9;
}

bool PLPlate::contains(const Coordinate& site) const {
	typedef boost::geometry::model::point<double, 2, boost::geometry::cs::geographic<boost::geometry::degree> > point_type;
	typedef boost::geometry::model::segment<point_type> segment_type;

	const point_type pnt_site(site.longitude, site.latitude);

	// Shooting a ray to a fixed target causes all sorts of problems. So: shoot a series of rays to
	// coordinates that are guaranteed to be in different plates.
	// One of the ray targets might be on the same plate as the site_coord, which will render that
	// ray. Also: the plates of the targets will cause a single false positive each.

	vector<Coordinate> ray_targets = {
			Coordinate(89, 0),					// north pole = North American plate
			Coordinate(-89, 0),					// south pole = East Antarctic plate
			Coordinate(-18.933333, 47.516667),	// Antananarivo = Madagascar plate
			Coordinate(21.3, -157.816667),		// Honolulu, Hawaii = Pacific plate
			Coordinate(64.175, -51.738889),		// Nuuk, Greenland = Greenland plate
			Coordinate(9.06, 7.47),				// Abuja, Nigeria = African plate
			Coordinate(51.5, -0.1),				// London = European plate
			Coordinate(-33.8, 151.2),			// Sydney = Australian plate
			Coordinate(45,-93.25),				// Minneapolis = North American plate
			Coordinate(-22.9, -42.2),			// Rio de Janeiro = South American plate
			Coordinate(14.75, -17.45),			// Dakar = NW African plate
			Coordinate(22.3, 114.16)			// Hong Kong
	};

	vector<segment_type> rays;
	for (const Coordinate& ray_target : ray_targets){
		const point_type pnt_ray_target(ray_target.longitude, ray_target.latitude);
		segment_type ray(pnt_site, pnt_ray_target);
		rays.push_back(ray);
	}

	vector<unsigned int> ray_targets_intersection_counts(ray_targets.size(), 0);


	for (unsigned int i = 0; i < _polygon_coordinates->size(); i++){
		const Coordinate& curr = (*_polygon_coordinates)[i];
		const Coordinate& next = (*_polygon_coordinates)[(i+1) % _polygon_coordinates->size()];

		if (abs(curr.longitude - next.longitude) > 270){
			// Line likely crosses date boundary on 180 degree meridian. Rather than
			// drawing a very short line segment, Boost will draw one all the way
			// around the sphere. Given that multiple rays are used, we can afford to
			// skip this single segment.
			continue;
		}

		const point_type pnt_curr(curr.longitude, curr.latitude);
		const point_type pnt_next(next.longitude, next.latitude);

		const segment_type plate_segment(pnt_curr, pnt_next);

		for (unsigned int r = 0; r < rays.size(); r++){
			bool intersects = bg::intersects(rays[r], plate_segment);

			if (intersects) ray_targets_intersection_counts[r]++;
		}
	}

	// Each of the ray targets will have an intersection count that represents the number
	// of polygon lines that ray intersected
	unsigned int votes_inside = 0;
	unsigned int votes_outside = 0;
	for (unsigned int intersections : ray_targets_intersection_counts){
		if (intersections % 2 == 0) votes_outside++;
		else votes_inside++;
	}

	assert(votes_inside + votes_outside == ray_targets.size());

	if (votes_inside > 2 * votes_outside){
		// Point is inside plate
		return true;
	} else if (votes_outside > 2 * votes_inside){
		// Point not in plate
		return false;
	}

	Exception ex;
	ex << "Could not determine with sufficient certainty whether site (" << site.to_string() << ") is on plate '" << _name << "' (" << _id << "). Is the site on the border of two plates? (Details: in/out ray votes are " << votes_inside << " vs " << votes_outside << ")";
	throw ex;
}



PLPlate::~PLPlate() {
	delete _polygon_coordinates;
	_polygon_coordinates = NULL;
}

string PLPlate::getName() const {
	return _name;
}

unsigned int PLPlate::getId() const {
	return _id;
}

const vector<Coordinate>* PLPlate::getCoordinates() const {
	return _polygon_coordinates;
}


string PLPlate::_ppCoordinates() const {
	stringstream ss;

	for (Coordinate coord: (*_polygon_coordinates)){
		ss << coord.to_string() << "  ";
	}

	return ss.str();
}

string Coordinate::to_string() const {
	stringstream ss;
	ss << latitude << "," << longitude;
	return ss.str();
}

void paleo_latitude::PLPlate::writeKMLPlacemark(ostream& output_stream, string linecolor) const {
	output_stream << "<Placemark>" << endl
		<< " <name>" << this->getName() << " (" << this->getId() << ")</name>" << endl
		<< " <Style><LineStyle><color>" << linecolor << "</color></LineStyle><PolyStyle><fill>0</fill></PolyStyle></Style>" << endl
		<< " <Polygon><outerBoundaryIs><LinearRing><coordinates>" << endl;

	// Add coordinates in comma-separated pairs of lon,lat:
	for (const Coordinate& coord : *_polygon_coordinates){
		output_stream << coord.longitude << "," << coord.latitude << " ";
	}
	output_stream << endl;

	output_stream << " </coordinates></LinearRing></outerBoundaryIs></Polygon>" << endl;
	output_stream << "</Placemark>" << endl;
}
