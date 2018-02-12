CC       = gcc
FLAGS    = -std=gnu99

BINFILES = minishell


all: $(BINFILES)

%:%.c	
	$(CC) $(FLAGS) $+ -o $@

clean:
	$(RM) $(BINFILES)
