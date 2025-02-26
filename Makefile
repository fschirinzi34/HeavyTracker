CC = cc
SRC = main.c genetic_algorithm.c heavytracker.c xxhash.c
OUT = heavyTracker

TEST_HT_SRC = test_heavyTracker.c genetic_algorithm.c heavytracker.c xxhash.c
TEST_HT_OUT = test_heavyTracker

TEST_GA_SRC = test_genetic_algorithm.c genetic_algorithm.c heavytracker.c xxhash.c
TEST_GA_OUT = test_ga

all: $(OUT) $(TEST_HT_OUT) $(TEST_GA_OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT)

$(TEST_HT_OUT): $(TEST_HT_SRC)
	$(CC) $(TEST_HT_SRC) -o $(TEST_HT_OUT)
	
$(TEST_GA_OUT): $(TEST_GA_SRC)
	$(CC) $(TEST_GA_SRC) -o $(TEST_GA_OUT)

clean:
	rm -f $(OUT) $(TEST_HT_OUT) $(TEST_GA_OUT)
