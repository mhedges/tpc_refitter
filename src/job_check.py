import os
import subprocess

from ROOT import TFile, TTree, gROOT

def main():

    #data_path = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC/skims/indiv_skims/'
    data_path = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC/skims/'

    r_files = []

    counter = 0
    for subdir, dirs, files in os.walk(data_path):
        #for file in os.listdir(data_path):
        for f in files:
            if '.py' in f: continue
            fname = str(subdir) + str('/') + str(f)
            #fname = str(data_path) + str (f)
            rfile = TFile(fname)
            print('Checking file', fname)
            tree = gROOT.FindObject('tr')
            test = str(tree)
            print()
            if test == '<ROOT.TObject object at 0x(nil)>':
                new_name = f.split('_')[:-2] + [str(f.split('_')[-2]) + str('.root')]
                new_file = '_'.join(new_name)
                print('Input file is bad. Now finding source file...')
                print()
                source_dir = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC'
                date_dir = fname.split('/')[-2]

                iname_dir = str(source_dir) + str('/') + str(date_dir)
                iname = f.replace('_string', '')

                #print(f.replace('_string', ''))
                #print(f)
                #print(new_file)
                #input('is the name right?')

                #found_file = subprocess.check_output('find "%s" -name "%s"' % (source_dir, new_file), shell=True)
                #found_file = str(source_dir) + str('/')
                #print('Source file found:')
                #print(found_file)
                #ifile = found_file.decode('utf-8')
                
                ifile = str(iname_dir) + str('/') + str(new_file)

                print('Now resubmitting', ifile, 'with output of', fname)

                ofile = fname
                log = str('logs/') + str(f) + str('.log')
                print()
                #print('bsub -q s -o %s "./skimmer %s %s"' % (log, ifile.strip('\n'), ofile))
                print()
                #input('which file?')
                os.system('bsub -q s -o %s "src/refitter %s %s"' % (log, ifile.strip('\n'), ofile))
                counter += 1
            print()

    if counter == 0:
        r_files = []
        for subdir, dirs, files in os.walk(ifpath):
            for f in files:
                if '.py' in f: continue
                
                r_file = str(subdir) + str(f)
                ifile = os.path.join(subdir, f)
                print(ifile)
                if os.path.getsize(ifile) < 1000:
                    info = []
                    info.append(ifile)
                    info.append(os.path.getsize(ifile))
                    print('File is less than 1 Kb. Size =', os.path.getsize(ifile))
                    r_files.append(info)
                    os.remove(ifile)
        print('Empty root files:', r_files)
        print('Number of empty files =', len(r_files))

if __name__ == "__main__":
    main()