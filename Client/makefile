CC = gcc
CFLAGS = -Iinclude 

SRCS1 = serveur.c handlers.c controle.c 
SRCS2 = client.c controle.c
        
OBJS1 = $(SRCS1:.c=.o) 
OBJS2 = $(SRCS2:.c=.o)

EXEC1 = serveur
EXEC2 = client

all: $(EXEC1) $(EXEC2)

$(EXEC1): $(OBJS1)
	$(CC) $(OBJS1) -o $(EXEC1)
$(EXEC2): $(OBJS2)
	$(CC) $(OBJS2) -o $(EXEC2)

clean : 
	@echo " Efface les fichiers ojbets"
	rm $(OBJS1) $(EXEC1) 
	rm $(OBJS2) $(EXEC2)

s:
	@echo "Execute serveur " 
	./serveur 5897
cl:
	@echo "Execute client "
	./client 5897
