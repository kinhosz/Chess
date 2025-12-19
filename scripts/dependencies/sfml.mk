# SFML-3.0.0-linux-gcc-64-bit.tar.gz
SFML_VERSION := 3.0.0
SFML_TAR_NAME := SFML-$(SFML_VERSION)-linux-gcc-64-bit.tar.gz
SFML_BIN := https://www.sfml-dev.org/files/$(SFML_TAR_NAME)

LIB_SFML := lib/SFML/
INCLUDE_SFML := include/SFML/

define sfml_download
	@curl -LO $(SFML_BIN) 2>/dev/null
endef

define sfml_unzip
	@tar -xzf $(SFML_TAR_NAME)
endef

define sfml_clear_destin_dir
	@rm -rf $(INCLUDE_SFML)
	@rm -rf $(LIB_SFML)
endef

define sfml_move_lib
	@mkdir -p include/SFML && mv -f SFML-$(SFML_VERSION)/include/SFML/* $(INCLUDE_SFML)
	@mkdir -p lib/SFML && mv -f SFML-$(SFML_VERSION)/lib/* $(LIB_SFML)
endef

define sfml_clear_temp_files
	@rm -r SFML-$(SFML_VERSION)-linux-gcc-64-bit.tar.gz
	@rm -r SFML-$(SFML_VERSION)
endef

define sfml_install
	@echo ------------------------------
	@echo Installing SFML $(SFML_VERSION)
	$(call sfml_download)
	$(call sfml_unzip)
	$(call sfml_clear_destin_dir)
	$(call sfml_move_lib)
	$(call sfml_clear_temp_files)
	@echo OK
endef
