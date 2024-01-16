CC = gcc
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
CFLAGS = -Wall -Wextra \
         -std=c99 \
         -O2 \
         -g \
         -D_GNU_SOURCE \
         -I./include \
         `pkg-config --cflags --libs gtk+-3.0` \

LDFLAGS = -g

LDLIBS = -lm \
         -L./lib \
         -ljson-c \
         -fuse-ld=gold \
         -lcurl \
         -ltensorflow \
         -fsanitize=address \
         -Wl,-rpath=./lib \
         -rdynamic \
         -lssl -lcrypto \

TARGET ?= safelink

SRCS_IA=$(wildcard ./ia/*.c)
SRCS_IA+= $(wildcard ./ia/black_list/*.c)
SRCS_SECU=$(wildcard ./secu/*.c)
SRCS_MAIL=$(wildcard ./mail/*.c)


SRCS=$(wildcard ./*.c)
SRCS+= ${SRCS_IA}
SRCS+= ${SRCS_SECU}
SRCS+= ${SRCS_MAIL}

#delete the main.c from the SRCS
SRCS := $(filter-out ./ia/main.c,$(SRCS))
SRCS := $(filter-out ./secu/main.c,$(SRCS))
SRCS := $(filter-out ./mail/main.c,$(SRCS))


OBJS_IA=$(SRCS_IA:.c=.o)
OBJS_SECU=$(SRCS_SECU:.c=.o)
OBJS_MAIL=$(SRCS_MAIL:.c=.o)

OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) ${LDFLAGS} ${LDLIBS} -o $(TARGET)

safelink:
	@echo "Compilation de $@ en cours..."
	$(eval SRCS := $(filter-out ./testsuit.c,$(SRCS)))
	$(CC)  $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(SRCS) -o $(TARGET)

test:
	@echo "Compilation en cours..."
	$(eval SRCS := $(filter-out ./safelink.c,$(SRCS)))
	$(CC)  $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(SRCS) -o $(TARGET)

run : safelink
	./safelink
ia : $(OBJS_IA)
	$(CC) $(CFFLAGS) $(OBJS_IA) ${LDFLAGS} $(LDLIBS) -o test_ia
secu : $(OBJS_SECU)
	$(CC) $(CFLAGS) -I/usr/include/mysql $(OBJS_SECU) ${LDFLAGS} $(LDLIBS) -lmysqlclient -o test_secu
mail : $(OBJS_MAIL)
	$(CC) $(CFLAGS) $(OBJS_MAIL) ${LDFLAGS} $(LDLIBS) -o test_mail

clean:
	${RM} ${OBJS} ${OBJS_IA} ${OBJS_SECU} ${OBJS_MAIL} ${DEPS} ${TARGET} test_mail test_secu test_ia ids.JSON
list:
	@echo $(SRCS)



-include $(DEPS)

# END
