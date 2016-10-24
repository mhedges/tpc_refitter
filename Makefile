all:
	g++ -o refitter -L$(ROOTSYS)/lib src/refitter.C -O2 `root-config --cflags --libs` -lTreePlayer
