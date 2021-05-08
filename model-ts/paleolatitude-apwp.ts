import fs from 'fs';

export const STUDENT_NAMES: string[] = ["JOHN","BOB","NICK"];
export const CSV_DATA: { [id: string] : string; } = {
    "besse-courtillot-2002-vandervoo-2015": "besse-courtillot-2002-vandervoo-2015.csv"
};

/**
 * Class to represent data on Apparent Polar Wander Paths (APWP). The Paleolatitude model can use
 * different sources of APWP data (see data directory), but will only ever use one such source
 * at any one time.
 */
export class PaleoLatitudeAPWP {
    
    constructor (apwp_data_id: string) {

    }

    /**
     * 
     * @returns object (map) containing all known APWP data files (CSV)
     */
    public static getKnownAPWPDataFiles() : { [id: string] : string; } {
        var res: { [id: string] : string; } = {};

        var files = fs.readdirSync('data');
        for (let filename of files) {
            if (filename.startsWith('apwp-') && filename.endsWith('.csv')) {
                var id = filename.slice(5, -4)
                res[id] = filename
            }
        }

        return res;
    }
}