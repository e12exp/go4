libgo4hack.so: go4hack.cxx
	g++ -g --std=c++14 -I $(GO4SYS)/include/ -I $(ROOTSYS)/include -fPIC  -shared -o $@ $<

clean:
	rm -f *.so *~
