const VERSION = "3.0.0-alpha1"

import { Command } from 'commander';
const program = new Command();

/**
 * Define command-line parameters
 */
program
    .version(VERSION)
    .description("PaleoLatitude model command-line interface (CLI)");

program
    .option('-v, --verbose', 'Print out additional debugging information')
    .command('run-tests', "Runs the model's internal test suite");

/**
 * Command: compute
 */
const compute = new Command('compute');

compute
  .option('--pm-ref-frame <frame>', "Paleomagnetic reference frame to use: torsvik-vandervoo, besse-courtillot, kent-irving", 'torsvik-vandervoo')
  .option('-i, --time-interval', 'Specifies the time interval for the "compute" command (default: compute for all time)')
  .action((name, options, command) => {
    if (options.debug) {
      console.error('Called %s with options %o', command.name(), options);
    }
    const title = options.title ? `${options.title} ` : '';
    console.log(`Thank-you ${title}${name}`);
  });

program.addCommand(compute)



// Header
program.addHelpText('beforeAll',
`This is PaleoLatitude version ${VERSION} (http://www.paleolatitude.org)
Implementation by Sebastiaan J. van Schaik (https://github.com/sj/paleolatitude)
Source code licensed under the GNU Lesser GPL (LGPL) version 3.0
`)


// Include an example in the additional help text
program.addHelpText('after', `

Example invokation:
$ node paleolatitude-model compute 50.3,300 --model blah --interval -500:-50`);

// Include citation information after each run
program.addHelpText('afterAll', `

Please cite:
  Douwe J.J. van Hinsbergen, Lennart V. de Groot, Sebastiaan J. van Schaik,
  Appy Sluijs, Peter K. Bijl, Wim Spakman, Cor G. Langereis, Henk Brinkhuis:
  A Paleolatitude Calculator for Paleoclimate Studies
  In: PLoS ONE, 2015 (http://doi.org/10.1371/journal.pone.0126946).`)


program.parse(process.argv)

let message = "hello world paleolatitude-cli.ts";
console.log(message)
