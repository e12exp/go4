#!/usr/bin/python3

import ROOT, sys, ctypes, itertools

f=ROOT.TFile(sys.argv[1])


probs=[0.9, 0.99, 0.999, 0.9999]
probs=[0.9999]

nprobs=len(probs)

probs=(ctypes.c_double*nprobs)(*probs)
quantiles=(ctypes.c_double*nprobs)()
th={}

def kwproduct(**d):
    keys=d.keys()
    iters=d.values()
    for t in itertools.product(*iters):
        yield dict(zip(keys, t))

for t in kwproduct(
        pc=range(2, 3), # pc id
        sfp=range(0, 4), # sfp
        mod=range(0,16), # module
        ch=range(0,16), # channel
        ):
    locals().update(t)
    name="Histograms/full_discr_amp/sfp_%d%d/febex_%02d/full_discr_amp_%d%d_%02d_%02d"%(pc, sfp, mod, pc, sfp, mod, ch)
    h=f.Get(name)
    if not h:
        print("%s not found!"%name)
        exit(1)
        continue
    h.GetQuantiles(nprobs, quantiles, probs)
    #print(t, list(quantiles), h.GetEntries())
    th[t.values()]=(list(quantiles)[-1])


wix0=open("wix0.h", "w")
for t,thres in th.items():
    pc,sfp,mod,ch=t
    if mod>=16:
        continue
    if pc==2:
        wix0.write("set %d.%02d.%02d.discr_threshold_timing %d\n"%
                   (sfp, mod, ch, thres))
