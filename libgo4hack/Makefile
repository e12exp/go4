libgo4hack.so: go4hack.cxx
	clang++ -g $(shell root-config --cflags) -I $(GO4SYS)/include/ -I $(ROOTSYS)/include/root/ -I $(ROOTSYS)/include/ -fPIC  -shared -o $@ $<

clean:
	rm -f *.so *~
