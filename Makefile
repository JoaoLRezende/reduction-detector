#-------------------------------------------------------------------------------
# This makefile is based on code by Eli Bendersky; see
# https://eli.thegreenplace.net/2014/samples-for-using-llvm-and-clang-as-a-library/.
#-------------------------------------------------------------------------------

# TODO: sanitize this makefile. Make it capable of incremental compilation
# (building one relocatable object file from each source file individually
# and preserving it in the build directory − and, in each build, regenerating
# only relocatable object files whose source files were changed since
# the last build). Make it aware of header files.

# To link against a build of LLVM, LLVM_PATH should point to its location
# (the untarred directory that contains bin/, lib/, include/
# and other directories).
LLVM_PATH 		:= "$${HOME}/slightly-more-important-downloads/clang+llvm-12.0.0-x86_64-linux-gnu-ubuntu-20.04"
LLVM_BIN_PATH 	:= $(LLVM_PATH)/bin

$(info -----------------------------------------------)
$(info Using LLVM_PATH = $(LLVM_PATH))
$(info Using LLVM_BIN_PATH = $(LLVM_BIN_PATH))
$(info -----------------------------------------------)

# If you see an error message of "clang++ not found" but you have
# Clang 12 (which is required for executing reduction-detector anyway),
# you might consider changing the value of CXX to "clang++-12" to use that
# installation of Clang. Or execute something like "sudo apt install clang".
CXX := clang++
CXXFLAGS := -fno-rtti -O0 -g
PLUGIN_CXXFLAGS := -fpic

LLVM_CXXFLAGS := `$(LLVM_BIN_PATH)/llvm-config --cxxflags`
LLVM_LDFLAGS := `$(LLVM_BIN_PATH)/llvm-config --ldflags --libs --system-libs`

# Plugins shouldn't link LLVM and Clang libs statically, because they are
# already linked into the main executable (opt or clang). LLVM doesn't like its
# libs to be linked more than once because it uses globals for configuration
# and plugin registration, and these trample over each other.
LLVM_LDFLAGS_NOLIBS := `$(LLVM_BIN_PATH)/llvm-config --ldflags`
PLUGIN_LDFLAGS := -shared

# These are required when compiling vs. a source distribution of Clang. For
# binary distributions llvm-config --cxxflags gives the right path.
CLANG_INCLUDES := \
	-I$(LLVM_PATH)/tools/clang/include \
	-I$(LLVM_PATH)/tools/clang/include

# List of Clang libraries to link. The proper -L will be provided by the
# call to llvm-config
# Note that I'm using -Wl,--{start|end}-group around the Clang libs; this is
# because there are circular dependencies that make the correct order difficult
# to specify and maintain. The linker group options make the linking somewhat
# slower, but IMHO they're still perfectly fine for tools that link with Clang.
CLANG_LIBS := \
	-Wl,--start-group \
	-lclangAST \
	-lclangASTMatchers \
	-lclangAnalysis \
	-lclangBasic \
	-lclangDriver \
	-lclangEdit \
	-lclangFrontend \
	-lclangFrontendTool \
	-lclangLex \
	-lclangParse \
	-lclangSema \
	-lclangEdit \
	-lclangRewrite \
	-lclangRewriteFrontend \
	-lclangStaticAnalyzerFrontend \
	-lclangStaticAnalyzerCheckers \
	-lclangStaticAnalyzerCore \
	-lclangCrossTU \
	-lclangIndex \
	-lclangSerialization \
	-lclangToolingCore \
	-lclangTooling \
	-lclangFormat \
	-Wl,--end-group

# Internal paths in this project: where to find sources, and where to put
# build artifacts.
SRC_DIR := src
BUILD_DIR := build

.PHONY: all
all: make_builddir \
	emit_build_config \
	$(BUILD_DIR)/reduction-detector \

.PHONY: emit_build_config
emit_build_config: make_builddir
	@echo $(LLVM_BIN_PATH) > $(BUILD_DIR)/_build_config

.PHONY: make_builddir
make_builddir:
	@test -d $(BUILD_DIR) || mkdir $(BUILD_DIR)


$(BUILD_DIR)/reduction-detector: 	$(SRC_DIR)/*.cpp \
									$(SRC_DIR)/loop_analysis/*.cpp
	$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $^ \
		$(CLANG_LIBS) $(LLVM_LDFLAGS) -o $@

.PHONY: clean format

clean:
	rm -rf $(BUILD_DIR)/*

format:
	find . -name "*.cpp" | xargs clang-format -style=file -i
