
TARGET= keyboardlayoutoptimizer 
OBJS= keyboardlayoutoptimizer.o \
      configuration.o

CC = g++
CPPFLAGS += -O2 -Wall
LIBS = -lrt

optimize_keyboard : $(OBJS)
	$(CC) $(CPPFLAGS) $(LIBS) $(OBJS) -o $(TARGET)

.PHONY : clean
clean : 
	@rm -f $(OBJS)
	@rm -f $(TARGET)

