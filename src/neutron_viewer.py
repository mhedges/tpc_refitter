from ROOT import TFile, TTree, gROOT, TCanvas, TH1F, TH2F, TLine, TMath

ifile = TFile('test.root')
t = gROOT.FindObject('tr')

h = TH2F('h', 'Track', 80, 0.5, 80.5*250.0, 336, 0.5, 336.5*50.0)
h2=TH1F('h2','bcids',256,0.5,256.5)

counter = 0
for event in t:
    if event.neutron == 1:
        npoints = event.npoints
        for i in xrange(npoints):
            h.Fill(event.col[i]*250.0, event.row[i]*50.0, event.tot[i])
        h.Draw('COLZ')
        print 'Max distance = ', max(event.distances)
        print 'Average distance = ', sum(event.distances)/len(event.distances)
        phi = event.phi*3.14159/180.0
        pars = event.par_fit
        x0 = pars[0]
        y0=pars[1]
        x1=0.0
        y1=y0-x0*TMath.Tan(phi)
        x2=500.0*250.0
        y2=y0+(x2-x0)*TMath.Tan(phi)
        print x0, y0, x1, y1, x2, y2, event.t_length, event.theta, event.phi
        nline = TLine(x1,y1,x2,y2)
        nline.SetLineColor(2)
        nline.Draw('SAME')
        print 'Event number ', counter
        raw_input('Well?')
        h.Reset()
    counter += 1
