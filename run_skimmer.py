import os
import datetime
import job_check
import emptyfile_cleaner

def main():
    ### Paths for KEKCC
    ifpath = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC/'
    #ofpath = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC/skims/indiv_skims/'

    ### Debug variables
    counter = 0
    r_files = []

    for subdir, dirs, files in os.walk(ifpath):
        for f in files:
            ofpath = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC/skims/'
            r_file = str(subdir) + str(f)

            test = subdir.split('/')

            if 'TPC3' in test or 'TPC4' in test or 'skims' in test:
                continue

            tpc_num = f.split('_')[0]
            date_dir = subdir.split('/')[-1]
            print('Date dir is:', date_dir)

            if tpc_num == 'tpc3':
                ofpath += str('TPC3/')
            elif tpc_num == 'tpc4':
                ofpath += str('TPC4/')

            ofpath += str(date_dir) + str('/')

            good_run = 1
            for i in test:
                if i  == 'badtime' or i  == 'old' or i == 'ENV' or i == 'tmp' or i == 'ToRemove':
                    good_run = 0

            ifile = os.path.join(subdir, f)

            if good_run == 0:
                continue

            names=f.split('/')
            infile_name=names[-1].split('.')

            tfile = str(infile_name[0]) + str('_skim') + str('.root')
            match = 0

            ofile = str(ofpath) + str(infile_name[0]) + str('_skim') + str('.root')
            #if os.path.isfile(ofile): continue
            counter += 1

            if os.path.isfile(ofile): os.system('rm %s' % (ofile))

            print('Infile is:', ifile)
            print('Outfile is:', ofile)
            #input('Did it work?')

            log = str('logs/') + str(f) + str('.log')
            os.system('bsub -q s -o %s "./skimmer %s %s"' % (log, ifile, ofile))
    if counter == 0:
        job_check.main()
        emptyfile_cleaner.main()

if __name__ == "__main__":
    main()
