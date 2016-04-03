SRC=src
TPL=Thirdparty

PREFIX=.
LIB=$(PREFIX)/lib
BIN=$(PREFIX)/bin

COVERAGE=
CINC=-I$(SRC) -I$(TPL)/solvers
CFLAGS=$(CINC) -pipe -DNDEBUG -DASL_BUILD -fPIC -DPIC -Wall
LDFLAGS=-ldl

SOURCES= $(SRC)/AmplInterface.cpp \
	 $(SRC)/gjh_asl_json.cpp
HEADERS= $(SRC)/AmplInterface.hpp

OBJECTS := $(SOURCES:%.cpp=%.o)

all: 	$(TPL)/solvers/amplsolver.a \
	$(LIB)/libAmplInterface.a \
	$(BIN)/gjh_asl_json

$(BIN)/gjh_asl_json: $(SRC)/gjh_asl_json.o $(LIB)/libAmplInterface.a $(TPL)/solvers/amplsolver.a
	@mkdir -p $(BIN)
	$(CXX) $(COVERAGE) $(CFLAGS) $(SRC)/gjh_asl_json.o $(LIB)/libAmplInterface.a $(TPL)/solvers/amplsolver.a $(LDFLAGS) -o $@

$(LIB)/libAmplInterface.a: $(SRC)/AmplInterface.o $(TPL)/solvers/amplsolver.a
	@mkdir -p $(LIB)
	ar rcs $@ $(SRC)/AmplInterface.o

%.o : %.cpp $(HEADERS)
	$(CXX) $(COVERAGE) $(CFLAGS) -c $< -o $@

$(TPL)/solvers/amplsolver.a :
	make -C $(TPL)/solvers/;

clean:
	rm -f $(LIB)/libAmplInterface.a;
	rm -f $(BIN)/gjh_asl_json;
	rm -f $(OBJECTS);
	make clean -C $(TPL)/solvers;
