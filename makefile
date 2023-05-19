SUBDIRS=./misc ./src/assembler
TARGETS=all clean distribute diff

$(TARGETS): subdirs
	@echo making top $@

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(filter $(TARGETS),$(MAKECMDGOALS))

.PHONY: subdirs $(TARGETS) $(SUBDIRS)

%::
	@echo making top $@