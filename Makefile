CC=cc
INCLUDE=-I. -I./c/ -I./raylib-5.5_linux_amd64/include/
FLAGS=-Wall -Wextra -g
LIB=-L./raylib-5.5_linux_amd64/lib/ -l:libraylib.a -lm
NAME=main
SRC_PATH=./c/
SRC=fit_crc.c fit_convert.c fit.c fit_example.c
SRCS=$(addprefix $(SRC_PATH), $(SRC))

OBJ_PATH=obj/
OBJ = $(SRC:.c=.o)
OBJS = $(addprefix $(OBJ_PATH), $(OBJ))

SRC_MAIN=main.c
OBJ_MAIN=$(SRC_MAIN:.c=.o)
OBJS_MAIN = $(addprefix $(OBJ_PATH), $(OBJ_MAIN))

all: $(NAME)

$(OBJ_PATH)%.o: $(SRC_PATH)%.c
	@mkdir -p $(OBJ_PATH)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(OBJS_MAIN): $(SRC_MAIN)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(NAME): $(OBJS) $(OBJS_MAIN)
	$(CC) $(INCLUDE) $(FLAGS) $^ -o $@ $(LIB)
