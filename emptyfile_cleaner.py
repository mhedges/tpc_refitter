import os

def main():
    ifpath = '/ghi/fs01/belle2/bdata/group/detector/BEAST/data/NTP/TPC/skims/'

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
