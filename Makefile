CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
INCLUDES = -Isrc
BUILDDIR = build

all: $(BUILDDIR)/main

test: $(BUILDDIR)/test
	./$(BUILDDIR)/test

$(BUILDDIR)/main: src/main.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

$(BUILDDIR)/test: tests/test.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean test
