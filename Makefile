CC=cc
RAYLIB_PATH=./raylib-5.5_linux_amd64
INCLUDE=-I. -I./c/ -I$(RAYLIB_PATH)/include/
FLAGS=-Wall -Wextra -g
LIB=-Wl,-rpath=$(RAYLIB_PATH)/lib/ -L$(RAYLIB_PATH)/lib/ -l:libraylib.so -lm
NAME=main
SRC_PATH=./c/
SRC=fit_crc.c fit_convert.c fit.c fit_example.c

OBJ_PATH=obj
OBJS = $(SRC:%.c=$(OBJ_PATH)/%.o)

SRC_MAIN=main.c
OBJS_MAIN=$(SRC_MAIN:%.c=$(OBJ_PATH)/%.o)

all: $(NAME)

$(OBJ_PATH)%.o: $(SRC_PATH)%.c
	@mkdir -p $(OBJ_PATH)
	$(CC) $(FLAGS) $(INCLUDE) -o $@ -c $<

$(OBJS_MAIN): $(SRC_MAIN)
	$(CC) $(FLAGS) $(INCLUDE) -o $@ -c $<

$(NAME): $(OBJS) $(OBJS_MAIN)
	$(CC) $(INCLUDE) $(FLAGS) $^ -o $@ $(LIB)
