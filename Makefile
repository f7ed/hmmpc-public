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