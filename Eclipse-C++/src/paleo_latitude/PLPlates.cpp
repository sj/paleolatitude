/*
 * PLPlates.cpp
 *
 *  Created on: 3 Jul 2014
 *      Author: Sebastiaan J. van Schaik
 */

#include "PLPlates.h"

#include <algorithm>

#include "../util/Util.h"
#include "../util/Logger.h"

#include "kml/dom.h"
#include "kml/base/file.h"
#include "kml/engine.h"

#include "pugixml.hpp"

#include <boost/algorithm/string.hpp>

#include "../debugging-macros.h"
#include "exceptions/PLFileParseException.h"
#include <set>

using pugi::xml_document;
using namespace paleo_latitude;
using namespace std;


const unsigned int PLPlates::PLATE_ID_AFRICA = 701;


PLPlates::PLPlates() {}

PLPlates::~PLPlates() {
	for (const PLPlate* plate : _plates){
		delete plate;
	}
}

PLPlates* PLPlates::readFromFile(const string& filename) {
	PLPlates* res = new PLPlates();

	if (Util::string_ends_with(filename, ".kml")){
		res->_readPlatesFromKML(filename);
	} else if (Util::string_ends_with(filename, ".gpml")){
		res->_readPlatesFromGPML(filename);
	} else {
		throw PLFileParseException("Unsupported file format (expecting .kml or .gpml): " + filename);
	}

	return res;
}


void paleo_latitude::PLPlates::_readPlatesFromKML(const string& kml_filename) {
	// Read plates.kml
	string kml_data;

	Logger::logInfo("Reading plate coordinate data from " + kml_filename + "...");
	if (!kmlbase::File::ReadFileToString(kml_filename, &kml_data)){
		PLFileParseException plfe;
		plfe << "Error importing plate polygons from KML file: read failed (" << kml_filename << ")";
		throw plfe;
	}

	Logger::logInfo("Parsing plate data...");
	string kml_parse_errors;
	kmldom::ElementPtr kml_root_elem = kmldom::Parse(kml_data, &kml_parse_errors);

	if (kml_root_elem.get() == NULL){
		PLFileParseException plfe;
		plfe << "Error reading polygons from KML file: " << kml_parse_errors << " (" << kml_filename << ")";
		throw plfe;
	}

	kmlengine::ElementVector kml_polygons;
	kmlengine::GetElementsById(kml_root_elem, kmldom::Type_Polygon, &kml_polygons);

	// Iterate over all plates:
	for (unsigned int p = 0; p < kml_polygons.size(); p++){
		const kmldom::PolygonPtr polygon = kmldom::AsPolygon(kml_polygons[p]);

		const kmldom::ElementPtr polygon_parent = polygon->GetParent();

		kmldom::PlacemarkPtr placemark;
		if (polygon_parent->IsA(kmldom::Type_Placemark)){
			placemark = kmldom::AsPlacemark(polygon_parent);
		} else if (polygon_parent->IsA(kmldom::Type_MultiGeometry)){
			const kmldom::MultiGeometryPtr mgeom = kmldom::AsMultiGeometry(polygon_parent);
			placemark = kmldom::AsPlacemark(mgeom->GetParent());
		}

		if (placemark.get() == NULL){
			Logger::logWarn("Ignoring polygon - does not have parent Placemark");
			continue;
		}

		const string plate_name = placemark->get_name();

		kmlengine::ElementVector simpledata_fields;
		kmlengine::GetElementsById(placemark, kmldom::Type_SimpleData, &simpledata_fields);

		unsigned int plate_id = 0;
		string plate_id_str;

		for (unsigned int s = 0; s < simpledata_fields.size(); s++){
			const kmldom::SimpleDataPtr sdata = kmldom::AsSimpleData(simpledata_fields[s]);
			if (sdata->get_name() == "PLATEID1"){
				plate_id_str = sdata->get_text();

				if (!Util::string_to_something(plate_id_str, plate_id)){
					Logger::logWarn("Ignoring plate '" + plate_name + "': unable to parse plate ID");
					plate_id = 0;
				}

				break;
			}
		}

		if (plate_id == 0){
			Logger::logWarn("Ignoring plate '" + plate_name + "': unable to determine plate ID");
			continue;
		}

		kmldom::OuterBoundaryIsPtr boundary = polygon->get_outerboundaryis();
		kmldom::LinearRingPtr ring = boundary->get_linearring();
		kmldom::CoordinatesPtr kml_plate_coords = ring->get_coordinates();

		const unsigned int num_coords = kml_plate_coords->get_coordinates_array_size();

		vector<Coordinate>* poly_coords = new vector<Coordinate>();
		poly_coords->reserve(num_coords);

		for (unsigned int c = 0; c < num_coords; c++){
			kmlbase::Vec3 coords_tuple = kml_plate_coords->get_coordinates_array_at(c);
			poly_coords->push_back(Coordinate(coords_tuple.get_latitude(), coords_tuple.get_longitude()));
		}

		PLPlate* plate = new PLPlate(plate_id, plate_name, poly_coords);
		_plates.push_back(plate);
	}
}


const PLPlate* paleo_latitude::PLPlates::findPlate(double lat, double lon) const {
	return findPlate(Coordinate(lat, lon));
}

const PLPlate* paleo_latitude::PLPlates::findPlate(const Coordinate& site) const {
	const PLPlate* res = NULL;

	for (const PLPlate* plate : _plates){
		if (plate->contains(site)){
			if (res != NULL){
				// Already found a plate that contains this point? In exceptional circumstances,
				// a plate is contained by another plate. In such a situation, check whether
				// that is the case, and if so, use the most specific plate available.

				if (res->contains(*plate)){
					// This plate is fully contained in the previously found plate. Use
					// this plate instead.
					res = plate;
				} else if (plate->contains(*res)){
					// This plate fully contains the previously found plate. Keep the
					// previous plate as result
				} else {
					// Neither contains the other. That means that plates are
					// overlapping at the provided coordinate.
					Exception ex;
					ex << "Two (or possibly more) plates contain site " << site.to_string() << ": '" << res->getName() << "' (" << res->getId() << ") and '" << plate->getName() << "' (" << plate->getId() << ")";
					throw ex;
				}
			}
			res = plate;
		}
	}

	if (res == NULL){
		Exception ex;
		ex << "No plate found for site " << site.to_string() << "?" << endl;
		throw ex;
	}

	return res;
}

/**
 * Returns the plate parts as defined by the input data. Note that some plates consist
 * of multiple parts stitched together. In that case, every part is returned separately,
 * as if it were a 'plate' on its own. However, the plate IDs of those parts will be
 * the same.
 */
const vector<const PLPlate*> paleo_latitude::PLPlates::getPlates() const {
	return _plates;
}

/**
 * Returns the name of a plate based on the given ID
 */
string paleo_latitude::PLPlates::getPlateName(unsigned int plate_id) const {
	for (const PLPlate* plate : _plates){
		if (plate->getId() == plate_id) return plate->getName();
	}

	return "";
}

/**
 * Counts the actual number of plates (rather than the plate parts, which are
 * returned by #getPlates()). In other words: counts the number of unique plate IDs
 */
const int paleo_latitude::PLPlates::countRealNumberOfPlates() const {
	set<int> unique_ids;
	for (const PLPlate* plate : _plates) unique_ids.insert(plate->getId());
	return unique_ids.size();
}

/**
 * Method to read plate definitions from a GPML plate generated by GPlates (gplates.org)
 *
 * @param gpmlfilename
 */
void paleo_latitude::PLPlates::_readPlatesFromGPML(const string& gpmlfilename) {
	xml_document gpml;
	gpml.load_file(gpmlfilename.c_str());

	// Structure of XML file:
	// <gml:featureMember>							<-- one for each plate
	//  <gpml:UnclassifiedFeature>                  <-- sometimes '<gpml:ContinentalFragment>' or '<gpml:DisplacementPoint>'
	//   <gpml:reconstructionPlateId>
	//    <gpml:ConstantValue>
	//     <gpml:value>101</gpml:value>				<-- plate ID
	//     <gpml:valueType xmlns:gpml="http://www.gplates.org/gplates">gpml:plateId</gpml:valueType>
	//     (...)
	//
	//   <gpml:unclassifiedGeometry>
	//    <gpml:ConstantValue>
	//     <gpml:value>
	//      <gml:Polygon>
	//       <gml:exterior>
	//        <gml:LinearRing>
	//         <gml:posList gml:dimension="2">
	//          [lat1] [lon1] [lat2] [lon2]			<-- separated by spaces, latitude first

	// XPath to find XML nodes that specifies plate ID
	const string xpath_plate_ids = "//gpml:reconstructionPlateId/gpml:ConstantValue[gpml:valueType/text()='gpml:plateId']";

	// Iterate over all nodes that describe a plate ID
	pugi::xpath_node_set plate_ids_nodes = gpml.select_nodes(xpath_plate_ids.c_str());
	for (pugi::xpath_node node_constantvalue_plate_id : plate_ids_nodes){
		const pugi::xpath_node node_value_plate_id = node_constantvalue_plate_id.node().child("gpml:value");

		// Plate ID is hidden in
		// <gpml:reconstructionPlateId><gpml:ConstantValue><gpml:value>[PLATE_ID]
		const unsigned int plate_id = node_value_plate_id.node().text().as_uint();
		const string plate_id_str = node_value_plate_id.node().text().as_string();


		// Different GPML files have the 'gpml:UnclassifiedFeature' node at different levels.
		// Look for ancestor 'gpml:UnclassifiedFeature' by moving up the tree. However, it appears
		// that some GPML files use 'gpml:ContinentalFragment' or 'gpml:DisplacementPoint' nodes
		// instead of 'gpml:UnclassifiedFeature'
		const string xpath_unclassifiedFeature = ".//ancestor::gpml:UnclassifiedFeature";
		const string xpath_continentalfragment = ".//ancestor::gpml:ContinentalFragment";
		const string xpath_displacementpoint = ".//ancestor::gpml:DisplacementPoint";

		pugi::xpath_node node_uf_dp_cf = node_value_plate_id.node().select_single_node(xpath_unclassifiedFeature.c_str());

		if (node_uf_dp_cf.node().empty()){
			node_uf_dp_cf = node_value_plate_id.node().select_single_node(xpath_continentalfragment.c_str());
		}

		if (node_uf_dp_cf.node().empty()){
			node_uf_dp_cf = node_value_plate_id.node().select_single_node(xpath_displacementpoint.c_str());
		}

		if (node_uf_dp_cf.node().empty()){
			// Ancestor node <gpml:UnclassifiedFeature> not found
			Logger::logWarn("Could not find name and polygon definition for plate '" + plate_id_str + "' in GPML file (missing <gpml:UnclassifiedFeature> ancestor) - ignoring plate");
			continue;
		}

		const string xpath_plate_name = ".//gml:name";
		const pugi::xpath_node node_plate_name = node_uf_dp_cf.node().select_single_node(xpath_plate_name.c_str());

		if (node_plate_name.node().empty()){
			// No <gml:name>
			Logger::logWarn("Could not find name for plate '" + plate_id_str + "' in GPML file (missing <gml:name> element) - ignoring plate");
			continue;
		}

		const string plate_name = node_plate_name.node().text().as_string();


		// Find descendant 'gml:posList', which contains the coordinates of the line segments:
		// <gpml:UnclassifiedFeature>
		//  <gpml:unclassifiedGeometry>
		//   <gpml:ConstantValue>
		//    <gpml:value>
		//     <gml:Polygon>
		//      <gml:exterior>
		//       <gml:LinearRing>
		//        <gml:posList gml:dimension="2">
		const string xpath_posList = ".//gml:posList";
		const pugi::xpath_node node_poslist = node_uf_dp_cf.node().select_single_node(xpath_posList.c_str());

		if (node_poslist.node().empty()){
			// No <gml:posList> element
			Logger::logWarn("Could not find polygon definition for plate '" + plate_name + "' (id: " + plate_id_str + ") in GPML file (missing <gml:posList> element) - ignoring plate");
			continue;
		}

		string node_poslist_name = node_poslist.node().name();
		string node_coords_text = node_poslist.node().text().as_string();
		vector<string> coords_strs;
		boost::split(coords_strs, node_coords_text, boost::is_any_of(" "));

		vector<Coordinate>* poly_coords = new vector<Coordinate>();
		poly_coords->reserve(coords_strs.size() / 2.0);

		for (unsigned int i = 0; i < coords_strs.size() - 1; i+= 2){
			string str_lon = coords_strs[i+1];
			string str_lat = coords_strs[i];

			double lat = 0;
			double lon = 0;
			if (!Util::string_to_something(str_lat, lat)) throw PLFileParseException("Error parsing coordinate: " + str_lat);
			if (!Util::string_to_something(str_lon, lon)) throw PLFileParseException("Error parsing coordinate: " + str_lon);

			poly_coords->push_back(Coordinate(lat, lon));
		}

		PLPlate* plate = new PLPlate(plate_id, plate_name, poly_coords);
		_plates.push_back(plate);
	}


}
