import glob
import os

### Debug info: Implementing tools to measure time to completion
import timeit

### This was an attempt to recreate the NTP/TPC subdirectory structure, but it
### wasn't worth the hassle.
#ifpath = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC'
#ofpath = '/home/belle/mhedges/tpc_skimmer/data/'
#for subdirs in os.walk(ifpath):
#    print subdirs[0]
#    raw_input('looking into sub_subdir structure')
#    if subdirs[0].split('/')[-1] == 'TPC':
#        for subdir in subdirs[1]:
#            print 'subdir is ', subdir
#            newdir = str(ofpath) + str(subdir)
#            #os.system('mkdir %s' % newdir)
#            #os.system('ls -lhrt')
#            print newdir
#            raw_input('good so far?')
#    print raw_input('Going on to next subdir, should be a date')

start = timeit.default_timer()

### Paths for KEKCC
ifpath = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC'
ofpath = '/home/belle/mhedges/beast/phase1/data/tpc_skimmer/data/'

#ifpath = '/Volumes/Hedges_stuff/beast_phase1/data/TPC/kekcc'
#ofpath = '/Volumes/Hedges_stuff/beast_phase1/data/TPC/tpc_skimmer/data'

good_run = 1

### Debug variables
counter = 1
for subdir, dirs, files in os.walk(ifpath):
    for f in files:
        test = subdir.split('/')
        for i in test:
            if i  == 'badtime' or i  == 'old' or i == 'ENV' or i == 'tmp':
                good_run = 0
        if good_run == 0:
            continue
        if counter >= 5:
            break
        ifile = os.path.join(subdir, f)
        print ifile
        names=f.split('/')
        infile_name=names[-1].split('.')
        ofile = str('data/') + str(infile_name[0]) + str('_skim') + str('.root')
        if os.path.isfile(ofile):
            continue
        os.system('./skimmer %s %s' % (ifile, ofile))
        #counter += 1
        #raw_input('good so far?')

stop = timeit.default_timer()

### Print debug info
#print ''
#print 'Total run time was: ', stop - start, ' seconds'
