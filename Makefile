SRC 		= 		src/malloc.c 			\
					src/free.c				\
					src/heap_start.c 		\
					src/utils.c				\
					src/realloc.c			\
					src/merge_free_blocks.c \
					src/calloc.c			\
					src/init_block.c 		\
					src/check_heap.c 		\
					src/init_heap.c 		\
					src/check_free_list.c 	\
					src/boundary_tags.c 	\
					src/malloc_usable_size.c \

TSRC		=		tests/malloc.c 			\

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

debug: CFLAGS += -g3 -O0
debug: re

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)
	$(RM) test_main

re: fclean all

tests_run:
	$(CC) $(TSRC) --coverage -lcriterion -o unit_tests
	./unit_tests

.PHONY:	fclean clean all re tests_run debug