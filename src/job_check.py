import os
import subprocess

from ROOT import TFile, TTree, gROOT

def main():

    #data_path = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC/skims/indiv_skims/'
    data_path = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC/skims/'

    for subdir, dirs, files in os.walk(data_path):
        #for file in os.listdir(data_path):
        for file in files:
            if '.py' in file: continue
            fname = str(subdir) + str('/') + str(file)
            #fname = str(data_path) + str (file)
            rfile = TFile(fname)
            print('Checking file', fname)
            tree = gROOT.FindObject('tr')
            test = str(tree)
            print()
            if test == '<ROOT.TObject object at 0x(nil)>':
                new_name = file.split('_')[:-2] + [str(file.split('_')[-2]) + str('.root')]
                new_file = '_'.join(new_name)
                print('Input file is bad. Now finding source file...')
                print()
                source_dir = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC'
                found_file = subprocess.check_output('find "%s" -name "%s"' % (source_dir, new_file), shell=True)
                print('Source file found:')
                print(found_file)
                ifile = found_file.decode('utf-8')
                print('Now resubmitting', ifile, 'with output of', fname)
                ofile = fname
                log = str('logs/') + str(file) + str('.log')
                print()
                print('bsub -q s -o %s "./skimmer %s %s"' % (log, ifile.strip('\n'), ofile))
                print()
                #input('which file?')
                #os.system('bsub -q s -o %s "./skimmer %s %s"' % (log, ifile.strip('\n'), ofile))
            print()

if __name__ == "__main__":
    main()