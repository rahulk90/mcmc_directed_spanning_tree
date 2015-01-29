CC = g++
#INCLUDES = -I/usr/include -I/home/rahul/software/boost_1_55_0/include
INCLUDES = -I/home/rahul/software/boost_1_55_0/include
LFLAGS = -L/home/rahul/software/boost_1_55_0/lib
OPTFLAGS = -O2 -std=c++0x 
CFLAGS  = -g -Wall
TARGET = MCMC_spanning_tree
TEST = random_spanning_tree_test

#Set to -DDEBUG, -DDEBUG_L2 (only for test) to compile with debug statements
DEBUG   = #-DDEBUG 

all: $(TARGET) $(TEST)
#all: $(TEST)

$(TARGET): $(TARGET).o
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).o $(LFLAGS)

$(TARGET).o: $(TARGET).cpp 
	$(CC) $(DEBUG) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $(TARGET).o -c $(TARGET).cpp

$(TEST): $(TEST).o
	$(CC) $(CFLAGS) -o $(TEST) $(TEST).o $(LFLAGS)

$(TEST).o: $(TEST).cpp 
	$(CC) $(DEBUG) $(CFLAGS) $(INCLUDES) $(OPTFLAGS) -o $(TEST).o -c $(TEST).cpp

clean:
	$(RM) $(TEST) $(TARGET) *.o *.out 
