NAME_TEST = test2
SRC_TEST =  ./testsuite/unit_tests.c \
			./src/utils.c \
			./src/request.c \
			./src/header_struct.c\
			./src/request_handler.c\
			./src/response.c\
			./src/media_struct.c\
			./src/files.c\
			./src/queue.c\
			./src/cache.c\

NAME_TEST_E2E = test-e2e
SRC_TEST_E2E =  ./testsuite/e2e_tests.c \
			./src/utils.c \
			./src/request.c \
			./src/header_struct.c\
			./src/request_handler.c\
			./src/response.c\
			./src/media_struct.c\
			./src/files.c\
			./src/queue.c\
			./src/cache.c\

NAME_CLIENT = client
SRC_CLIENT = ./src/client.c \
 
NAME_CLIENT_2 = client2
SRC_CLIENT_2 = ./src/client2.c \

NAME_SERVER = cherokee
SRC_SERVER = ./src/server.c \
			 ./src/process.c \
			 ./src/request.c \
			 ./src/header_struct.c\
			 ./src/request_handler.c\
			 ./src/response.c\
			 ./src/media_struct.c\
			 ./src/files.c\
			 ./src/cache.c\
			 ./src/queue.c\
			

CFLAGS	+=  -Wall -Werror -Wextra -pedantic -std=c99 -g -D_GNU_SOURCE  -I /usr/local/include
LDFLAGS = -L /lib/x86_64-linux-gnu/ -L /usr/local/lib/ $(LDLIBS) -ldl
LDLIBS = -lcriterion -lm
DEBUGFLAGS = 

OBJ_CLIENT		=	$(SRC_CLIENT:.c=.o)
OBJ_CLIENT_2	=	$(SRC_CLIENT_2:.c=.o)
OBJ_SERVER		=	$(SRC_SERVER:.c=.o)
OBJ_TEST		=	$(SRC_TEST:.c=.o)
OBJ_TESTS_E2E	=	$(SRC_TEST_E2E:.c=.o)
RM		=	rm -f


$(NAME_CLIENT):	$(OBJ_CLIENT)
			$(CC) $(OBJ_CLIENT) -o $(NAME_CLIENT) $(DEBUGFLAGS) -pthread

$(NAME_SERVER):	$(OBJ_SERVER)
			$(CC) $(OBJ_SERVER) -o $(NAME_SERVER) -lm $(DEBUGFLAGS) -pthread $(LDFLAGS)

$(NAME_CLIENT_2):	$(OBJ_CLIENT_2)
			$(CC) $(OBJ_CLIENT_2) -o $(NAME_CLIENT_2)  -lreadline $(DEBUGFLAGS) -pthread

$(NAME_TEST): $(OBJ_TEST)
			$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ $(DEBUGFLAGS) -pthread

$(NAME_TEST_E2E): $(OBJ_TESTS_E2E)
			$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ $(DEBUGFLAGS) -pthread

all:		$(NAME_CLIENT) $(NAME_SERVER) $(NAME_CLIENT_2)
			


test: $(NAME_TEST)

e2e: $(NAME_TEST_E2E)

clean:	
			$(RM) $(OBJ_CLIENT) $(OBJ_SERVER) $(OBJ_CLIENT_2) $(OBJ_TEST)
			$(RM) ./src/*~* *#*

fclean:		clean
			$(RM) $(NAME_CLIENT) $(NAME_SERVER) $(NAME_CLIENT_2) $(NAME_TEST) $(NAME_TEST_E2E)

re:			fclean all

.PHONY:     all clean fclean re
