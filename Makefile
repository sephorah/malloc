SRC 		= 		src/malloc.c \
					src/free.c

TSRC		=		src/main.c

OBJ 		=		$(SRC:.c=.o)

NAME 		=	 	libmalloc.so

CPPFLAGS	=		-iquote include/

CFLAGS 		=		-fPIC -Werror -Wall -Wextra

LDFLAGS 		=		-shared

CC 		?=		gcc

RM 		= 		rm -f

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(LDFLAGS) -o $(NAME) $(OBJ)

debug: CFLAGS += -g3
debug: re

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)
	$(RM) test_main

re: fclean all

test:
	$(CC) $(TSRC) -o test_main $(CPPFLAGS)

.PHONY:	fclean clean all re test debug