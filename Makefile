# This file is part of bolt.
#
# Copyright (c) 2015, Alexandre Monti
#
# bolt is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# bolt is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with bolt.  If not, see <http://www.gnu.org/licenses/>.

-include Makefile.inc

## Source management
SOURCES:=$(wildcard $(SOURCE_DIR)/*.cpp)
OBJECTS:=$(patsubst $(SOURCE_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
DEPENDENCIES:=$(patsubst $(BUILD_DIR)/%.o,$(BUILD_DIR)/%.d,$(OBJECTS))

## Top-level targets
all: $(BINARY)

$(BINARY): $(OBJECTS)
	@echo "${blue}Linking product '$@'${rcol}"
	@mkdir -p $(@D)
	@$(CXX) $(OBJECTS) $(LDFLAGS) -o $(BINARY)


run: $(BINARY)
	@$(BINARY) $(options)

check: $(BINARY)
	@valgrind --tool=memcheck --leak-check=full $(BINARY)

-include $(DEPENDENCIES)

## Translation rules
$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@echo "${green}Building object file '$@'${rcol}"
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

## Phony targets
.PHONY: clean
clean:
	@echo "${blue}Removing build directories${rcol}"
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
