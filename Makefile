all:
	g++ -o skimmer -L$(ROOTSYS)/lib skimmer.C -O2 `root-config --cflags --libs` -lTreePlayer
