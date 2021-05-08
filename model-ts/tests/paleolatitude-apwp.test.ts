import { expect } from 'chai';
import { PaleoLatitudeAPWP } from '../paleolatitude-apwp';


const APWP_CSV_files = [
  "data/apwp-torsvik-2012-vandervoo-2015.csv",
	"data/apwp-besse-courtillot-2002-vandervoo-2015.csv",
	"data/apwp-kent-irving-2010-vandervoo-2015.csv"
];

describe('apwp-data', function(){
  it('should report three APWP data files', function(){
    var files = PaleoLatitudeAPWP.getKnownAPWPDataFiles();
    expect(Object.keys(files).length).equal(3)

    expect(files["torsvik-2012-vandervoo-2015"]).equal("apwp-torsvik-2012-vandervoo-2015.csv");
    expect(files["besse-courtillot-2002-vandervoo-2015"]).equal("apwp-besse-courtillot-2002-vandervoo-2015.csv");
    expect(files["kent-irving-2010-vandervoo-2015"]).equal("apwp-kent-irving-2010-vandervoo-2015.csv");
  });
});

describe('calculate', function() {
    it('add', function() {
      const a = 5
      expect(a).equal(3);
    }); 
  });