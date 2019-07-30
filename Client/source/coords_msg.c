#include "../headers/coords_msg.h"

int get_coords_json(json_t *obj, const char **args) {
    double lat, lon;

    if ((lat = strtod(args[1], NULL)) && (lon = strtod(args[2], NULL))) {
        json_object_set(obj, "lat", json_real(lat));
        json_object_set(obj, "lon", json_real(lon));

        return 0;
    }

    return -1;
}