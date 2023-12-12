include CONFIG

# All %.o files related %.cpp files
NETWORKING = $(patsubst %.cpp,%.o,$(wildcard Networking/*.cpp))
TOOLS = $(patsubst %.cpp,%.o,$(wildcard Tools/*.cpp))
MATH = $(patsubst %.cpp,%.o,$(wildcard Math/*.cpp))
PROTOCOLS = $(patsubst %.cpp,%.o,$(wildcard Protocols/*.cpp))
TYPES = $(patsubst %.cpp,%.o,$(wildcard Types/*.cpp))
NEURALNET = $(patsubst %.cpp,%.o,$(wildcard NeuralNet/*.cpp))

COMMONOBJS = $(TOOLS) $(NETWORKING) $(MATH) $(PROTOCOLS) $(TYPES) $(NEURALNET)

DEPS := $(wildcard */*.d */*/*.d)

-include $(DEPS)
include $(wildcard *.d static/*.d)

all: $(SHAREDLIB) $(LIBRELEASE)
.PHONY : all

CFLAGS += -fPIC
LDLIBS += -Wl,-rpath -Wl,$(CURDIR)

LIBRELEASE = librelease.a
SHAREDLIB = libHM.so

$(SHAREDLIB): $(COMMONOBJS)
	$(CXX) $(CFLAGS) -shared -o $@ $^ $(LDLIBS)

$(LIBRELEASE):  $(COMMONOBJS)
	$(AR) -csr $@ $^

%.o: %.cpp
	$(CXX) -o $@ $< $(CFLAGS) -MMD -MP -c 

# If I use the sharedlib to complie it, there is no duplicate symbols.
%.x: Test/%.o $(LIBRELEASE)
	$(CXX) -o $@ $(CFLAGS) $^ $(LDLIBS)

%.x: ML/%.o $(LIBRELEASE)
	$(CXX) -o $@ $(CFLAGS) $^ $(LDLIBS)

all: test_units.x test_network.x test_eigen.x inference.x

test: test_units.x ## Run this to print the output of (only) Party 0 to terminal
	./test_units.x 2  >/dev/null &
	./test_units.x 1  >/dev/null &
	./test_units.x 0  
	@echo "Execution completed"

network: test_network.x
	./test_network.x 2  >/dev/null &
	./test_network.x 1  >/dev/null &
	./test_network.x 0
	@echo "Execution completed"

eigen: test_eigen.x
	./test_eigen.x
	@echo "Execution completed"

# The following are the supported settings in the bash script.
# Here we illustrate an example of 3PC setting and show the detailed execution command for each party.
# It inconvenient to entering such tedious command in the terminal to conduct experiments for the settings of large number of parties. 
# So we use bash script to faciliate the experiment. 
MAX_PLAYERS=3
THRESHOLD=1
TEST_DATA_SIZE=1
PRIME=PR31# {PR31, PR61}
NETWORK=SecureML# {SecureML, Sarda, MiniONN}
DATASET=MNIST# {MNIST}
TRUE_OFFLINE=0
CORES=4
NET=LAN

IP_FILE=Inference/IP_HOSTS/IP_LOCAL
BASE_FILE=Inference/$(NETWORK)
Output_file=$(BASE_FILE)/$(NETWORK).P0
OFFLINE_ARG=$(BASE_FILE)/offline/$(PRIME)_offline_b$(TEST_DATA_SIZE).txt

terminal: inference.x
	./inference.x 2 $(IP_FILE) $(THRESHOLD) $(NETWORK) $(DATASET) $(TEST_DATA_SIZE) $(TRUE_OFFLINE) $(OFFLINE_ARG) $(CORES) > /dev/null &
	./inference.x 1 $(IP_FILE) $(THRESHOLD) $(NETWORK) $(DATASET) $(TEST_DATA_SIZE) $(TRUE_OFFLINE) $(OFFLINE_ARG) $(CORES) > /dev/null &
	./inference.x 0 $(IP_FILE) $(THRESHOLD) $(NETWORK) $(DATASET) $(TEST_DATA_SIZE) $(TRUE_OFFLINE) $(OFFLINE_ARG) $(CORES)
	@echo "Execution completed"

zero: inference.x
	./inference.x 0 $(IP_FILE) $(THRESHOLD) $(NETWORK) $(DATASET) $(TEST_DATA_SIZE) $(TRUE_OFFLINE) ${OFFLINE_ARG} ${CORES}
	@echo "Execution completed"

one: inference.x
	./inference.x 1 $(IP_FILE) $(THRESHOLD) $(NETWORK) $(DATASET) $(TEST_DATA_SIZE) $(TRUE_OFFLINE) ${OFFLINE_ARG} ${CORES}
	@echo "Execution completed"

two: inference.x
	./inference.x 2 $(IP_FILE) $(THRESHOLD) $(NETWORK) $(DATASET) $(TEST_DATA_SIZE) $(TRUE_OFFLINE) ${OFFLINE_ARG} ${CORES}
	@echo "Execution completed"

ifeq ($(OS), Darwin)
setup: mac-setup
else
setup: linux-machine-setup
endif

mac-setup: 
	brew install libsodium
	-echo MY_CFLAGS += -I/usr/local/opt/openssl/include -I/opt/homebrew/opt/openssl/include -I/opt/homebrew/include >> CONFIG.mine
	-echo MY_LDLIBS += -L/usr/local/opt/openssl/lib -L/opt/homebrew/lib -L/opt/homebrew/opt/openssl/lib >> CONFIG.mine

ifeq ($(MACHINE), aarch64)
mac-machine-setup:
	-echo ARCH = >> CONFIG.mine
linux-machine-setup:
	-echo ARCH = -march=armv8.2-a+crypto >> CONFIG.mine
else
mac-machine-setup:
linux-machine-setup:
endif

ifeq ($(MACHINE), aarch64)
setup: simde/simde
endif

simde/simde:
	git submodule update --init simde || git clone https://github.com/simd-everywhere/simde

clean:
	-rm -f */*.o *.o */*.d *.d *.x core.* *.a gmon.out */*/*.o static/*.x *.so
.PHONY : clean