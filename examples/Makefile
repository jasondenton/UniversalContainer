all: example1 example_schema

CXXFLAGS=-L/usr/local/opt/flex/lib -lfl

example1 : example1.o ../libuc.a
	$(CXX) -o $@ $^

example_schema : example_schema.cpp ../libuc.a
	$(CXX) -o $@ $^
		
clean :
	rm -f *.o
	rm -f *.dSYM
	rm -f example1
	rm -rf example_schema
