CC = gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wshadow -Wconversion -pedantic
TARGET := assign_labs

.PHONY: all clean test strict asan test-asan stress benchmark

all: $(TARGET)

$(TARGET): assignment.c
	$(CC) $(CFLAGS) -o $(TARGET) assignment.c

strict:
	$(MAKE) clean
	$(MAKE) CFLAGS='-std=c11 -O2 -Wall -Wextra -Wshadow -Wconversion -pedantic -Werror'

test: $(TARGET)
	python3 -m pytest -q

asan:
	$(MAKE) clean
	$(MAKE) CC=clang CFLAGS='-std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Wshadow -Wconversion -pedantic'

test-asan:
	$(MAKE) clean
	ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=print_stacktrace=1 \
	ASSIGN_LABS_CC=$(CC) \
	ASSIGN_LABS_CFLAGS='-std=c11 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Wshadow -Wconversion -pedantic' \
	python3 -m pytest -q

stress:
	$(MAKE) clean
	$(MAKE) CFLAGS='-std=c11 -O2 -Wall -Wextra -Wshadow -Wconversion -pedantic'
	mkdir -p tmp
	python3 scripts/generate_stress.py \
	  --labs 256 \
	  --students 1024 \
	  --preferences 256 \
	  --name-bytes 250 \
	  --capacity-mode varied \
	  --seed 7 \
	  --lab-file tmp/stress_labs.txt \
	  --preference-file tmp/stress_prefs.txt
	/usr/bin/time -p ./$(TARGET) tmp/stress_labs.txt tmp/stress_prefs.txt tmp/stress_out.txt

benchmark: $(TARGET)
	python3 scripts/run_benchmarks.py \
	  --binary ./$(TARGET) \
	  --bench-dir tmp/bench_final \
	  --output docs/benchmark_results.tsv \
	  --repeat 1

clean:
	rm -f $(TARGET) $(TARGET).exe
	rm -rf $(TARGET).dSYM
	rm -rf .pytest_cache tests/__pycache__ tmp
	rm -f *.metrics.txt *.by_lab.txt *.by_student.tsv *.outside_preferences.tsv
	rm -f *.reasons.tsv *.adjustment_delta.tsv *.explain.tsv *.portfolio.tsv *.portfolio_parallel.tsv
	rm -f *.profile.tsv
	rm -f *.rubric.txt *.satisfaction.txt *.fair.txt *.balanced.txt *.guarded.txt *.fill_focused.txt *.fill_convex.txt *.weighted_exact.txt
