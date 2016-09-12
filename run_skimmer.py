#import glob
import os

### Debug info: Implementing tools to measure time to completion
#import timeit

### Paths for KEKCC
ifpath = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC'
ofpath = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC/skims/indiv_skims/'
#ofpath = '/home/belle/mhedges/beast/phase1/data/tpc_skimmer/data/'

#ifpath = '/Volumes/Hedges_stuff/beast_phase1/data/TPC/kekcc'
#ofpath = '/Volumes/Hedges_stuff/beast_phase1/data/TPC/tpc_skimmer/data'

good_run = 1

### Debug variables
counter = 1
r_files = []
for subdir, dirs, files in os.walk(ifpath):
    for f in files:
        r_file = str(subdir) + str(f)
        if 'skim' not in r_file:
            continue
        r_files.append(r_file)

#print r_files
#raw_input('Does it contain skim files?')

for subdir, dirs, files in os.walk(ifpath):
    for f in files:
        r_file = str(subdir) + str(f)
        if r_file in r_files:
            continue
        test = subdir.split('/')
        if 'TPC3' in test or 'TPC4' in test or 'skims' in test:
            continue
        for i in test:
            if i  == 'badtime' or i  == 'old' or i == 'ENV' or i == 'tmp' or i == 'ToRemove':
                good_run = 0
        if good_run == 0:
            continue
        ifile = os.path.join(subdir, f)
        print(ifile)
        names=f.split('/')
        infile_name=names[-1].split('.')
        if '_skim' in infile_name:
            continue
        #ofile = str('data/') + str(infile_name[0]) + str('_skim') + str('.root')
        tfile = str(infile_name[0]) + str('_skim') + str('.root')
        match = 0
        #for d in r_files:
        #    if tfile in d:
        #        match = 1
        #if match == 1:
        #    continue

        ofile = str(ofpath) + str(infile_name[0]) + str('_skim') + str('.root')

        #if os.path.isfile(ofile):
        #    continue
        log = str('logs/') + str(f) + str('.log')
        os.system('bsub -q s -o %s "./skimmer %s %s"' % (log, ifile, ofile))
        #os.system("./skimmer %s %s" % (ifile, ofile))
        #counter += 1
        #raw_input('good so far?')
        #print 'Output file is, ', ofile
        #raw_input('Does the file exist?')
        #raw_input('does it submit properly?')

### Print debug info
#print ''
#print 'Total run time was: ', stop - start, ' seconds'
