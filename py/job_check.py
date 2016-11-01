import os
import subprocess

from ROOT import TFile, TTree, gROOT

def main():

    #data_path = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC/skims/indiv_skims/'
    #data_path = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC/skims/'
    data_paths = (['/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC3/',
        '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC4'])

    r_files = []

    counter = 0
    #for subdir, dirs, files in os.walk(data_path):
        #if ('TPC3' not in subdir or 'TPC4' not in subdir or 'skims' in subdir
        #        or '.tar' in files):
        #    continue
        #print(dirs, subdir, files)
        #if 'TPC3' not in subdir: continue
        #if 'TPC4' not in subdir: continue
        #if 'skims' in subdir: continue
        #if ('.tar.gz' in dirs): continue
        #if ('.tar.gz' in subdir): continue
    for data_path in data_paths:
        for subdir, dirs, files, in os.walk(data_path):
            #input('ha!')
            for f in files:
                if '.py' in f or '.tar' in f: continue
                fname = str(subdir) + str('/') + str(f)
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
                    #print('bsub -q s -o %s "src/refitter %s %s"' % (log, ifile, ofile))
                    #print()
                    #input('which file?')
                    os.system('bsub -q s -o %s "src/refitter %s %s"' % (log, ifile, ofile))
                    #os.system('rm %s' % (fname))
                    counter += 1
                print()

    counter = 0
    if counter == 0:
        r_files = []
        for data_path in data_paths:
            for subdir, dirs, files in os.walk(data_path):
                for f in files:
                    if '.py' in f or '.tar' in f: continue
                    
                    #input('wait!')
                    r_file = str(subdir) + str(f)
                    ifile = os.path.join(subdir, f)
                    print(ifile)
                    dfile = TFile(ifile)
                    tree = dfile.Get('tr')
                    if (os.path.getsize(ifile) < 1000 or tree.GetEntries() == 0 or
                            str(tree)  == '<ROOT.TObject object at 0x(nil)>'):
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