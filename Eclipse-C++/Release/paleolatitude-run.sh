#!/bin/bash

this_params="$@"
age_args="--age=50 --age-pm=20"

if [ "$this_params" = "" ]; then
	args="$age_args"
else
	args=""
fi

./PaleoLatitude --site-lat=52.366667 --site-lon=4.9 $args $@
