
CC = gcc -g -Wall
CFLAGS = `pkg-config --cflags gtk+-2.0 gtkgl-2.0`
LDFLAGS = `pkg-config --libs gtk+-2.0 gtkgl-2.0`

NAME = trees

OBJS = $(NAME).o draw.o random.o

$(NAME): $(OBJS)
# $(CC) $(GTK_LIB) $(X11_LIB) $(OBJS) -o $(NAME) $(LDFLAGS)

clean:
	rm -f *.o *~ $(NAME)
