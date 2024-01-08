#!/usr/bin/python3 -i
import sys, os
os.environ["DISPLAY"]=""
sys.argv.append( '-b' )
import ROOT
sys.argv=sys.argv[:-1]

import re, itertools, tempfile

#outdir=tempfile.mkdtemp()
outdir=sys.argv[2]
os.mkdir(outdir)

page=1
fname=sys.argv[1]

htype="lim_energy"
if len(sys.argv)>3:
    htype=sys.argv[3]

colors=[1, 2, 3, 4, 6, 7, 8, 9, 11, 30, 37, 40, 41, 42, 46, 48]
f=ROOT.TFile(sys.argv[1])
stuff=[]
sfpprefix=10
sfprange=range(0,4)
fbxrange=range(0,16)
split=1
for sfp,fbx in itertools.product(sfprange, fbxrange):
    if sfp in [1,2] and fbx%2==0:
        continue # skip proton channels
    sfp+=sfpprefix
    title="%02d"%sfp+".%02d"%fbx
    
    cv=ROOT.TCanvas(title, title, 1200, 1000)
    #ROOT.gPad.SetLogy(1)
    if split:
        cv.Divide(4,4)
    leg=ROOT.TLegend(0.7, 0.4, 0.9, 0.9)
    for i in range(16):
        if split:
            titleh=title+".%02d"%i
            cv.cd(i+1)
        else:titleh=title
        hpath="Histograms/%s/sfp_%02d/febex_%02d/%s_%02d_%02d_%02d"%(htype, sfp, fbx, htype, sfp, fbx, i)
        h=f.Get(hpath)
        if (not h):
            print("Warning: could not find %s"%hpath)
            h=ROOT.TH1I("%s_%02d_%02d_%02d"%(htype, sfp, fbx, i), "",  100, 0, 4000)
        stuff.append(h)
        h.SetTitle(titleh)
        if htype=="lim_energy":
            h.GetXaxis().SetRangeUser(100, 2500)
        if not split:
            h.SetLineColor(colors[i])
        h.Draw(["HIST", "HIST same"][i>0])
        leg.AddEntry(h, "FbxCh %d (count=%d)" %(i, h.Integral()), "L")
    if not split:
        leg.Draw()
    
    cv.Print("%s/p%03d.pdf"%(outdir, page))
    page+=1
    stuff.append([cv, leg])

print("you might want to use pdfunite to concat these pdfs.")
