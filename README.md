TPC Refitter:

This is a program written to handle the BASF2 output ntuples produced by Igal
analysis/simulation suite and process them with only the minimal information
needed for doing TPC analysis.

Instructions:
The source code is in the src directory.  The refitter.C file has all of the 
fitting and other scripts.  The compiled refitter file is the executable that 
process the data.  It takes an input file and an output file as arguments:
"./refitter <input_data_file> <output_data_file>".

The run_refitter.py is a steering file that loops over directories to find root 
files that match a certain criteria.  It then passes files into the refitter 
program, produces output files, and sorts them.  Additionally, it submits a job 
to bsub for every file and manages log files by default.

I only implemented tags as of v2 of BEAST ntuples.  I am keeping v2 as the first
tag so the naming convention is consistent with the BEAST ntuple versions.
