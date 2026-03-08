CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

.PHONY: all clean

all: marketplace_cli marketplace_cgi stress_test

marketplace_cli: main.cxx marketplace.cxx
	$(CXX) $(CXXFLAGS) -o marketplace_cli main.cxx marketplace.cxx

marketplace_cgi: cgi_main.cxx cgi_utils.cxx marketplace.cxx
	$(CXX) $(CXXFLAGS) -o marketplace_cgi cgi_main.cxx cgi_utils.cxx marketplace.cxx

stress_test: stress_test.cxx marketplace.cxx
	$(CXX) $(CXXFLAGS) -o stress_test stress_test.cxx marketplace.cxx

clean:
	rm -f marketplace_cli marketplace_cgi stress_test
