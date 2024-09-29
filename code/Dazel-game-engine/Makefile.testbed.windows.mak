DIR := $(subst /,\,$(CURDIR))
BUILD_DIR := bin
OBJ_DIR := obj


ASSEMBLY := testbed
EXTENSION := .exe
COMPILER_FLAGS := -g -Wno-missing-braces -fdeclspec#-fpic
INCLUDE_FLAGS := -Iengine\src -Itestbed\src
LINKER_FLAGS := -g  -Iengine.lib  -L$(OBJ_DIR)\engine -L$(VULKAN_SDK)
DEFINES := -D_DEBUG -DKIMPORT

#make does not offer recursive wild card functions so here is one
rwildcard=$(wildcard $1$2) $(foreach d, $(wildcard $1*),$(call rwildcard,$d/,$2))

SRC_FILES := $(call rwildcard, $(ASSEMBLY)/,*.cpp)
DIRECTORIES := \$(ASSEMBLY)\src $(subst $(DIR),,$(shell dir $(ASSEMBLY)\src /S /AD /B | findstr /i src))
OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o)

all: scaffold compile link

.PHONY: scaffold
scaffold: #create build directory
         @echo Scaffold folder  structure...
		 @setlocal enableextensions enabledelayedexpansion && mkdir $(addprefix $(OBJ_DIR),$(DIRECTORIES)) 2>NUL || cd .
		 @echo Done.


.PHONY: link
link: scaffold $(OBJ_FILES) #link
      @echo Linking $(ASSEMBLY)....
	  @clang $(OBJ_FILES) -o $(BUILD_DIR)/$(ASSEMBLY)$(EXTENSION) $(LINKER_FLAGS)

.PHONY: compile
compile: #compile cpp files 
         @echo Compiling...
.PHONY: clean
clean: # clean build directory
       if exist $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION) del $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION)
	   rmdir /s /q $(OBJ_DIR)\$(ASSEMBLY)

$(OBJ_DIR)/%.cpp.o: %.cpp 
       @echo  $<...
	   @clang $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)