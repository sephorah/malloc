SRC 		= 		src/malloc.c

TSRC		=		tests/tests_malloc.c

OBJ 		=		$(SRC:.c=.o)

NAME 		=	 	libmalloc.so

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
	$(RM) unit_tests*

re: fclean all

tests_run:
	$(CC) $(TSRC) --coverage -lcriterion -o unit_tests
	./unit_tests

.PHONY:	fclean clean all re tests_run debug