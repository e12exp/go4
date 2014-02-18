{
  TFile *f1 = new TFile("rpid-cut.root", "recreate");
  f1->cd();
  CUTG->Write();
  delete f1;
}
