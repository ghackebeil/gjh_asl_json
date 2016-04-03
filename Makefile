SRC=src
TPL=Thirdparty

PREFIX=.
BIN=$(PREFIX)/bin

# Override this to inject gcov flags "-coverage -O0"
# in travis CI script. E.g., $ make COVERAGE="-coverage -O0"
COVERAGE=-O3

CINC=-I$(SRC) -I$(TPL)/solvers
CFLAGS= -pipe -DASL_BUILD -fPIC -DPIC -Wall
LDFLAGS=-ldl

all: 	$(TPL)/solvers/amplsolver.a \
	$(BIN)/gjh_asl_json

$(BIN)/gjh_asl_json: $(SRC)/gjh_asl_json.o \
	             $(SRC)/AmplInterface.o \
		     $(TPL)/solvers/amplsolver.a
	@mkdir -p $(BIN)
	$(CXX) $(COVERAGE) $(CFLAGS) $(CINC) $^ $(LDFLAGS) -o $@

%.o : %.cpp $(SRC)/AmplInterface.hpp
	$(CXX) $(COVERAGE) $(CFLAGS) $(CINC) -c $< -o $@

$(TPL)/solvers/amplsolver.a :
	make -C $(TPL)/solvers/;

clean:
	rm -f $(BIN)/gjh_asl_json;
	rm -f $(SRC)/*.o
	rm -f $(SRC)/*~
	make clean -C $(TPL)/solvers;
