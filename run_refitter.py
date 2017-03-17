import os
import datetime
import sys
from root_numpy import root2rec

def main():
    ### Paths for KEKCC
    ifpath = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC/'
    #ofpath = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC/skims/indiv_skims/'

    ### Debug variables
    counter = 0
    r_files = []

    for subdir, dirs, files in os.walk(ifpath):
        for f in files:
            ofpath = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/'
            r_file = str(subdir) + str(f)

            test = subdir.split('/')

            #if 'TPC3' in test or 'TPC4' in test or 'skims' in test:
            #    continue

            if 'skims' in test:
                continue

            tpc_num = f.split('_')[0]
            date_dir = subdir.split('/')[-1]
            if 'TPC4' in test :
                date_dir = '2016-05-10'
                print('Date dir is:', date_dir)
            #print('Date dir is:', date_dir)
            print('Directory is:', subdir)

            if tpc_num == 'tpc3':
                ofpath += str('TPC3/')
            elif tpc_num == 'tpc4':
                ofpath += str('TPC4/')

            ofpath += str(date_dir) + str('/')

            if ('badtime' in test or 'old' in test or 'ENV' in test or 'tmp' in
                    test or 'ToRemove' in test):
                continue

            ifile = os.path.join(subdir, f)

            names=f.split('/')
            infile_name=names[-1].split('.')

            tfile = str(infile_name[0]) + str('_skim') + str('.root')
            match = 0

            ofile = str(ofpath) + str(infile_name[0]) + str('_skim') + str('.root')


            ### Uncomment this line if only non-existing files are to be generated
            #if os.path.isfile(ofile): continue

            counter += 1

            ### Uncomment these lines if all files must be regenerated
            #input('Warning! You are about to delete all existing files!')
            if os.path.isfile(ofile): os.system('rm %s' % (ofile))

            print('Infile is:', ifile)
            print('Outfile is:', ofile)

            log = str('logs/') + str(f) + str('.log')

            #os.system('bsub -q s -o %s "./refitter %s %s"' % (log, ifile, ofile))
            ### Send large files to long queue, small files to short queue
            df = root2rec(ifile, 'tree', branches='m_event')
            evts = len(df)
            print(evts)

            if evts > 60000:
                os.system('bsub -q l -o %s "./refitter %s %s"' % (log, ifile, ofile))
            else:
                os.system('bsub -q s -o %s "./refitter %s %s"' % (log, ifile, ofile))
            input('Did it work?')

    if counter == 0:
        sys.path.append('py')
        import job_check
        job_check.main()
        #import emptyfile_cleaner
        #emptyfile_cleaner.main()

if __name__ == "__main__":
    main()
