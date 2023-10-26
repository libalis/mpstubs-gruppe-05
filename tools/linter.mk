# Perform static code checks

TIDY ?= clang-tidy
CPPLINT ?= /usr/bin/env python3 "$(CURRENT_DIR)/cpplint.py"

# Check sources with Clang Tidy
tidy::
ifeq (,$(CC_SOURCES))
	@echo "(nothing to tidy)"
else
	$(VERBOSE) $(TIDY) --format-style=google -header-filter=.* -warnings-as-errors="readability*" -checks="readability*,google-readability-casting,google-explicit-constructor,bugprone*,-bugprone-easily-swappable-parameters,-bugprone-implicit-widening-of-multiplication-result,-bugprone-narrowing-conversions,-bugprone-reserved-identifier,-readability-else-after-return,-readability-identifier-length,-readability-magic-numbers,-readability-use-anyofallof,-readability-function-cognitive-complexity" $(filter-out utils/png.cc,$(CC_SOURCES)) -- $(CXXFLAGS_ARCH) $(CXXFLAGS_DEFAULT) $(CXXFLAGS_OPT)
endif

# Check sources with cpplint
lint::
	@if $(CPPLINT) --quiet --recursive . ; then \
		echo "Congratulations, coding style obeyed!" ; \
	else \
		echo "Coding style violated -- see CPPLINT.cfg for details" ; \
		exit 1 ; \
	fi

# Documentation
help::
	@/bin/echo -e "" \
		"	\e[3mlint\e[0m     Checks the coding style using \e[4mCPPLINT\e[0m\n\n" \
		"	\e[3mtidy\e[0m     Uses \e[4mClang Tidy\e[0m for a static code analysis\n\n"

# Phony targets
.PHONY: tidy lint help
