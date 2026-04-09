SOURCES= \
	 common/tools.cpp\
	 list/list.cpp\
	 hashtable/hashtable.cpp\
	 hashtable/hashtable_dump.cpp\
	 hash/hash.cpp\
	 test.cpp
	
INCLUDES_DIR = \
	 list/\
	 common/\
	 hashtable/\
	 hash/


OBJ_DIR = obj
SOURCE_DIR = src

INCLUDES = $(addprefix -I$(SOURCE_DIR)/, $(INCLUDES_DIR))

OBJECTS := $(addprefix $(OBJ_DIR)/, $(SOURCES:.cpp=.o))
SOURCES := $(addprefix $(SOURCE_DIR)/, $(SOURCES))
TARGET = hashtable.out

# c++/c compiler options
CC = gcc
CXXFLAGS =  -D _DEBUG -ggdb3 -std=c++17 -O2 
CXXFLAGS += -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations\
		 -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts\
		 -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal\
		 -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op\
		 -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self\
		 -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel\
		 -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods\
		 -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand\
		 -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix\
		 -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector\
		 -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -pie\
		 -fPIE -Werror=vla \
		 -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,   \
		               -fsanitize=leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow\
		               -fsanitize=undefined,unreachable,vla-bound,vptr
CXXFLAGS += $(INCLUDES)

LDFLAGS := $(CXXFLAGS)
LDFLAGS += -lstdc++

logfolder = logs

$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@mkdir -p $(logfolder)
	@echo "Compiling" $<
	@$(CC) $(CXXFLAGS) -c $< -o $@ 	
	@echo "Compiled Successfully" $<

$(TARGET): $(OBJECTS)
	@echo "Linking..."
	@$(CC) $(LDFLAGS) $^ -o $@
	@echo "Linked Successfully"

all: $(TARGET)
	@./$(TARGET)

clean:
	@rm -rf $(OBJ_DIR)
	@rm -f $(TARGET)
	@echo "Cleaned Successfully"
